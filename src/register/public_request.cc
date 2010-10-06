
#include <boost/utility.hpp>

#include "common_impl.h"
#include "public_request.h"
#include "log/logger.h"
#include "util.h"
#include "db_settings.h"
#include "types/convert_sql_db_types.h"
#include "types/sqlize.h"
#include "random.h"
#include <boost/lexical_cast.hpp>


namespace Register {
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
    default:                              return "TYPE UNKNOWN";
  }
}


std::string Status2Str(Status _status) {
  switch (_status) {
    case PRS_NEW:       return "New";
    case PRS_ANSWERED:  return "Answered";
    case PRS_INVALID:   return "Invalidated";
    default:            return "STATUS UNKNOWN";
  }
}

std::string
ObjectType2Str(ObjectType type)
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


static bool checkState(Database::ID objectId, 
                       unsigned state) {
  Database::Query sql;
  sql.buffer() << "SELECT COUNT(*) FROM object_state WHERE state_id=" 
               << state << " AND object_id=" << objectId 
               << " AND valid_to ISNULL";
  Database::Connection conn = Database::Manager::acquire();
  Database::Result r_objects = conn.exec(sql);
  int res = (r_objects.size() == 0 ? 0 : (int)r_objects[0][0]);
  return res > 0;
}

// static Database::ID insertNewStateRequest(
static void insertNewStateRequest(Database::ID blockRequestID, 
                                  Database::ID objectId,
                                  unsigned state) {
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

/** check if already blocked request interfere with requested states
 this is true in every situation other then when actual states are
 smaller subset of requested states. if requested by setting parameter 
 blockRequestID, all states in subset are closed */
static bool queryBlockRequest(Database::ID objectId, 
                              Database::ID blockRequestID, 
                              const std::string& states, 
                              bool unblock) {
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
 
class PublicRequestImpl : public Register::CommonObjectImpl,
                          virtual public PublicRequest {
protected:
  Register::PublicRequest::Type type_;
  Database::ID epp_action_id_;
  Database::ID logd_request_id_;
  Database::DateTime create_time_;
  Register::PublicRequest::Status status_;
  Database::DateTime resolve_time_;
  std::string reason_;
  std::string email_to_answer_;
  Database::ID answer_email_id_;

  std::string svtrid_;
  Database::ID registrar_id_;
  std::string registrar_handle_;
  std::string registrar_name_;
  std::string registrar_url_;


  std::vector<OID> objects_;
  
protected:
  Manager* man_;
  
public:
  PublicRequestImpl() : CommonObjectImpl(0), type_(), 
      epp_action_id_(0), logd_request_id_(0), status_(PRS_NEW), answer_email_id_(0), registrar_id_(0), man_() {
  }
  
  PublicRequestImpl(Database::ID _id,
              Register::PublicRequest::Type _type,
              Database::ID _epp_action_id,
              Database::ID _logd_request_id,
              Database::DateTime _create_time,
              Register::PublicRequest::Status _status,
              Database::DateTime _resolve_time,
              std::string _reason,
              std::string _email_to_answer,
              Database::ID _answer_email_id,
              std::string _svtrid,
              Database::ID _registrar_id,
              std::string _registrar_handle,
              std::string _registrar_name,
              std::string _registrar_url
              ) :
              CommonObjectImpl(_id), type_(_type), epp_action_id_(_epp_action_id), 
              logd_request_id_(_logd_request_id),
              create_time_(_create_time), status_(_status),
              resolve_time_(_resolve_time), reason_(_reason), 
              email_to_answer_(_email_to_answer), answer_email_id_(_answer_email_id), 
              svtrid_(_svtrid), registrar_id_(_registrar_id), 
              registrar_handle_(_registrar_handle), 
              registrar_name_(_registrar_name), registrar_url_(_registrar_url),
              man_() {
  }
  
  void setManager(Manager* _man) {
    man_ = _man;
  }
  
  virtual void init(Database::Row::Iterator& _it) {
    id_               = (unsigned long long)*_it;
    epp_action_id_    = *(++_it);
    logd_request_id_  = *(++_it);
    create_time_      = *(++_it);
    status_           = (Register::PublicRequest::Status)(int)*(++_it);
    resolve_time_     = *(++_it);
    reason_           = (std::string)*(++_it);
    email_to_answer_  = (std::string)*(++_it);
    answer_email_id_  = *(++_it);
    svtrid_           = (std::string)*(++_it);
    registrar_id_     = *(++_it);
    registrar_handle_ = (std::string)*(++_it);
    registrar_name_   = (std::string)*(++_it);
    registrar_url_    = (std::string)*(++_it);
  }

  virtual void save() {
    TRACE("[CALL] Register::Request::RequestImpl::save()");
    if (objects_.empty()) {
      LOGGER(PACKAGE).error("can't create or update request with no object specified!");
      throw;
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
        if(logd_request_id_) {
            update_request.buffer()  << ", logd_request_id = " << logd_request_id_;
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
      insert_request.add("epp_action_id", epp_action_id_);
      insert_request.add("logd_request_id", logd_request_id_);
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
    
  virtual Register::PublicRequest::Type getType() const {
    return type_;
  }
  
  virtual void setType(Register::PublicRequest::Type _type) {
    type_ = _type;
    modified_ = true;
  }
  
  virtual Register::PublicRequest::Status getStatus() const {
    return status_;
  }
  
  virtual void setStatus(Register::PublicRequest::Status _status) {
   status_ = _status;
   modified_ = true;
  }
  
  virtual ptime getCreateTime() const {
    return create_time_;
  }
  
  virtual ptime getResolveTime() const {
    return resolve_time_;
  }
  
  virtual const std::string& getReason() const {
    return reason_;
  }
  
  virtual void setReason(const std::string& _reason) {
    reason_ = _reason;
    modified_ = true;
  }
  
  virtual const std::string& getEmailToAnswer() const {
    return email_to_answer_;
  }
  
  virtual void setEmailToAnswer(const std::string& _email) {
    email_to_answer_ = _email;
    modified_ = true;
  }
  
  virtual const Database::ID getAnswerEmailId() const {
    return answer_email_id_;
  }
  
  virtual const Database::ID getEppActionId() const {
    return epp_action_id_;
  }

  virtual void setEppActionId(const Database::ID& _epp_action_id) {
    epp_action_id_ = _epp_action_id;
    modified_ = true;
  }

  virtual const Database::ID getRequestId() const {
    return logd_request_id_;
  }

  virtual void setRequestId(const Database::ID& _logd_request_id) {
    logd_request_id_ = _logd_request_id;
    modified_ = true;
  }
  
  virtual void setRegistrarId(const Database::ID& _registrar_id) {
    registrar_id_ = _registrar_id;
    modified_ = true;
  }

  
  virtual void addObject(const OID& _oid) {
    objects_.push_back(_oid);
  }
  
  virtual const OID& getObject(unsigned _idx) const {
    return objects_.at(_idx);
  }
  
  virtual unsigned getObjectSize() const {
    return objects_.size();
  }
  
  
  virtual const std::string getSvTRID() const {
    return svtrid_;
  }

  virtual const Database::ID getRegistrarId() const {
    return registrar_id_;
  }
  
  virtual const std::string getRegistrarHandle() const {
    return registrar_handle_;
  }
  
  virtual const std::string getRegistrarName() const {
    return registrar_name_;
  }
  
  virtual const std::string getRegistrarUrl() const {
    return registrar_url_;
  } 

  /// default destination emails for answer are from objects 
  virtual std::string getEmails() const {    
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
  virtual TID sendEmail() const throw (Mailer::NOT_SEND) {
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
  virtual void processAction(bool check) {
    // default is to do nothing special
  }

  /// process request (or just close in case of invalid flag)
  virtual void process(bool invalid, bool check) {
    if (invalid) {
      status_ = PRS_INVALID;
    }
    else {
    	processAction(check);
      status_ = PRS_ANSWERED;
      answer_email_id_ = sendEmail();
    }
    resolve_time_ = ptime(boost::posix_time::second_clock::local_time());
    save();
  }

  /// type for PDF letter template in case there is no template
  virtual unsigned getPDFType() const {
    return 0;
  }
  /// function called inside save() to handle special behavior after create
  virtual void postCreate() {
    // normally do nothing
  }
};

COMPARE_CLASS_IMPL(PublicRequestImpl, CreateTime)
COMPARE_CLASS_IMPL(PublicRequestImpl, ResolveTime)
COMPARE_CLASS_IMPL(PublicRequestImpl, Type)
COMPARE_CLASS_IMPL(PublicRequestImpl, Status)

#define SERVER_TRANSFER_PROHIBITED 3
#define SERVER_UPDATE_PROHIBITED 4

class AuthInfoRequestImpl : public PublicRequestImpl {
public:
  /// check if object exists & no serverTransferProhibited
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = !checkState(getObject(i).id,SERVER_TRANSFER_PROHIBITED);
    return res;
  }
  virtual void processAction(bool _check) {
    if (_check && !check()) throw REQUEST_BLOCKED();
  }
  std::string getAuthInfo() const throw (Database::Exception) {
    // just one object is supported
    if (!getObjectSize() || getObjectSize() > 1) return "";
    Database::SelectQuery sql;
    sql.buffer() << "SELECT o.AuthInfoPw "
                 << "FROM object o "
                 << "WHERE o.id=" << getObject(0).id;
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec(sql);
    if (!res.size()) {
      return "";
    }
    return (*res.begin())[0];
  }
  /// fill mail template with common param for authinfo requeste templates
  virtual void fillTemplateParams(Mailer::Parameters& params) const {    
    std::ostringstream buf;
    buf << getRegistrarName() << " (" << getRegistrarUrl() << ")";
    params["registrar"] = buf.str();
    buf.str("");
    buf.imbue(std::locale(std::locale(""),new date_facet("%x")));
    buf << getCreateTime().date();
    params["reqdate"] = buf.str();
    buf.str("");
    buf << getId();
    params["reqid"] = buf.str();
    if (getObjectSize()) {
      buf.str("");  
      buf << getObject(0).type;
      params["type"] = buf.str();          
      params["handle"] = getObject(0).handle;
    }
    params["authinfo"] = getAuthInfo();    
  }
};

class AuthInfoRequestEPPImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_epp";
  }
  /// after creating, this type of request is processed
  virtual void postCreate() {
  	// cannot call process(), object need to be loaded completly
    man_->processRequest(getId(),false,false);
  }
};

class AuthInfoRequestPIFAutoImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_pif";
  }
  /// after creating, this type of request is processed
  virtual void postCreate() {
  	// cannot call process(), object need to be loaded completly
    man_->processRequest(getId(),false,false);
  }
};

class AuthInfoRequestPIFEmailImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_pif";
  }
  /// answer is sent to requested email
  std::string getEmails() const {
    return getEmailToAnswer();
  }
};

class AuthInfoRequestPIFPostImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_pif";
  }
  /// answer is sent to requested email
  std::string getEmails() const {
    return getEmailToAnswer();
  }
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 1;
  }
};

class BlockUnblockRequestPIFImpl : public PublicRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "request_block";
  }
  virtual short blockType() const = 0;
  virtual short blockAction() const = 0;
  virtual void fillTemplateParams(Mailer::Parameters& params) const {    
    std::ostringstream buf;
    buf.imbue(std::locale(std::locale(""),new date_facet("%x")));
    buf << getCreateTime().date();
    params["reqdate"] = buf.str();
    buf.str("");
    buf << getId();
    params["reqid"] = buf.str();
    if (getObjectSize()) {
      buf.str("");  
      buf << getObject(0).type;
      params["type"] = buf.str();          
      params["handle"] = getObject(0).handle;
    }
    buf.str("");
    buf << blockType();
    params["otype"] = buf.str();
    buf.str("");
    buf << blockAction();
    params["rtype"] = buf.str();
  }
};

class BlockTransferRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = !checkState(getObject(i).id,SERVER_UPDATE_PROHIBITED) &&
      queryBlockRequest(getObject(i).id,0,"3",false);
    return res;    
  }
  virtual short blockType() const {
  	return 1;
  }
  virtual short blockAction() const {
  	return 2;
  }
	virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
    	if (!checkState(getObject(i).id, SERVER_UPDATE_PROHIBITED) &&
    	      queryBlockRequest(getObject(i).id, getId(), "3", false)) {
    		insertNewStateRequest(getId(),getObject(i).id, 3);
    	} else throw REQUEST_BLOCKED();

      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      conn.exec(q);
    }
	}
};

class BlockUpdateRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i = 0; res && i < getObjectSize(); i++)
     res = !checkState(getObject(i).id, SERVER_UPDATE_PROHIBITED) &&
      queryBlockRequest(getObject(i).id, 0, "3,4", false);
    return res;    
  }
  virtual short blockType() const {
  	return 1;
  }
  virtual short blockAction() const {
  	return 1;
  }
	virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
    	if (!checkState(getObject(i).id, SERVER_UPDATE_PROHIBITED) &&
    	      queryBlockRequest(getObject(i).id, getId(), "3,4", false)) {
    		insertNewStateRequest(getId(),getObject(i).id, SERVER_TRANSFER_PROHIBITED);
    		insertNewStateRequest(getId(),getObject(i).id, SERVER_UPDATE_PROHIBITED);
    	} else throw REQUEST_BLOCKED();
      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      conn.exec(q);
    }
	}
};

class UnBlockTransferRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id,0,"3",true);
    return res;    
  }
  virtual short blockType() const {
  	return 2;
  }
  virtual short blockAction() const {
  	return 2;
  }
	virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
    	if (!queryBlockRequest(getObject(i).id, getId(), "3", true))
        throw REQUEST_BLOCKED();
      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      conn.exec(q);
    }
	}
};

class UnBlockUpdateRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i = 0; res && i < getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id, 0, "3,4", true);
    return res;    
  }
  virtual short blockType() const {
  	return 2;
  }
  virtual short blockAction() const {
  	return 1;
  }
	virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
    	if (!queryBlockRequest(getObject(i).id, getId(), "3,4", true))
        throw REQUEST_BLOCKED();
      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      conn.exec(q);
    }
	}
};

class BlockTransferRequestPIFPostImpl : public BlockTransferRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 2;
  }
};

class BlockUpdateRequestPIFPostImpl : public BlockUpdateRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 4;
  }
};

class UnBlockTransferRequestPIFPostImpl : public UnBlockTransferRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 3;
  }
};

class UnBlockUpdateRequestPIFPostImpl : public UnBlockUpdateRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 5;
  }
};


class PublicRequestAuthImpl
    : virtual public PublicRequestAuth,
      public PublicRequestImpl
{
protected:
    bool authenticated_;
    std::string identification_;
    std::string password_;


public:
    PublicRequestAuthImpl()
        : PublicRequestImpl(),
          authenticated_(false)
    {
    }

    virtual ~PublicRequestAuthImpl()
    {
    }

    virtual void init(Database::Row::Iterator& _it)
    {
        PublicRequestImpl::init(_it);
        identification_ = static_cast<std::string>(*(++_it));
        password_ = static_cast<std::string>(*(++_it));
    }

    bool authenticate(const std::string &_password)
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

    void save()
    {

        if (id_) {
            PublicRequestImpl::save();
            /* don't need update */
            return;
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        PublicRequestImpl::save();

        if (!id_) {
            throw std::runtime_error("cannot save authenticated public request"
                    " without base request (id not set)");
        }

        identification_ = Random::string_alpha(32);
        password_ = Random::string_alpha(32);

        conn.exec_params(
                "INSERT INTO public_request_auth (id, identification, password)"
                " VALUES ($1::integer, $2::text, $3::text)",
                Database::query_param_list
                    (id_)
                    (identification_)
                    (password_));
        tx.commit();
    }

    void process(bool _invalid, bool _check)
    {
        if (status_ != PRS_NEW) {
            throw std::runtime_error("already processed");
        }
        if (authenticated_) {
            if (_invalid) {
                status_ = PRS_INVALID;
            }
            else {
                processAction(_check);
                status_ = PRS_ANSWERED;
            }
            resolve_time_ = ptime(boost::posix_time::second_clock::local_time());
            save();
        }
        else {
            throw NOT_AUTHENTICATED();
        }
    }

    /* just to be sure of empty impl (if someone would change base impl) */
    virtual void postCreate() { }

    /* don't use this methods for constucting email so far */
    std::string getTemplateName() const
    {
        return std::string();
    }

    void fillTemplateParams(Mailer::Parameters& params) const
    {
    }

    /* helper method for sending passwords */
    void sendPasswords(const std::string &_template, const std::string &_type)
    {
        LOGGER(PACKAGE).debug("public request auth - send pasword");

        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
                "SELECT create_time FROM public_request WHERE id = $1::integer",
                Database::query_param_list(id_));
        if (res.size() == 1)
            create_time_ = res[0][0];
        else
            throw std::runtime_error("unable to find public request");

        Mailer::Attachments attach;
        Mailer::Handles handles;
        Mailer::Parameters params;

        handles.push_back(getObject(0).handle);

        std::ostringstream buf;
        buf.imbue(std::locale(std::locale(""), new date_facet("%x")));
        buf << getCreateTime().date();
        params["reqdate"] = buf.str();
        params["rtype"] = _type;
        params["handle"] = getObject(0).handle;
        params["passwd"] = password_;

        unsigned long long id = man_->getMailerManager()->sendEmail(
                "",           /* default sender */
                getEmails(),
                "",           /* default subject */
                _template,
                params,
                handles,
                attach
                );
    }
};


class ConditionalContactIdentificationImpl
    : public PublicRequestAuthImpl
{
public:
    bool check() const
    {
        return true;
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
                % getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        insertNewStateRequest(getId(), getObject(0).id, 21);
        conn.exec_params(
                "SELECT update_object_states($1::integer)",
                Database::query_param_list(getObject(0).id));
        tx.commit();
    }

    void sendPasswords()
    {
        PublicRequestAuthImpl::sendPasswords("mojeid_identification", "1");
    }
};


class ContactIdentificationImpl
    : public PublicRequestAuthImpl
{
public:
    bool check() const
    {
        return true;
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
                % getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        insertNewStateRequest(getId(), getObject(0).id, 22);
        conn.exec_params(
                "SELECT update_object_states($1::integer)",
                Database::query_param_list(getObject(0).id));
        tx.commit();
    }

    void sendPasswords()
    {
        PublicRequestAuthImpl::sendPasswords("mojeid_identification", "2");
    }
};



class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;

public:
  ListImpl(Manager *_manager) : CommonListImpl(),
                                                             manager_(_manager) {
  }

  virtual ~ListImpl() {
  }

  virtual const char* getTempTableName() const {
    return "tmp_public_request_filter_result";
  }

  virtual PublicRequest* get(unsigned _idx) const {
    try {
      PublicRequest *request = dynamic_cast<PublicRequest* >(data_.at(_idx));
      if (request)
        return request;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    }
  }

  virtual void reload(Database::Filters::Union& _filter) {
    TRACE("[CALL] Register::Request::ListImpl::reload()");
    clear();
    _filter.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::PublicRequest *rf = dynamic_cast<Database::Filters::PublicRequest* >(*fit);
      if (!rf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("id", rf->joinRequestTable(), "DISTINCT"));
      _filter.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    id_query.order_by() << "id DESC";
    id_query.limit(load_limit_);
    _filter.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select() << "t_1.request_type, t_1.id, t_1.epp_action_id, t_1.logd_request_id, "
                               << "t_1.create_time, t_1.status, t_1.resolve_time, "
                               << "t_1.reason, t_1.email_to_answer, t_1.answer_email_id, "
                               << "'ccReg-' || to_char(t_1.epp_action_id, 'FM0999999999'), t_4.id, t_4.handle, t_4.name, t_4.url, "
                               << "t_5.identification, t_5.password";
    object_info_query.from() << getTempTableName() << " tmp "
                             << "JOIN public_request t_1 ON (t_1.id = tmp.id) "
                             << "LEFT JOIN registrar t_4 ON (t_1.registrar_id = t_4.id) "
                             << "LEFT JOIN public_request_auth t_5 ON (t_5.id = t_1.id) ";
    object_info_query.order_by() << "t_1.id";

    Database::Connection conn = Database::Manager::acquire();
    try {
      fillTempTable(tmp_table_query);

      Database::Result r_info = conn.exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Register::PublicRequest::Type type = (Register::PublicRequest::Type)(int)*col;
        PublicRequest* request = manager_->createRequest(type);
        request->init(++col);
        data_.push_back(request);
      }

      if (data_.empty())
        return;

      /*
       * load objects details for requests
       */
      resetIDSequence();

      Database::SelectQuery objects_query;
      objects_query.select() << "tmp.id, t_1.object_id, t_2.name, t_2.type";
      objects_query.from() << getTempTableName() << " tmp "
                           << "JOIN public_request_objects_map t_1 ON (tmp.id = t_1.request_id) "
                           << "JOIN object_registry t_2 ON (t_1.object_id = t_2.id)";
      objects_query.order_by() << "tmp.id";

      Database::Result r_objects = conn.exec(objects_query);
      for (Database::Result::Iterator it = r_objects.begin(); it != r_objects.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID request_id    = *col;
        Database::ID object_id     = *(++col);
        std::string  object_handle = *(++col);
        ObjectType   object_type   = (ObjectType)(int)*(++col);

        PublicRequestImpl *request_ptr = dynamic_cast<PublicRequestImpl *>(findIDSequence(request_id));
        if (request_ptr)
          request_ptr->addObject(OID(object_id, object_handle, object_type));
      }
      /* checks if row number result load limit is active and set flag */
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }


  }

  virtual void sort(MemberType _member, bool _asc) {
    switch(_member) {
      case MT_CRDATE:
        stable_sort(data_.begin(), data_.end(), CompareCreateTime(_asc));
	break;
      case MT_RDATE:
        stable_sort(data_.begin(), data_.end(), CompareResolveTime(_asc));
	break;
      case MT_TYPE:
        stable_sort(data_.begin(), data_.end(), CompareType(_asc));
	break;
      case MT_STATUS:
        stable_sort(data_.begin(), data_.end(), CompareStatus(_asc));
	break;
    }
  }

  virtual void reload() {
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
  }
};


class ManagerImpl : virtual public Manager {
private:
  Domain::Manager   *domain_manager_;
  Contact::Manager  *contact_manager_;
  NSSet::Manager    *nsset_manager_;
  KeySet::Manager   *keyset_manager_;
  Mailer::Manager   *mailer_manager_;
  Document::Manager *doc_manager_;


public:
  ManagerImpl(Domain::Manager   *_domain_manager,
              Contact::Manager  *_contact_manager,
              NSSet::Manager    *_nsset_manager,
              KeySet::Manager   *_keyset_manager,
              Mailer::Manager   *_mailer_manager,
              Document::Manager *_doc_manager) :
              domain_manager_(_domain_manager),
              contact_manager_(_contact_manager),
              nsset_manager_(_nsset_manager),
              keyset_manager_(_keyset_manager),
              mailer_manager_(_mailer_manager),
              doc_manager_(_doc_manager) {
  }

  virtual ~ManagerImpl() {
  }

  virtual Mailer::Manager* getMailerManager() const {
    return mailer_manager_;
  }

  virtual List* createList() const {
    TRACE("[CALL] Register::Request::Manager::createList()");
    /*
     * can't use this way; connection will not be properly closed when
     * List is destroyed. Also can't closing connection in CommonLisrImpl
     * because it can be passed as existing connection and then used after
     * List destruction.
     */
    // return new ListImpl(db_manager_->getConnection(), (Manager *)this);
    return new ListImpl((Manager *)this);
  }

  virtual void getPdf(Database::ID _id,
                      const std::string& _lang,
                      std::ostream& _output) const
    throw (NOT_FOUND, SQL_ERROR, Document::Generator::ERROR) {
    TRACE(boost::format("[CALL] Register::Request::Manager::getPdf(%1%, '%2%')") %
          _id % _lang);
    std::auto_ptr<List> l(loadRequest(_id));
    PublicRequest* p = l->get(0);
    std::auto_ptr<Document::Generator> g(
      doc_manager_->createOutputGenerator(
        Document::GT_PUBLIC_REQUEST_PDF,
        _output,
        _lang
      )
    );
    g->getInput().imbue(std::locale(std::locale(""),new date_facet("%x")));
    g->getInput() << "<?xml version='1.0' encoding='utf-8'?>"
                  << "<enum_whois>"
                  << "<public_request>"
                  << "<type>" << p->getPDFType() << "</type>"
                  << "<handle type='"
                  << (p->getObjectSize() ? p->getObject(0).type : 0)
                  << "'>"
                  << (p->getObjectSize() ? p->getObject(0).handle : "")
                  << "</handle>"
                  << "<date>"
                  << p->getCreateTime().date()
                  << "</date>"
                  << "<id>"
                  << p->getId()
                  << "</id>"
                  << "<replymail>"
                  << p->getEmailToAnswer()
                  << "</replymail>"
                  << "</public_request>"
                  << "</enum_whois>";
    g->closeInput();
  }

  virtual PublicRequest* createRequest(
    Type _type
  ) const throw (NOT_FOUND, SQL_ERROR, Mailer::NOT_SEND, REQUEST_BLOCKED) {
    // TRACE("[CALL] Register::Request::Manager::createRequest()");
    PublicRequestImpl *request = 0;
    switch(_type) {
      case PRT_AUTHINFO_AUTO_RIF :
        request = new AuthInfoRequestEPPImpl(); break;
      case PRT_AUTHINFO_AUTO_PIF :
        request = new AuthInfoRequestPIFAutoImpl(); break;
      case PRT_AUTHINFO_EMAIL_PIF :
        request = new AuthInfoRequestPIFEmailImpl(); break;
      case PRT_AUTHINFO_POST_PIF :
        request = new AuthInfoRequestPIFPostImpl(); break;
      case PRT_BLOCK_TRANSFER_EMAIL_PIF :
        request = new BlockTransferRequestPIFImpl(); break;
      case PRT_BLOCK_CHANGES_EMAIL_PIF :
        request = new BlockUpdateRequestPIFImpl(); break;
      case PRT_UNBLOCK_TRANSFER_EMAIL_PIF :
        request = new UnBlockTransferRequestPIFImpl(); break;
      case PRT_UNBLOCK_CHANGES_EMAIL_PIF :
        request = new UnBlockUpdateRequestPIFImpl(); break;
      case PRT_BLOCK_TRANSFER_POST_PIF :
        request = new BlockTransferRequestPIFPostImpl(); break;
      case PRT_BLOCK_CHANGES_POST_PIF :
        request = new BlockUpdateRequestPIFPostImpl(); break;
      case PRT_UNBLOCK_TRANSFER_POST_PIF :
        request = new UnBlockTransferRequestPIFPostImpl(); break;
      case PRT_UNBLOCK_CHANGES_POST_PIF :
        request = new UnBlockUpdateRequestPIFPostImpl(); break;
      case PRT_CONDITIONAL_CONTACT_IDENTIFICATION:
        request = new ConditionalContactIdentificationImpl(); break;
      case PRT_CONTACT_IDENTIFICATION:
        request = new ContactIdentificationImpl(); break;
      case PRT_CONTACT_VALIDATION:
        throw std::runtime_error("not implemented"); break;
    }
    request->setType(_type);
    request->setManager((Manager *)this);
    return request;
  }

  List *loadRequest(Database::ID id) const throw (NOT_FOUND) {
    Database::Filters::PublicRequest *prf = new Database::Filters::PublicRequestImpl();
    prf->addId().setValue(id);
    Database::Filters::Union uf;
    uf.addFilter(prf);
    List *l = createList();
    l->reload(uf);
    if (l->getCount() != 1) throw NOT_FOUND();
    return l;
  }

  virtual void processRequest(Database::ID _id, bool _invalidate,
                              bool check) const
    throw (NOT_FOUND, SQL_ERROR, Mailer::NOT_SEND, REQUEST_BLOCKED) {
    TRACE(boost::format("[CALL] Register::Request::Manager::processRequest(%1%, %2%)") %
          _id % _invalidate);
    try {
      std::auto_ptr<List> l(loadRequest(_id));
      l->get(0)->process(_invalidate,check);
    }
    catch (Database::Exception) { throw SQL_ERROR(); }
  }

  virtual unsigned long long processAuthRequest(
          const std::string &_identification,
          const std::string &_password)
  {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result rid = conn.exec_params(
              "SELECT id FROM public_request_auth WHERE identification = $1::text",
              Database::query_param_list(_identification));
      if (rid.size() != 1)
          throw NOT_FOUND();

      unsigned long long id = rid[0][0];
      std::auto_ptr<List> list(loadRequest(id));
      PublicRequestAuth *request = dynamic_cast<PublicRequestAuth*>(list->get(0));
      if (!request) {
          throw NOT_FOUND();
      }

      request->authenticate(_password);
      request->process(false, true);
      return request->getObject(0).id;
  }

  virtual std::string getIdentification(
          Database::ID _contact_id)
  {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result rid = conn.exec_params(
              "SELECT identification FROM public_request_auth pra"
              " JOIN public_request pr ON (pra.id=pr.id)"
              " JOIN public_request_objects_map prom ON (prom.request_id=pr.id)"
              " WHERE pr.resolve_time IS NULL AND pr.status = 0"
              " AND object_id = $1::integer",
              Database::query_param_list(_contact_id));
      if (rid.size() != 1)
          throw NOT_FOUND();

      return rid[0][0];
  }
};

Manager* Manager::create(Domain::Manager    *_domain_manager,
                         Contact::Manager   *_contact_manager,
                         NSSet::Manager     *_nsset_manager,
                         KeySet::Manager    *_keyset_manager,
                         Mailer::Manager    *_mailer_manager,
                         Document::Manager  *_doc_manager) {

    TRACE("[CALL] Register::Request::Manager::create()");
    return new ManagerImpl(
      _domain_manager,
      _contact_manager,
      _nsset_manager,
      _keyset_manager,
      _mailer_manager,
      _doc_manager
    );
}

}
}
