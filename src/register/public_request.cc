#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "public_request.h"
#include "log/logger.h"
#include "util.h"

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


static bool checkState(
  DBase::ID objectId, unsigned state, DBase::Connection *c
) {
  DBase::Query sql;
  sql.buffer() << "SELECT COUNT(*) FROM object_state WHERE state_id=" 
             << state << " AND object_id=" << objectId 
             << " AND valid_to ISNULL";
  std::auto_ptr<DBase::Result> r_objects(c->exec(sql));
  std::auto_ptr<DBase::ResultIterator> oit(r_objects->getIterator());
  int res = oit->isDone() ? 0 : (int)oit->getNextValue();
  return res > 0;
}

// static DBase::ID insertNewStateRequest(
static void insertNewStateRequest(
  DBase::ID blockRequestID, DBase::ID objectId, 
  unsigned state, DBase::Connection *c
) {
  DBase::InsertQuery osr("object_state_request");
  osr.add("object_id",objectId);
  osr.add("state_id",state);  
  c->exec(osr);
  DBase::Query prsrm;
  prsrm.buffer() << "INSERT INTO public_request_state_request_map ("
                 << " state_request_id, block_request_id "
                 << ") VALUES ( " 
                 << "CURRVAL('object_state_request_id_seq')," 
                 << blockRequestID << ")";
  std::auto_ptr<DBase::Result> prsrmResult(c->exec(prsrm));               
}

/** check if already blocked request interfere with requested states
 this is true in every situation other then when actual states are
 smaller subset of requested states. if requested by setting parameter 
 blockRequestID, all states in subset are closed */
static bool queryBlockRequest(
  DBase::ID objectId, DBase::ID blockRequestID, 
  const std::string& states, bool unblock,
  DBase::Connection *c
) {
  // number of states in csv list of states
  unsigned numStates = count(states.begin(),states.end(),',') + 1; 
  
  DBase::Query sql;
  sql.buffer() << "SELECT " // ?? FOR UPDATE ??
               << "  osr.state_id IN (" << states << ") AS st, "
               << "  state_request_id "
               << "FROM public_request_state_request_map ps "
               << "JOIN public_request pr ON (pr.id=ps.block_request_id) "
               << "JOIN object_state_request osr "
               << "  ON (osr.id=ps.state_request_id AND osr.canceled ISNULL) " 
               << "WHERE osr.object_id=" << objectId << " "                 
               << "ORDER BY st ASC ";
  std::auto_ptr<DBase::Result> result(c->exec(sql));
  std::auto_ptr<DBase::ResultIterator> it(result->getIterator());
  // in case of unblocking, it's error when no block request states are found
  if (!result->getNumRows() && unblock) return false;
  for (it->first(); !it->isDone(); it->next()) {
    if (!(bool)it->getNextValue()) return false;
    if ((result->getNumRows() == numStates && !unblock) ||
    	(result->getNumRows() != numStates && unblock)) return false;
    if (!blockRequestID) return true;
    DBase::ID stateRequestID = it->getNextValue();
    // close
    DBase::Query sql1;
    sql1.buffer() << "UPDATE public_request_state_request_map "
                  << "SET block_request_id=" << blockRequestID << " "  
                  << "WHERE state_request_id=" << stateRequestID; 
    std::auto_ptr<DBase::Result> result1(c->exec(sql1));
    DBase::Query sql2;
    sql2.buffer() << "UPDATE object_state_request "
                  << "SET canceled=CURRENT_TIMESTAMP "
                  << "WHERE id= " << stateRequestID;
    std::auto_ptr<DBase::Result> result2(c->exec(sql2));  
  }
  return true;
}
 
class PublicRequestImpl : public Register::CommonObjectImpl,
                    virtual public PublicRequest {
private:
  Register::PublicRequest::Type type_;
  DBase::ID epp_action_id_;
  DBase::DateTime create_time_;
  Register::PublicRequest::Status status_;
  DBase::DateTime resolve_time_;
  std::string reason_;
  std::string email_to_answer_;
  DBase::ID answer_email_id_;
  
  std::string svtrid_;
  DBase::ID registrar_id_;
  std::string registrar_handle_;
  std::string registrar_name_;
  std::string registrar_url_;

  std::vector<OID> objects_;
  
protected:
  Manager* man_;
  DBase::Connection* conn_;
  
public:
  PublicRequestImpl() : CommonObjectImpl(0), status_(PRS_NEW) {
  }
  
  PublicRequestImpl(DBase::ID _id,
              Register::PublicRequest::Type _type,
              DBase::ID _epp_action_id,
              DBase::DateTime _create_time,
              Register::PublicRequest::Status _status,
              DBase::DateTime _resolve_time,
              std::string _reason,
              std::string _email_to_answer,
              DBase::ID _answer_email_id,
              std::string _svtrid,
              DBase::ID _registrar_id,
              std::string _registrar_handle,
              std::string _registrar_name,
              std::string _registrar_url
              ) :
              CommonObjectImpl(_id), type_(_type), epp_action_id_(_epp_action_id), 
              create_time_(_create_time), status_(_status),
              resolve_time_(_resolve_time), reason_(_reason), 
              email_to_answer_(_email_to_answer), answer_email_id_(_answer_email_id), 
              svtrid_(_svtrid), registrar_id_(_registrar_id), 
              registrar_handle_(_registrar_handle), 
              registrar_name_(_registrar_name), registrar_url_(_registrar_url) {
  }
  
  void setManager(Manager* _man, DBase::Connection* _conn) {
    man_ = _man;
    conn_ = _conn;
  }
  
  void init(DBase::ResultIterator *_it) {
    id_ = (unsigned long long)_it->getNextValue();
    epp_action_id_ = _it->getNextValue();
    create_time_ = _it->getNextValue();
    status_ = (Register::PublicRequest::Status)(int)_it->getNextValue();
    resolve_time_ = _it->getNextValue();
    reason_ = (std::string)_it->getNextValue();
    email_to_answer_ = (std::string)_it->getNextValue();
    answer_email_id_ = _it->getNextValue();
    svtrid_ = (std::string)_it->getNextValue();
    registrar_id_ = _it->getNextValue();
    registrar_handle_ = (std::string)_it->getNextValue();
    registrar_name_ = (std::string)_it->getNextValue();
    registrar_url_ = (std::string)_it->getNextValue();
  }
  
  virtual void save(DBase::Connection *_conn) {
    TRACE("[CALL] Register::Request::RequestImpl::save()");
    if (objects_.empty()) {
      LOGGER("register").error("can't create or update request with no object specified!");
      throw;
    }

    if (id_) {
      DBase::Query update_request;
      update_request.buffer() << "UPDATE public_request SET "
                              << "status = " << status_ << ", "
                              << "resolve_time = now(), "
                              << "answer_email_id = " << DBase::Value(answer_email_id_) << " "
                              << "WHERE id = " << id_;
      try {
        std::auto_ptr<DBase::Result> result(_conn->exec(update_request));
  
        LOGGER("db").info(boost::format("request id='%1%' updated successfully -- %2%") % 
                          id_ % (status_ == PRS_INVALID ? "invalidated" : "answered"));
      }
      catch (DBase::Exception& ex) {
        LOGGER("db").error(boost::format("%1%") % ex.what());
      }
      catch (std::exception& ex) {
        LOGGER("db").error(boost::format("%1%") % ex.what());
      }
      
    }
    else {    
      
      DBase::InsertQuery insert_request("public_request");
      
      insert_request.add("request_type", type_);
      insert_request.add("epp_action_id", epp_action_id_);
      insert_request.add("status", status_);
      insert_request.add("reason", reason_);
      insert_request.add("email_to_answer", email_to_answer_);
        
      try {
        DBase::Transaction new_request_transaction = _conn->getTransaction();
        
        _conn->exec(insert_request);
        std::string objects_str = "{";
        std::vector<OID>::iterator it = objects_.begin();
        for (; it != objects_.end(); ++it) {
          DBase::Query insert_object;
          insert_object.buffer() << "INSERT INTO public_request_objects_map "
                                 << "(request_id, object_id) VALUES ("
                                 << "currval('public_request_id_seq')" << ", "
                                 << it->id << ")";
          std::auto_ptr<DBase::Result> r_object(_conn->exec(insert_object));
          objects_str += " " + Util::stream_cast<std::string>(it->id);
        }
        objects_str += " }";
        
        DBase::Sequence pp_seq(_conn, "public_request_id_seq");
        id_ = pp_seq.getCurrent();
        
        new_request_transaction.commit();
        LOGGER("db").info(boost::format("request id='%1%' for objects=%2% created successfully")
                  % id_ % objects_str);
        // handle special behavior (i.e. processing request after creation)
        postCreate(); 
      }
      catch (DBase::Exception& ex) {
        LOGGER("db").error(boost::format("%1%") % ex.what());
      }
      catch (std::exception& ex) {
        LOGGER("db").error(boost::format("%1%") % ex.what());
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
  
  virtual const DBase::ID getAnswerEmailId() const {
    return answer_email_id_;
  }
  
  virtual const DBase::ID getEppActionId() const {
    return epp_action_id_;
  }

  virtual void setEppActionId(const DBase::ID& _epp_action_id) {
    epp_action_id_ = _epp_action_id;
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
    DBase::Filters::Union uf;
    std::stringstream emails;
    DBase::SelectQuery sql;
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
	case OT_UNKNOWN:
	  break;
      };
      std::auto_ptr<DBase::Result> r_emails(conn_->exec(sql));
      std::auto_ptr<DBase::ResultIterator> it(r_emails->getIterator());
      for (it->first(); !it->isDone(); it->next()) {
        emails << it->getNextValue() << " ";
      }
    }
    return emails.str();
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
  virtual void processAction(bool check) throw (REQUEST_BLOCKED) {
    // default is to do nothing special
  }
  /// process request (or just close in case of invalid flag)
  virtual void process(bool invalid, bool check) throw (REQUEST_BLOCKED) {
    DBase::Transaction process_request_transaction = conn_->getTransaction(); 
    if (invalid) status_ = PRS_INVALID;
    else {
    	processAction(check);
      status_ = PRS_ANSWERED;
      answer_email_id_ = sendEmail();
    }
    resolve_time_ = ptime(boost::posix_time::second_clock::local_time());
    save(conn_);
    process_request_transaction.commit();
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
     res = !checkState(getObject(i).id,SERVER_TRANSFER_PROHIBITED,conn_);
    return res;
  }
  virtual void processAction(bool _check) throw (REQUEST_BLOCKED) {
    if (_check && !check()) throw REQUEST_BLOCKED();
  }
  std::string getAuthInfo() const throw (SQL_ERROR) {
    // just one object is supported
    if (!getObjectSize() || getObjectSize() > 1) return "";
    DBase::SelectQuery sql;
    sql.buffer() << "SELECT o.AuthInfoPw "
                 << "FROM object o "
                 << "WHERE o.id=" << getObject(0).id;
    std::auto_ptr<DBase::Result> res(conn_->exec(sql));
    std::auto_ptr<DBase::ResultIterator> it(res->getIterator());
    if (it->isDone()) return "";
    return it->getNextValue();
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
     res = !checkState(getObject(i).id,SERVER_UPDATE_PROHIBITED,conn_) &&
      queryBlockRequest(getObject(i).id,0,"3",false,conn_);
    return res;    
  }
  virtual short blockType() const {
  	return 1;
  }
  virtual short blockAction() const {
  	return 2;
  }
	virtual void processAction(bool check) throw (REQUEST_BLOCKED){
    for (unsigned i=0; i<getObjectSize(); i++) {
    	if (!checkState(getObject(i).id,SERVER_UPDATE_PROHIBITED,conn_) &&
    	      queryBlockRequest(getObject(i).id,getId(),"3",false,conn_)) {
    		insertNewStateRequest(getId(),getObject(i).id,3,conn_);
    	} else throw REQUEST_BLOCKED();
      DBase::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      std::auto_ptr<DBase::Result> res(conn_->exec(q));
    }
	}
};

class BlockUpdateRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = !checkState(getObject(i).id,SERVER_UPDATE_PROHIBITED,conn_) &&
      queryBlockRequest(getObject(i).id,0,"3,4",false,conn_);
    return res;    
  }
  virtual short blockType() const {
  	return 1;
  }
  virtual short blockAction() const {
  	return 1;
  }
	virtual void processAction(bool check) throw (REQUEST_BLOCKED){
    for (unsigned i=0; i<getObjectSize(); i++) {
    	if (!checkState(getObject(i).id,SERVER_UPDATE_PROHIBITED,conn_) &&
    	      queryBlockRequest(getObject(i).id,getId(),"3,4",false,conn_)) {
    		insertNewStateRequest(getId(),getObject(i).id,SERVER_TRANSFER_PROHIBITED,conn_);
    		insertNewStateRequest(getId(),getObject(i).id,SERVER_UPDATE_PROHIBITED,conn_);
    	} else throw REQUEST_BLOCKED();
      DBase::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      std::auto_ptr<DBase::Result> res(conn_->exec(q));
    }
	}
};

class UnBlockTransferRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id,0,"3",true,conn_);
    return res;    
  }
  virtual short blockType() const {
  	return 2;
  }
  virtual short blockAction() const {
  	return 2;
  }
	virtual void processAction(bool check) throw (REQUEST_BLOCKED){
    for (unsigned i=0; i<getObjectSize(); i++) {
    	if (!queryBlockRequest(getObject(i).id,getId(),"3",true,conn_))
        throw REQUEST_BLOCKED();
      DBase::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      std::auto_ptr<DBase::Result> res(conn_->exec(q));
    }
	}
};

class UnBlockUpdateRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id,0,"3,4",true,conn_);
    return res;    
  }
  virtual short blockType() const {
  	return 2;
  }
  virtual short blockAction() const {
  	return 1;
  }
	virtual void processAction(bool check) throw (REQUEST_BLOCKED){
    for (unsigned i=0; i<getObjectSize(); i++) {
    	if (!queryBlockRequest(getObject(i).id,getId(),"3,4",true,conn_))
        throw REQUEST_BLOCKED();
      DBase::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";      
      std::auto_ptr<DBase::Result> res(conn_->exec(q));
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


class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;
  
public:
  ListImpl(DBase::Connection *_conn, Manager *_manager) : CommonListImpl(_conn),
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
  
  virtual void reload(DBase::Filters::Union& _filter) {
    TRACE("[CALL] Register::Request::ListImpl::reload()");
    clear();
    _filter.clearQueries();
    
    DBase::SelectQuery id_query;
    DBase::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      DBase::Filters::PublicRequest *rf = dynamic_cast<DBase::Filters::PublicRequest* >(*fit);
      if (!rf)
        continue;
      DBase::SelectQuery *tmp = new DBase::SelectQuery();
      tmp->addSelect(new DBase::Column("id", rf->joinRequestTable(), "DISTINCT"));
      _filter.addQuery(tmp);
    }
    id_query.limit(load_limit_);
    _filter.serialize(id_query);
    
    DBase::InsertQuery tmp_table_query = DBase::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER("db").debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    DBase::SelectQuery object_info_query;
    object_info_query.select() << "t_1.request_type, t_1.id, t_1.epp_action_id, "
                               << "t_1.create_time, t_1.status, t_1.resolve_time, "
                               << "t_1.reason, t_1.email_to_answer, t_1.answer_email_id, "
                               << "t_2.servertrid, t_4.id, t_4.handle, t_4.name, t_4.url";
    object_info_query.from() << getTempTableName() << " tmp " 
                             << "JOIN public_request t_1 ON (t_1.id = tmp.id) "
                             << "LEFT JOIN action t_2 ON (t_1.epp_action_id = t_2.id) "
                             << "LEFT JOIN login t_3 ON (t_2.clientid = t_3.id) "
                             << "LEFT JOIN registrar t_4 ON (t_3.registrarid = t_4.id) ";
    object_info_query.order_by() << "t_1.id";
    
    try {
      fillTempTable(tmp_table_query);
      
      std::auto_ptr<DBase::Result> r_info(conn_->exec(object_info_query));
      std::auto_ptr<DBase::ResultIterator> it(r_info->getIterator());
      for (it->first(); !it->isDone(); it->next()) {
        Register::PublicRequest::Type type = (Register::PublicRequest::Type)(int)it->getNextValue();
        PublicRequest* request = manager_->createRequest(type,conn_);
        request->init(it.get());
        data_.push_back(request);
      }
      
      if (data_.empty())
        return;

      /*
       * load objects details for requests
       */      
      resetIDSequence();
      
      DBase::SelectQuery objects_query;
      objects_query.select() << "tmp.id, t_1.object_id, t_2.name, t_2.type";
      objects_query.from() << getTempTableName() << " tmp "
                           << "JOIN public_request_objects_map t_1 ON (tmp.id = t_1.request_id) "
                           << "JOIN object_registry t_2 ON (t_1.object_id = t_2.id)";
      objects_query.order_by() << "tmp.id";

      std::auto_ptr<DBase::Result> r_objects(conn_->exec(objects_query));
      std::auto_ptr<DBase::ResultIterator> oit(r_objects->getIterator());
      for (oit->first(); !oit->isDone(); oit->next()) {
        DBase::ID request_id = oit->getNextValue();
        DBase::ID object_id = oit->getNextValue();
        std::string object_handle = oit->getNextValue(); 
        ObjectType object_type = (ObjectType)(int)oit->getNextValue();
             
        PublicRequestImpl *request_ptr = dynamic_cast<PublicRequestImpl *>(findIDSequence(request_id));
        if (request_ptr)
          request_ptr->addObject(OID(object_id, object_handle, object_type));
      }

      
    }
    catch (DBase::Exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
      clear();
    }
    catch (std::exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
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
  DBase::Manager *db_manager_;
  Domain::Manager *domain_manager_;
  Contact::Manager *contact_manager_;
  NSSet::Manager *nsset_manager_;
  Mailer::Manager *mailer_manager_;
  Document::Manager *doc_manager_;
  
  DBase::Connection *conn_;
    
public:
  ManagerImpl(DBase::Manager *_db_manager,
              Domain::Manager *_domain_manager,
              Contact::Manager *_contact_manager,
              NSSet::Manager *_nsset_manager,
              Mailer::Manager *_mailer_manager,
              Document::Manager *_doc_manager) :
              db_manager_(_db_manager),
              domain_manager_(_domain_manager),
              contact_manager_(_contact_manager),
              nsset_manager_(_nsset_manager),
              mailer_manager_(_mailer_manager),
              doc_manager_(_doc_manager),
              conn_(db_manager_->getConnection()) {
  }

  virtual ~ManagerImpl() {
    boost::checked_delete<DBase::Connection>(conn_);
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
    return new ListImpl(conn_, (Manager *)this);
  }
  
  virtual void getPdf(DBase::ID _id, 
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
    Type _type, DBase::Connection* _conn
  ) const throw (NOT_FOUND, SQL_ERROR, Mailer::NOT_SEND, REQUEST_BLOCKED) {
    TRACE("[CALL] Register::Request::Manager::createRequest()");
    PublicRequestImpl *request;
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
    }
    request->setType(_type);
    request->setManager((Manager *)this,_conn);
    return request;
  }
  
  List *loadRequest(DBase::ID id) const throw (NOT_FOUND) {
    DBase::Filters::PublicRequestImpl prf;
    prf.addId().setValue(id);
    DBase::Filters::Union uf;
    uf.addFilter(&prf);
    List *l = createList();
    l->reload(uf);
    if (l->getCount() != 1) throw NOT_FOUND();
    return l;
  }
  
  virtual void processRequest(DBase::ID _id, bool _invalidate, 
                              bool check) const 
    throw (NOT_FOUND, SQL_ERROR, Mailer::NOT_SEND, REQUEST_BLOCKED) {
    TRACE(boost::format("[CALL] Register::Request::Manager::processRequest(%1%, %2%)") %
          _id % _invalidate);
    try {
      std::auto_ptr<List> l(loadRequest(_id));
      l->get(0)->process(_invalidate,check);
    } 
    catch (DBase::Exception) { throw SQL_ERROR(); }
  }
  
};

Manager* Manager::create(DBase::Manager *_db_manager,
                         Domain::Manager *_domain_manager,
                         Contact::Manager *_contact_manager,
                         NSSet::Manager *_nsset_manager,
                         Mailer::Manager *_mailer_manager,
                         Document::Manager *_doc_manager) {
    
    TRACE("[CALL] Register::Request::Manager::create()");
    return new ManagerImpl(
      _db_manager, _domain_manager, _contact_manager, 
      _nsset_manager, _mailer_manager, _doc_manager
    );
}

}
}
