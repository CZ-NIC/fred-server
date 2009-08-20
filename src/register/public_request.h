#ifndef PUBLIC_REQUEST_H_
#define PUBLIC_REQUEST_H_

#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "common_object.h"
#include "registrar.h"
#include "object.h"
#include "domain.h"
#include "contact.h"
#include "nsset.h"
#include "keyset.h"
#include "mailer.h"
#include "documents.h"

#include "db_settings.h"
#include "model/model_filters.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Register {
namespace PublicRequest {

/// Member identification (i.e. for sorting) 
enum MemberType {
  MT_CRDATE, ///< create date
  MT_RDATE,  ///< resolve date
  MT_TYPE,   ///< type of request
  MT_STATUS  ///< request status
};

/// Types of request
enum Type {
  PRT_AUTHINFO_AUTO_RIF,            ///< Request for authinfo was created by registrar through EPP
  PRT_AUTHINFO_AUTO_PIF,            ///< Request for authinfo automatic answer created through PIF
  PRT_AUTHINFO_EMAIL_PIF,           ///< Request for authinfo waiting for autorization by signed email
  PRT_AUTHINFO_POST_PIF,            ///< Request for authinfo waiting for autorization by checked letter
  PRT_BLOCK_CHANGES_EMAIL_PIF,      ///< Request for block update object waiting for autorization by signed email
  PRT_BLOCK_CHANGES_POST_PIF,       ///< Request for block update object waiting for autorization by checked letter
  PRT_BLOCK_TRANSFER_EMAIL_PIF,     ///< Request for block transfer object waiting for autorization by signed email
  PRT_BLOCK_TRANSFER_POST_PIF,      ///< Request for block transfer object waiting for autorization by checked letter
  PRT_UNBLOCK_CHANGES_EMAIL_PIF,    ///< Request for unblock update object waiting for autorization by signed email
  PRT_UNBLOCK_CHANGES_POST_PIF,     ///< Request for unblock update object waiting for autorization by checked letter
  PRT_UNBLOCK_TRANSFER_EMAIL_PIF,   ///< Request for unblock transfer object waiting for autorization by signed email
  PRT_UNBLOCK_TRANSFER_POST_PIF     ///< Request for unblock transfer object waiting for autorization by checked letter
};

std::string Type2Str(Type _type); 

/// Request status
enum Status {
  PRS_NEW,       ///< Request was created and waiting for autorization 
  PRS_ANSWERED,  ///< Email with answer was sent
  PRS_INVALID    ///< Time passed without authorization   
};

std::string Status2Str(Status _status); 
/// Object types
enum ObjectType {
  OT_UNKNOWN = 0,
  OT_CONTACT = 1,
  OT_NSSET   = 2,
  OT_DOMAIN  = 3,
  OT_KEYSET  = 4
};
std::string ObjectType2Str(ObjectType type);


/*
 * Object info
 */
struct OID {
  OID(Database::ID _id) : id(_id) { }
  OID(Database::ID _id, std::string _handle, ObjectType _type) : id(_id),
                                                              handle(_handle),
                                                              type(_type) { }
  Database::ID   id;
  std::string handle;
  ObjectType  type;
};

struct REQUEST_BLOCKED : std::exception {};

/*
 * Request interface
 */
class PublicRequest : virtual public Register::CommonObject {
public:
  virtual ~PublicRequest() {
  }
  
  virtual void init(Database::Row::Iterator& _it) = 0; 
  virtual void save() = 0;
  
  virtual Register::PublicRequest::Type getType() const = 0;
  virtual void setType(Register::PublicRequest::Type _type) = 0;
  virtual Register::PublicRequest::Status getStatus() const = 0;
  virtual void setStatus(Register::PublicRequest::Status _status) = 0;
  virtual ptime getCreateTime() const = 0;
  virtual ptime getResolveTime() const = 0;
  virtual const std::string& getReason() const = 0;
  virtual void setReason(const std::string& _reason) = 0;
  virtual const std::string& getEmailToAnswer() const = 0;
  virtual void setEmailToAnswer(const std::string& _email) = 0;
  virtual const Database::ID getAnswerEmailId() const = 0;
  virtual const Database::ID getEppActionId() const = 0;
  virtual void setEppActionId(const Database::ID& _epp_action_id) = 0;
  virtual void setRegistrarId(const Database::ID& _registrar_id) = 0;
  
  virtual void addObject(const OID& _oid) = 0;
  virtual const OID& getObject(unsigned _idx) const = 0;
  virtual unsigned getObjectSize() const = 0;
  
  virtual const std::string getSvTRID() const = 0;

  virtual const Database::ID getRegistrarId() const = 0;
  virtual const std::string getRegistrarHandle() const = 0;
  virtual const std::string getRegistrarName() const = 0;
  virtual const std::string getRegistrarUrl() const = 0;

  /// check if request can be handles
  virtual bool check() const = 0;
  /// return name of mailtemplate for answer email
	virtual std::string getTemplateName() const = 0;
	/// fill all mailtemplate parameters
	virtual void fillTemplateParams(Mailer::Parameters& params) const = 0;
	/// return list of destination email addresses for answer email
  virtual std::string getEmails() const = 0;
  /// send email with answer 
	virtual TID sendEmail() const throw (Mailer::NOT_SEND) = 0;
	/// process request (or just close in case of invalid flag)
	virtual void process(bool invalid, bool check) throw (REQUEST_BLOCKED, Mailer::NOT_SEND, Database::Exception)= 0;
	/// concrete action taken during request processing
	virtual void processAction(bool check) throw (REQUEST_BLOCKED, Database::Exception) = 0;
	/// return proper type for PDF template generation
	virtual unsigned getPDFType() const = 0;
};

class List : virtual public Register::CommonList {
public:
  virtual ~List() {
  }
  
  virtual const char* getTempTableName() const = 0;
  virtual PublicRequest* get(unsigned _idx) const = 0;
  virtual void reload(Database::Filters::Union& _filter) = 0;
  
  /// from CommonList; propably will be removed in future
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual void reload() = 0;
  virtual void sort(MemberType _member, bool _asc) = 0;
};


class Manager {
public:
  virtual ~Manager() {
  }
  
  static Manager* create(Domain::Manager    *_domain_manager,
                         Contact::Manager   *_contact_manager,
                         NSSet::Manager     *_nsset_manager,
                         KeySet::Manager    *_keyset_manager,
                         Mailer::Manager    *_mailer_manager,
                         Document::Manager  *_doc_manager);
  
  virtual Mailer::Manager* getMailerManager() const = 0;
  virtual List* createList() const = 0;
  virtual void getPdf(Database::ID _id, 
                      const std::string& _lang, 
                      std::ostream& _output) const 
    throw (NOT_FOUND, SQL_ERROR, Document::Generator::ERROR) = 0;
  
  virtual PublicRequest* createRequest(Type _type) const
    throw (NOT_FOUND, SQL_ERROR, Mailer::NOT_SEND, REQUEST_BLOCKED) = 0;

  virtual void processRequest(Database::ID _id, 
                              bool _invalidate, bool _check) const 
    throw (NOT_FOUND, SQL_ERROR, Mailer::NOT_SEND, REQUEST_BLOCKED) = 0;

}; 
}
}

#endif /*PUBLIC_REQUEST_H_*/
