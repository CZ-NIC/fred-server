/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/libfred/public_request/public_request_impl.hh"
#include "util/log/logger.hh"
#include "util/util.hh"
#include "libfred/db_settings.hh"
#include "util/types/convert_sql_db_types.hh"
#include "util/types/sqlize.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"

#include <string>
#include <vector>
#include <algorithm>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>


namespace LibFred {
namespace PublicRequest {



std::string Status2Str(Status_PR _status)
{
    switch (_status)
    {
        case PRS_OPENED:
            return "Opened";
        case PRS_RESOLVED:
            return "Resolved";
        case PRS_INVALIDATED:
            return "Invalidated";
        default:
            return "STATUS UNKNOWN";
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
            return "Nsset";
        case OT_DOMAIN:
            return "Domain";
        case OT_KEYSET:
            return "Keyset";
        default:
            return "Type unknown";
    }
}


void insertNewStateRequest(
        Database::ID blockRequestID,
        Database::ID objectId,
        const std::string & state_name)
{
    insert_object_state(objectId, state_name);
}


/*
 * check if object states interfere with requested states
 */
bool queryBlockRequest(
        Database::ID objectId,
        Database::ID blockRequestID,
        const std::vector<std::string>& states_vect,
        bool unblock)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);

    //lock public_request
    if (blockRequestID) lock_public_request_id(blockRequestID);

    //already blocked
    if(object_has_one_of_states(objectId,Util::vector_of<std::string>(ObjectState::MOJEID_CONTACT)(ObjectState::SERVER_BLOCKED))) return false;

    if(unblock)//unblocking
    {
        if(!object_has_all_of_states(objectId,states_vect)) return false;//not blocked yet case 03, 04, 07, 08, 11, 12, 16

        if(std::find(states_vect.begin(),states_vect.end(),ObjectState::SERVER_TRANSFER_PROHIBITED) != states_vect.end()//unblocking SERVER_TRANSFER_PROHIBITED
            && std::find(states_vect.begin(),states_vect.end(),ObjectState::SERVER_UPDATE_PROHIBITED) == states_vect.end()//but not SERVER_UPDATE_PROHIBITED
            && object_has_state(objectId,ObjectState::SERVER_UPDATE_PROHIBITED)) return false; //case 19

        //return true case 15, 20
    }
    else //blocking
    {
        if(object_has_all_of_states(objectId,states_vect)) return false;//already blocked case 13, 17, 18

        if(object_has_one_of_states(objectId,states_vect) && blockRequestID != 0) //cancel previous requests case 14
        {//cancel all states
            for (std::vector<std::string>::const_iterator it = states_vect.begin()
                ; it != states_vect.end(); ++it)
            {
                LibFred::cancel_object_state(objectId, *it);
            }
        }

        //return true case 01, 02, 05, 06, 09, 10, 14
    }

  tx.commit();
  return true;
}


/* check if object has given request type already opened */
unsigned long long check_public_request(
        const unsigned long long &_object_id,
        const Type &_type)
{
    Database::Connection conn = Database::Manager::acquire();

    //get lock to the end of transaction for given object and request type
    lock_public_request_by_object(_object_id);

    Database::Result rcheck = conn.exec_params(
            "SELECT pr.id FROM public_request pr"
            " JOIN public_request_objects_map prom ON prom.request_id = pr.id"
            " JOIN enum_public_request_type eprt ON eprt.id = pr.request_type"
            " WHERE pr.resolve_time IS NULL"
            " AND prom.object_id = $1::integer"
            " AND eprt.name = $2::varchar"
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
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long prid = 0;
    if ((prid = check_public_request(_object_id, _type)) != 0) {

        conn.exec_params("UPDATE public_request SET resolve_time = now(),"
                " status = $1::integer, resolve_request_id = $2::bigint"
                " WHERE id = $3::integer",
                Database::query_param_list
                    (PRS_INVALIDATED)
                    (_request_id != 0 ? _request_id : Database::QPNull)
                    (prid));

        /* if there are associated letters for invalidated public request ... */
        Database::Result letter_ids = conn.exec_params(
            "SELECT la.id "
                "FROM "
                    "public_request pr "
                    "JOIN enum_public_request_type eprt ON eprt.id = pr.request_type "
                    "JOIN public_request_objects_map prom ON prom.request_id = pr.id "
                    "JOIN public_request_messages_map prmm ON prmm.public_request_id = pr.id "
                    "JOIN letter_archive la ON la.id = prmm.message_archive_id "
                "WHERE "
                    "prom.object_id = $1::bigint "
                    "AND pr.id = $2::bigint "
                    "AND eprt.name = $3::varchar",
            Database::query_param_list(_object_id)(prid)(_type));

        /* ... we also try to cancel send */
        LibFred::Messages::ManagerPtr msg_mgr = LibFred::Messages::create_manager();
        for (Database::Result::size_type i = 0; i < letter_ids.size(); ++i)
        {
            msg_mgr->cancel_letter_send(static_cast<unsigned long long>(letter_ids[i]["id"]));
        }
    }
}


bool object_was_changed_since_request_create(const unsigned long long _request_id)
{
    Database::Connection conn = Database::Manager::acquire();
    lock_public_request_id(_request_id);
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
      resolve_request_id_(0), status_(PRS_OPENED), answer_email_id_(0),
      registrar_id_(0), man_()
{
}


PublicRequestImpl::PublicRequestImpl(
        Database::ID _id,
        LibFred::PublicRequest::Type _type,
        Database::ID _create_request_id,
        Database::DateTime _create_time,
        LibFred::PublicRequest::Status_PR _status,
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
    status_           = (LibFred::PublicRequest::Status_PR)(int)*(++_it);
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
    TRACE("[CALL] LibFred::Request::RequestImpl::save()");
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

        LOGGER.info(boost::format("request id='%1%' updated successfully -- %2%") %
                          id_ % (status_ == PRS_INVALIDATED ? "invalidated" : "resolved"));
      }
      catch (Database::Exception& ex) {
        LOGGER.error(boost::format("%1%") % ex.what());
        throw;
      }
      catch (std::exception& ex) {
        LOGGER.error(boost::format("%1%") % ex.what());
        throw;
      }

    }
    else {
      try {
        Database::Transaction transaction(conn);

        for(std::vector<OID>::size_type i = 0 ; i < objects_.size(); ++i)
        {
           lock_public_request_by_object(objects_.at(i).id);
        }

        conn.exec_params(
                "INSERT INTO public_request"
                " (request_type, create_request_id,"
                " status, reason, email_to_answer, registrar_id)"
                " VALUES"
                " ((SELECT id FROM enum_public_request_type WHERE name = $1::varchar),"
                " $2::bigint, $3::bigint, $4::varchar, $5::varchar, $6::integer)",
                Database::query_param_list
                    (type_)(create_request_id_)(status_)
                    (reason_)(email_to_answer_)(registrar_id_ == 0 ? Database::QPNull : registrar_id_));

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
        LOGGER.info(boost::format("request id='%1%' for objects={%2%} created successfully")
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
        LOGGER.error(boost::format("%1%") % ex.what());
        throw;
      }
      catch (std::exception& ex) {
        LOGGER.error(boost::format("%1%") % ex.what());
        throw;
      }
    }
}


LibFred::PublicRequest::Type PublicRequestImpl::getType() const
{
    return type_;
}



void PublicRequestImpl::setType(LibFred::PublicRequest::Type _type)
{
    type_ = _type;
    modified_ = true;
}


LibFred::PublicRequest::Status_PR PublicRequestImpl::getStatus() const
{
    return status_;
}


void PublicRequestImpl::setStatus(LibFred::PublicRequest::Status_PR _status)
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
    LOGGER.debug(boost::format("for request id=%1% -- found notification recipients '%2%'")
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


/// process request (or just close in case of invalidated flag)
void PublicRequestImpl::process(
        bool invalidated,
        bool check,
        const unsigned long long &_request_id)
{
      Database::Connection conn = Database::Manager::acquire();
      Database::Transaction tx(conn);

      // TODO: probably bug - shoud be in UTC time zone
      resolve_time_ = ptime(boost::posix_time::second_clock::local_time());
      resolve_request_id_ = Database::ID(_request_id);

      if (invalidated) {
          status_ = PRS_INVALIDATED;
          invalidateAction();
      }
      else {
          processAction(check);
          status_ = PRS_RESOLVED;
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
    identification_ = Random::Generator().get_seq(Random::CharSet::letters(), 32);
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
        bool _invalidated,
        bool _check,
        const unsigned long long &_request_id)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);

    /* need to be authenticated */
    if (!authenticated_) {
        throw NotAuthenticated();
    }

    /* process only opened */
    if (status_ != PRS_OPENED) {
        throw AlreadyProcessed(
                this->getId(),
                this->getStatus() == PRS_RESOLVED ? true : false);
    }

    resolve_time_ = ptime(boost::posix_time::second_clock::local_time());
    resolve_request_id_ = (_request_id != 0) ? Database::ID(_request_id)
                                             : this->getRequestId();

    if (_invalidated) {
        status_ = PRS_INVALIDATED;
    }
    else {
        processAction(_check);
        status_ = PRS_RESOLVED;
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

