#include "public_request_impl.h"

#include "log/logger.h"
#include "util.h"
#include "db_settings.h"
#include "types/convert_sql_db_types.h"
#include "types/sqlize.h"
#include "random.h"


#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>


namespace Fred {
namespace PublicRequest {



std::string Type2Str(Type _type) {
  switch (_type) {
    case PRT_AUTHINFO_AUTO_RIF:           return "AuthInfo (EPP/Auto)";
    case PRT_AUTHINFO_AUTO_PIF:           return "AuthInfo (Web/Auto)";
    case PRT_AUTHINFO_EMAIL_PIF:          return "AuthInfo (Web/Email)";
    case PRT_AUTHINFO_POST_PIF:           return "AuthInfo (Web/Post)";
    case PRT_BLOCK_CHANGES_EMAIL_PIF:     return "Block changes (Web/Email)";
    case PRT_BLOCK_CHANGES_POST_PIF:      return "Block changes (Web/Post)";
    case PRT_BLOCK_TRANSFER_EMAIL_PIF:    return "Block transfer (Web/Email)";
    case PRT_BLOCK_TRANSFER_POST_PIF:     return "Block transfer (Web/Post)";
    case PRT_UNBLOCK_CHANGES_EMAIL_PIF:   return "Unblock changes (Web/Email)";
    case PRT_UNBLOCK_CHANGES_POST_PIF:    return "Unblock changes (Web/Post)";
    case PRT_UNBLOCK_TRANSFER_EMAIL_PIF:  return "Unblock transfer (Web/Email)";
    case PRT_UNBLOCK_TRANSFER_POST_PIF:   return "Unblock transfer (Web/Post)";
    case PRT_CONDITIONAL_CONTACT_IDENTIFICATION:
                                          return "Conditional identification";
    case PRT_CONTACT_IDENTIFICATION:      return "Full identification";
    case PRT_CONTACT_VALIDATION:          return "Validation";
    default:                              return "TYPE UNKNOWN";
  }
}


std::string Status2Str(Status _status)
{
  switch (_status) {
    case PRS_NEW:       return "New";
    case PRS_ANSWERED:  return "Answered";
    case PRS_INVALID:   return "Invalidated";
    default:            return "STATUS UNKNOWN";
  }
}


std::string ObjectType2Str(ObjectType type)
{
    switch (type) {
        case OT_UNKNOWN:
            return "Unknown";
        case OT_CONTACT:
            return "Contact";
        case OT_NSSET:
            return "NSSet";
        case OT_DOMAIN:
            return "Domain";
        case OT_KEYSET:
            return "KeySet";
        default:
            return "Type unknown";
    }
}


bool checkState(Database::ID objectId, unsigned state)
{
  Database::Query sql;
  sql.buffer() << "SELECT COUNT(*) FROM object_state WHERE state_id="
               << state << " AND object_id=" << objectId
               << " AND valid_to ISNULL";
  Database::Connection conn = Database::Manager::acquire();
  Database::Result r_objects = conn.exec(sql);
  int res = (r_objects.size() == 0 ? 0 : (int)r_objects[0][0]);
  return res > 0;
}


void insertNewStateRequest(
        Database::ID blockRequestID,
        Database::ID objectId,
        unsigned state)
{
  Database::InsertQuery osr("object_state_request");
  osr.add("object_id", objectId);
  osr.add("state_id", state);
  Database::Connection conn = Database::Manager::acquire();
  conn.exec(osr);
  Database::Query prsrm;
  prsrm.buffer() << "INSERT INTO public_request_state_request_map ("
                 << " state_request_id, block_request_id "
                 << ") VALUES ( "
                 << "CURRVAL('object_state_request_id_seq'),"
                 << blockRequestID << ")";
  conn.exec(prsrm);
}


/* 
 * check if already blocked request interfere with requested states
 * this is true in every situation other then when actual states are
 * smaller subset of requested states. if requested by setting parameter
 * blockRequestID, all states in subset are closed
 */
bool queryBlockRequest(
        Database::ID objectId,
        Database::ID blockRequestID,
        const std::string& states,
        bool unblock)
{
  // number of states in csv list of states
  unsigned numStates = count(states.begin(),states.end(),',') + 1;

  Database::Query sql;
  sql.buffer() << "SELECT " // ?? FOR UPDATE ??
               << "  osr.state_id IN (" << states << ") AS st, "
               << "  state_request_id "
               << "FROM public_request_state_request_map ps "
               << "JOIN public_request pr ON (pr.id=ps.block_request_id) "
               << "JOIN object_state_request osr "
               << "  ON (osr.id=ps.state_request_id AND osr.canceled ISNULL) "
               << "WHERE osr.object_id=" << objectId << " "
               << "ORDER BY st ASC ";

  Database::Connection conn = Database::Manager::acquire();
  Database::Result result = conn.exec(sql);
  // in case of unblocking, it's error when no block request states are found
  if (!result.size() && unblock)
    return false;

  for (Database::Result::Iterator it = result.begin(); it != result.end(); ++it) {
    Database::Row::Iterator col = (*it).begin();

    if (!(bool)*col)
      return false;

    if ((result.size() == numStates && !unblock) || (result.size() != numStates && unblock))
      return false;

    if (!blockRequestID)
      return true;

    Database::ID stateRequestID = *(++col);
    // close
    Database::Query sql1;
    sql1.buffer() << "UPDATE public_request_state_request_map "
                  << "SET block_request_id=" << blockRequestID << " "
                  << "WHERE state_request_id=" << stateRequestID;
    conn.exec(sql1);

    Database::Query sql2;
    sql2.buffer() << "UPDATE object_state_request "
                  << "SET canceled=CURRENT_TIMESTAMP "
                  << "WHERE id= " << stateRequestID;
    conn.exec(sql2);
  }
  return true;
}


/* check if object has given request type already active */
unsigned long long check_public_request(
        const unsigned long long &_object_id,
        const Type &_type)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result rcheck = conn.exec_params(
            "SELECT pr.id FROM public_request pr"
            " JOIN public_request_objects_map prom ON prom.request_id = pr.id"
            " WHERE pr.resolve_time IS NULL"
            " AND prom.object_id = $1::integer"
            " AND pr.request_type = $2::integer"
            " ORDER BY pr.create_time",
            Database::query_param_list
                (_object_id)
                (_type));
    if (rcheck.size() == 0) {
        return 0;
    }
    else {
        return static_cast<unsigned long long>(rcheck[0][0]);
    }
}


void cancel_public_request(
        const unsigned long long &_object_id,
        const Type &_type,
        const unsigned long long &_request_id)
{
    unsigned long long prid = 0;
    if ((prid = check_public_request(_object_id, _type)) != 0) {
        Database::Connection conn = Database::Manager::acquire();
        conn.exec_params("UPDATE public_request SET resolve_time = now(),"
                " status = $1::integer, resolve_request_id = $2::bigint"
                " WHERE id = $3::integer",
                Database::query_param_list
                    (PRS_INVALID)
                    (_request_id != 0 ? _request_id : Database::QPNull)
                    (prid));
    }
}


bool object_was_changed_since_request_create(const unsigned long long _request_id)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result rnot_changed = conn.exec_params(
            "SELECT ((o.update IS NULL OR o.update <= pr.create_time)"
             " AND (o.trdate IS NULL OR o.trdate <= pr.create_time))"
             " FROM object o"
             " JOIN public_request_objects_map prom on prom.object_id = o.id"
             " JOIN public_request pr on pr.id = prom.request_id"
             " WHERE pr.resolve_time IS NULL AND pr.id = $1::integer",
             Database::query_param_list(_request_id));

    if (rnot_changed.size() != 1 || static_cast<bool>(rnot_changed[0][0]) != true) {
        return true;
    }
    return false;
}
   


PublicRequestImpl::PublicRequestImpl()
    : CommonObjectImpl(0), type_(), create_request_id_(0),
      resolve_request_id_(0), status_(PRS_NEW), answer_email_id_(0),
      registrar_id_(0), man_()
{
}


PublicRequestImpl::PublicRequestImpl(
        Database::ID _id,
        Fred::PublicRequest::Type _type,
        Database::ID _create_request_id,
        Database::DateTime _create_time,
        Fred::PublicRequest::Status _status,
        Database::DateTime _resolve_time,
        std::string _reason,
        std::string _email_to_answer,
        Database::ID _answer_email_id,
        Database::ID _registrar_id,
        std::string _registrar_handle,
        std::string _registrar_name,
        std::string _registrar_url)
    : CommonObjectImpl(_id), type_(_type),
      create_request_id_(_create_request_id), resolve_request_id_(0),
      create_time_(_create_time), status_(_status),
      resolve_time_(_resolve_time), reason_(_reason),
      email_to_answer_(_email_to_answer), answer_email_id_(_answer_email_id),
      registrar_id_(_registrar_id),
      registrar_handle_(_registrar_handle),
      registrar_name_(_registrar_name), registrar_url_(_registrar_url),
      man_()
{
}


void PublicRequestImpl::setManager(Manager* _man)
{
    man_ = _man;
}


void PublicRequestImpl::init(Database::Row::Iterator& _it)
{
    id_               = (unsigned long long)*_it;
    create_request_id_  = *(++_it);
    resolve_request_id_ = *(++_it);
    create_time_      = *(++_it);
    status_           = (Fred::PublicRequest::Status)(int)*(++_it);
    resolve_time_     = *(++_it);
    reason_           = (std::string)*(++_it);
    email_to_answer_  = (std::string)*(++_it);
    answer_email_id_  = *(++_it);
    registrar_id_     = *(++_it);
    registrar_handle_ = (std::string)*(++_it);
    registrar_name_   = (std::string)*(++_it);
    registrar_url_    = (std::string)*(++_it);
}


void PublicRequestImpl::save()
{
    TRACE("[CALL] Fred::Request::RequestImpl::save()");
    if (objects_.empty()) {
      throw std::runtime_error("can't create or update request with no object specified!");
    }

    Database::Connection conn = Database::Manager::acquire();
    if (id_) {
      Database::Query update_request;
      update_request.buffer() << "UPDATE public_request SET "
                              << "status = " << status_ << ", "
                              << "resolve_time = now()";
        if(answer_email_id_ != 0) {
            update_request.buffer() << ", answer_email_id = " << Database::Value(answer_email_id_);
        }
        if(create_request_id_) {
            update_request.buffer()  << ", create_request_id = " << create_request_id_;
        }
        if (resolve_request_id_ != 0) {
            update_request.buffer() << ", resolve_request_id = " << Database::Value(resolve_request_id_);
        }
        update_request.buffer() << " WHERE id = " << id_;

      try {
        conn.exec(update_request);

        LOGGER(PACKAGE).info(boost::format("request id='%1%' updated successfully -- %2%") %
                          id_ % (status_ == PRS_INVALID ? "invalidated" : "answered"));
      }
      catch (Database::Exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        throw;
      }
      catch (std::exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        throw;
      }

    }
    else {

      Database::InsertQuery insert_request("public_request");

      insert_request.add("request_type", type_);
      insert_request.add("create_request_id", create_request_id_);
      insert_request.add("status", status_);
      insert_request.add("reason", reason_);
      insert_request.add("email_to_answer", email_to_answer_);
      insert_request.add("registrar_id", registrar_id_);

      try {
        Database::Transaction transaction(conn);
        transaction.exec(insert_request);

        std::string objects_str = "";
        std::vector<OID>::iterator it = objects_.begin();
        for (; it != objects_.end(); ++it) {
          Database::Query insert_object;
          insert_object.buffer() << "INSERT INTO public_request_objects_map "
                                 << "(request_id, object_id) VALUES ("
                                 << "currval('public_request_id_seq')" << ", "
                                 << it->id << ")";
          conn.exec(insert_object);
          objects_str += sqlize(it->id) + (it == objects_.end() - 1 ? "" : " ");
        }

        Database::Sequence pp_seq(conn, "public_request_id_seq");
        id_ = pp_seq.getCurrent();

        transaction.commit();
        LOGGER(PACKAGE).info(boost::format("request id='%1%' for objects={%2%} created successfully")
                  % id_ % objects_str);

        Database::Result res = conn.exec_params(
               "SELECT create_time FROM public_request WHERE id = $1::integer",
               Database::query_param_list(this->getId()));
        if (res.size() == 1) {
            create_time_ = res[0][0];
        }

        // handle special behavior (i.e. processing request after creation)
        postCreate();
      }
      catch (Database::Exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        throw;
      }
      catch (std::exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        throw;
      }
    }
}


Fred::PublicRequest::Type PublicRequestImpl::getType() const
{
    return type_;
}



void PublicRequestImpl::setType(Fred::PublicRequest::Type _type)
{
    type_ = _type;
    modified_ = true;
}


Fred::PublicRequest::Status PublicRequestImpl::getStatus() const
{
    return status_;
}


void PublicRequestImpl::setStatus(Fred::PublicRequest::Status _status)
{
   status_ = _status;
   modified_ = true;
}


ptime PublicRequestImpl::getCreateTime() const
{
    return create_time_;
}


ptime PublicRequestImpl::getResolveTime() const
{
    return resolve_time_;
}


const std::string& PublicRequestImpl::getReason() const
{
    return reason_;
}


void PublicRequestImpl::setReason(const std::string& _reason)
{
    reason_ = _reason;
    modified_ = true;
}


const std::string& PublicRequestImpl::getEmailToAnswer() const
{
    return email_to_answer_;
}


void PublicRequestImpl::setEmailToAnswer(const std::string& _email)
{
    email_to_answer_ = _email;
    modified_ = true;
}


const Database::ID PublicRequestImpl::getAnswerEmailId() const
{
    return answer_email_id_;
}

const Database::ID PublicRequestImpl::getRequestId() const
{
    return create_request_id_;
}


const Database::ID PublicRequestImpl::getResolveRequestId() const
{
    return resolve_request_id_;
}


void PublicRequestImpl::setRequestId(const Database::ID& _create_request_id)
{
    create_request_id_ = _create_request_id;
    modified_ = true;
}


void PublicRequestImpl::setRegistrarId(const Database::ID& _registrar_id)
{
    registrar_id_ = _registrar_id;
    modified_ = true;
}


void PublicRequestImpl::addObject(const OID& _oid)
{
    objects_.push_back(_oid);
}


const OID& PublicRequestImpl::getObject(unsigned _idx) const
{
    return objects_.at(_idx);
}


unsigned PublicRequestImpl::getObjectSize() const
{
    return objects_.size();
}

const Database::ID PublicRequestImpl::getRegistrarId() const
{
    return registrar_id_;
}


const std::string PublicRequestImpl::getRegistrarHandle() const
{
    return registrar_handle_;
}


const std::string PublicRequestImpl::getRegistrarName() const
{
    return registrar_name_;
}


const std::string PublicRequestImpl::getRegistrarUrl() const
{
    return registrar_url_;
}


/// default destination emails for answer are from objects
std::string PublicRequestImpl::getEmails() const
{
    Database::SelectQuery sql;
    std::string emails;

    for (unsigned i=0; i<getObjectSize(); i++) {
      switch (getObject(i).type) {
        case OT_DOMAIN:
          sql.buffer()
            << "SELECT DISTINCT c.email FROM domain d, contact c "
            << "WHERE d.registrant=c.id AND d.id=" << getObject(i).id
            << " UNION "
            << "SELECT DISTINCT c.email "
            << "FROM domain_contact_map dcm, contact c "
            << "WHERE dcm.contactid=c.id AND dcm.role=1 "
            << "AND dcm.domainid=" << getObject(i).id;
          break;
        case OT_CONTACT:
          sql.buffer()
            << "SELECT DISTINCT c.email FROM contact c "
            << "WHERE c.id=" << getObject(i).id;
          break;
        case OT_NSSET:
          sql.buffer()
            << "SELECT DISTINCT c.email "
            << "FROM nsset_contact_map ncm, contact c "
            << "WHERE ncm.contactid=c.id AND ncm.nssetid=" << getObject(i).id;
          break;
        case OT_KEYSET:
          sql.buffer()
            << "SELECT DISTINCT c.email "
            << "FROM keyset_contact_map kcm, contact c "
            << "WHERE kcm.contactid=c.id AND kcm.keysetid="
            << getObject(i).id;
          break;
        case OT_UNKNOWN:
          break;
      };

      Database::Connection conn = Database::Manager::acquire();
      Database::Result r_emails = conn.exec(sql);
      for (Database::Result::Iterator it = r_emails.begin(); it != r_emails.end(); ++it) {
        emails += (std::string)(*it)[0] + (it == r_emails.end() - 1 ? "" : " ");
      }

    }
    LOGGER(PACKAGE).debug(boost::format("for request id=%1% -- found notification recipients '%2%'")
                             % id_ % emails);
    return emails;
  }


/// send email with answer and return its id
TID PublicRequestImpl::sendEmail() const
{
    Mailer::Parameters params;
    fillTemplateParams(params);
    Mailer::Handles handles;
    // TODO: insert handles of contacts recieving
    // email (RT_EPP&RT_AUTO_PIF)
    // TODO: object->email relation should not be handled in mail module
    for (unsigned i=0; i<getObjectSize(); i++)
      handles.push_back(getObject(i).handle);
    Mailer::Attachments attach; // they are empty
    return man_->getMailerManager()->sendEmail(
      "", // default sender from notification system
      getEmails(),
      "", // default subject is taken from template
      getTemplateName(),params,handles,attach
    ); // can throw Mailer::NOT_SEND exception
  }


/// concrete resolution action
void PublicRequestImpl::processAction(bool check)
{
    // default is to do nothing special
}


/// what to do if request is invalidated
void PublicRequestImpl::invalidateAction()
{
}


/// process request (or just close in case of invalid flag)
void PublicRequestImpl::process(
        bool invalid,
        bool check,
        const unsigned long long &_request_id)
{
      Database::Connection conn = Database::Manager::acquire();
      Database::Transaction tx(conn);

      // TODO: probably bug - shoud be in UTC time zone
      resolve_time_ = ptime(boost::posix_time::second_clock::local_time());
      resolve_request_id_ = Database::ID(_request_id);

      if (invalid) {
          status_ = PRS_INVALID;
          invalidateAction();
      }
      else {
          processAction(check);
          status_ = PRS_ANSWERED;
          answer_email_id_ = sendEmail();
      }
      save();

      tx.commit();
}


/// type for PDF letter template in case there is no template
unsigned PublicRequestImpl::getPDFType() const
{
    return 0;
}


/// function called inside save() to handle special behavior after create
void PublicRequestImpl::postCreate()
{
    // normally do nothing
}


Manager* PublicRequestImpl::getPublicRequestManager() const
{
    return man_;
}


PublicRequestAuthImpl::PublicRequestAuthImpl()
    : PublicRequestImpl(), authenticated_(false)
{
    identification_ = Random::string_alpha(32);
}


void PublicRequestAuthImpl::init(Database::Row::Iterator& _it)
{
    PublicRequestImpl::init(_it);
    identification_ = static_cast<std::string>(*(++_it));
    password_ = static_cast<std::string>(*(++_it));
}


std::string PublicRequestAuthImpl::getIdentification() const
{
    return identification_;
}


std::string PublicRequestAuthImpl::getPassword() const
{
    return password_;
}


bool PublicRequestAuthImpl::authenticate(const std::string &_password)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result rcheck = conn.exec_params(
            "SELECT password = $1::text FROM public_request_auth"
            " WHERE identification = $2::text",
            Database::query_param_list(_password)(identification_));
    if (rcheck.size() == 1 && static_cast<bool>(rcheck[0][0]) == true) {
        authenticated_ = true;
        return true;
    }
    return false;
}


void PublicRequestAuthImpl::save()
{
    if (id_) {
        PublicRequestImpl::save();
        /* update (no need) */
        return;
    }
    else {
        /* insert */

        /* need only one object for request (not necessary but for simplicity */
        if (this->getObjectSize() != 1) {
            throw std::runtime_error("public request auth requires"
                    " (only) one object to work with");
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        PublicRequestImpl::save();

        if (!id_) {
            throw std::runtime_error("cannot save authenticated public request"
                    " without base request (id not set)");
        }

        /* generate passwords */
        password_ = this->generatePasswords();
        conn.exec_params(
                "INSERT INTO public_request_auth (id, identification, password)"
                " VALUES ($1::integer, $2::text, $3::text)",
                Database::query_param_list
                    (id_)
                    (identification_)
                    (password_));
        tx.commit();
    }
}


void PublicRequestAuthImpl::process(
        bool _invalid,
        bool _check,
        const unsigned long long &_request_id)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);

    /* need to be authenticated */
    if (!authenticated_) {
        throw NotAuthenticated();
    }

    /* proces only new */
    if (status_ != PRS_NEW) {
        throw AlreadyProcessed(
                this->getId(),
                this->getStatus() == PRS_ANSWERED ? true : false);
    }

    resolve_time_ = ptime(boost::posix_time::second_clock::local_time());
    resolve_request_id_ = (_request_id != 0) ? Database::ID(_request_id)
                                             : this->getRequestId();

    if (_invalid) {
        status_ = PRS_INVALID;
    }
    else {
        processAction(_check);
        status_ = PRS_ANSWERED;
    }
    save();

    tx.commit();
}


/* just to be sure of empty impl (if someone would change base impl) */
void PublicRequestAuthImpl::postCreate()
{
}


/* don't use this methods for constucting email so far */
std::string PublicRequestAuthImpl::getTemplateName() const
{
    return std::string();
}


void PublicRequestAuthImpl::fillTemplateParams(Mailer::Parameters& params) const
{
}



bool PublicRequestAuthImpl::check() const
{
    return true;
}



}
}

