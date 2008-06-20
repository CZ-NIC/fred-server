#ifndef _ADMIN_IMPL_H_
#define _ADMIN_IMPL_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "session_impl.h"
#include "corba/mailer_manager.h"
#include "register/register.h"
#include "register/invoice.h"
#include "old_utils/dbsql.h"
#include "old_utils/conf.h"
#include "db/dbs.h"
#include "model/model_filters.h"

class NameService;

class ccReg_Admin_i : public POA_ccReg::Admin,
  public PortableServer::RefCountServantBase {
private:
  std::string m_connection_string;
  NameService *ns;
  Conf& cfg;
  DB db;
  std::auto_ptr<Register::Manager> register_manager_;
  std::auto_ptr<DBase::Manager> m_db_manager;

  typedef std::map<std::string, ccReg_Session_i*> SessionListType;
  SessionListType m_session_list;
  boost::mutex m_session_list_mutex;
  bool session_garbage_active_;
  boost::thread *session_garbage_thread_;

  // TEMP: temporary list of possible users -> future in database table
  std::vector<std::string> m_user_list;

  
  void fillRegistrar(ccReg::Registrar& creg,
                     Register::Registrar::Registrar *reg);
  void garbageSession();

public:
  struct DB_CONNECT_FAILED {
  };
  // TEMP: bool _session_garbage - until splitting Whois and Admin interface 
  ccReg_Admin_i(const std::string database, NameService *ns, Conf& _cfg, bool _session_garbage = true)
      throw (DB_CONNECT_FAILED);
  virtual ~ccReg_Admin_i();
  virtual void authenticateUser(const char* _username, const char* _password)
      throw (ccReg::Admin::AuthFailed);
  
  // session management
  virtual char* createSession(const char* username)
      throw (ccReg::Admin::AuthFailed);
  virtual void destroySession(const char* _session_id);
  virtual ccReg::Session_ptr getSession(const char* sessionID)
      throw (ccReg::Admin::ObjectNotFound);
  
  // registrar management
  ccReg::RegistrarList* getRegistrars() throw (ccReg::Admin::SQL_ERROR);
  ccReg::RegistrarList* getRegistrarsByZone(const char *zone)
      throw (ccReg::Admin::SQL_ERROR);
  ccReg::Registrar* getRegistrarById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound, ccReg::Admin::SQL_ERROR);
  ccReg::Registrar* getRegistrarByHandle(const char* handle)
      throw (ccReg::Admin::ObjectNotFound, ccReg::Admin::SQL_ERROR);
  void putRegistrar(const ccReg::Registrar& regData);
  
  // object detail
  void fillContact(ccReg::ContactDetail* cv, Register::Contact::Contact* c);
  ccReg::ContactDetail* getContactByHandle(const char* handle)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::ContactDetail* getContactById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  void fillNSSet(ccReg::NSSetDetail* cn, Register::NSSet::NSSet* n);
  ccReg::NSSetDetail* getNSSetByHandle(const char* handle)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::NSSetDetail* getNSSetById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  void fillDomain(ccReg::DomainDetail* cd, Register::Domain::Domain* d);
  ccReg::DomainDetail* getDomainByFQDN(const char* fqdn)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::DomainDetail* getDomainById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::DomainDetails* getDomainsByInverseKey(const char* key,
                                               ccReg::DomainInvKeyType type,
                                               CORBA::Long limit);
  ccReg::NSSetDetails* getNSSetsByInverseKey(const char* key,
                                             ccReg::NSSetInvKeyType type,
                                             CORBA::Long limit);
  void fillEPPAction(ccReg::EPPAction* cea,
                     const Register::Registrar::EPPAction *rea);
  ccReg::EPPAction* getEPPActionById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::EPPAction* getEPPActionBySvTRID(const char* svTRID)
      throw (ccReg::Admin::ObjectNotFound);
  void fillAuthInfoRequest(ccReg::AuthInfoRequest::Detail *carid,
                           Register::AuthInfoRequest::Detail *rarid);
  ccReg::AuthInfoRequest::Detail* getAuthInfoRequestById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::Mailing::Detail* getEmailById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  // statistics
  CORBA::Long getDomainCount(const char *zone);
  CORBA::Long getEnumNumberCount();
  // counters
  ccReg::EPPActionTypeSeq* getEPPActionTypeList();
  ccReg::CountryDescSeq* getCountryDescList();
  char* getDefaultCountry();
  ccReg::ObjectStatusDescSeq* getDomainStatusDescList(const char *lang);
  ccReg::ObjectStatusDescSeq* getContactStatusDescList(const char *lang);
  ccReg::ObjectStatusDescSeq* getNSSetStatusDescList(const char *lang);
  /// testovaci fce na typ objektu
  void checkHandle(const char* handle, ccReg::CheckHandleTypeSeq_out ch);

  void fillInvoice(ccReg::Invoicing::Invoice *ci,
                   Register::Invoicing::Invoice *i);
  ccReg::Invoicing::Invoice* getInvoiceById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  char* getCreditByZone(const char*registrarHandle, ccReg::TID zone);
  void generateLetters();

  ccReg::TID createPublicRequest(ccReg::PublicRequest::Type _type,
                                 ccReg::TID _epp_action_id,
                                 const char *_reason,
                                 const char *_email_to_answer,
                                 const ccReg::Admin::ObjectIdList& _object_ids)
    throw (ccReg::Admin::BAD_EMAIL, ccReg::Admin::OBJECT_NOT_FOUND,
    ccReg::Admin::ACTION_NOT_FOUND, ccReg::Admin::SQL_ERROR,
    ccReg::Admin::INVALID_INPUT, ccReg::Admin::REQUEST_BLOCKED);
  
  void processPublicRequest(ccReg::TID id, CORBA::Boolean invalid) throw (
    ccReg::Admin::SQL_ERROR, ccReg::Admin::OBJECT_NOT_FOUND,
    ccReg::Admin::MAILER_ERROR, ccReg::Admin::REQUEST_BLOCKED
  );
  ccReg::Admin::Buffer* getPublicRequestPDF(ccReg::TID id, const char *lang);

};

#endif /*ADMIN_IMPL_H*/
