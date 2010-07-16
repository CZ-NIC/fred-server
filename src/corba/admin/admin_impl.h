#ifndef _ADMIN_IMPL_H_
#define _ADMIN_IMPL_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include "session_impl.h"
#include "corba/mailer_manager.h"
#include "register/register.h"
#include "register/invoice.h"
#include "old_utils/dbsql.h"
#include "model/model_filters.h"
#include "bankinginvoicing_impl.h"

#include "conf/manager.h"

class NameService;

class ccReg_Admin_i : public POA_ccReg::Admin,
  public PortableServer::RefCountServantBase {
private:
  std::string m_connection_string;
  NameService *ns;
  Config::Conf& cfg;
  ccReg_BankingInvoicing_i bankingInvoicing;
  DB db;
  std::auto_ptr<Register::Manager> register_manager_;

  typedef std::map<std::string, ccReg_Session_i*> SessionListType;
  SessionListType m_session_list;
  boost::mutex m_session_list_mutex;
  boost::condition cond_;
  bool session_garbage_active_;
  boost::thread *session_garbage_thread_;

  // TEMP: temporary list of possible users -> future in database table
  std::vector<std::string> m_user_list;

  std::string server_name_;
  
  Registry::Registrar::Certification::Manager_ptr reg_cert_mgr_ref_;
  Registry::Registrar::Group::Manager_ptr reg_grp_mgr_ref_;

  void fillRegistrar(ccReg::Registrar& creg,
                     Register::Registrar::Registrar *reg);
  void garbageSession();

public:
  struct DB_CONNECT_FAILED {
  };
  // TEMP: bool _session_garbage - until splitting Whois and Admin interface 
  ccReg_Admin_i(const std::string database, NameService *ns, Config::Conf& _cfg, bool _session_garbage = true)
      throw (DB_CONNECT_FAILED);
  virtual ~ccReg_Admin_i();
  virtual void authenticateUser(const char* _username, const char* _password)
      throw (ccReg::Admin::AuthFailed);
  
  // session management
  virtual char* createSession(const char* username);
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
  //void putRegistrar(const ccReg::Registrar& regData);

  // contact detail
  void fillContact(ccReg::ContactDetail* cv, Register::Contact::Contact* c);
  ccReg::ContactDetail* getContactByHandle(const char* handle)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::ContactDetail* getContactById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);

  // nsset
  void fillNSSet(ccReg::NSSetDetail* cn, Register::NSSet::NSSet* n);
  ccReg::NSSetDetail* getNSSetByHandle(const char* handle)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::NSSetDetail* getNSSetById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);

  // keyset
  void fillKeySet(ccReg::KeySetDetail* cn, Register::KeySet::KeySet* n);
  ccReg::KeySetDetail *getKeySetByHandle(const char* handle)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::KeySetDetail *getKeySetById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  // ccReg::KeySetDetail *getKeySetByDomainFQDN(const char *fqdn)
      // throw (ccReg::Admin::ObjectNotFound);
  ccReg::KeySetDetails *getKeySetsByContactId(ccReg::TID id, CORBA::Long limit)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::KeySetDetails *getKeySetsByContactHandle(const char *handle, CORBA::Long limit)
      throw (ccReg::Admin::ObjectNotFound);

  // domain
  void fillDomain(ccReg::DomainDetail* cd, Register::Domain::Domain* d);
  ccReg::DomainDetail* getDomainByFQDN(const char* fqdn)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::DomainDetail* getDomainById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::DomainDetails *getDomainsByKeySetId(ccReg::TID id, CORBA::Long limit)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::DomainDetails *getDomainsByKeySetHandle(const char *handle, CORBA::Long limit)
      throw (ccReg::Admin::ObjectNotFound);

  ccReg::DomainDetails* getDomainsByInverseKey(const char* key,
                                               ccReg::DomainInvKeyType type,
                                               CORBA::Long limit);
  ccReg::NSSetDetails* getNSSetsByInverseKey(const char* key,
                                             ccReg::NSSetInvKeyType type,
                                             CORBA::Long limit);
  ccReg::KeySetDetails *getKeySetsByInverseKey(
          const char *key,
          ccReg::KeySetInvKeyType type,
          CORBA::Long limit);

  void fillEPPAction(ccReg::EPPAction* cea,
                     const Register::Registrar::EPPAction *rea);
  ccReg::EPPAction* getEPPActionById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::EPPAction* getEPPActionBySvTRID(const char* svTRID)
      throw (ccReg::Admin::ObjectNotFound);
  ccReg::Mailing::Detail* getEmailById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
  // statistics
  CORBA::Long getDomainCount(const char *zone);
  CORBA::Long getSignedDomainCount(const char *_fqdn);
  CORBA::Long getEnumNumberCount();
  // counters
  ccReg::EPPActionTypeSeq* getEPPActionTypeList();
  ccReg::CountryDescSeq* getCountryDescList();
  char* getDefaultCountry();
  ccReg::ObjectStatusDescSeq* getDomainStatusDescList(const char *lang);
  ccReg::ObjectStatusDescSeq* getContactStatusDescList(const char *lang);
  ccReg::ObjectStatusDescSeq* getNSSetStatusDescList(const char *lang);
  ccReg::ObjectStatusDescSeq* getKeySetStatusDescList(const char *lang);
  ccReg::ObjectStatusDescSeq* getObjectStatusDescList(const char *lang);

  /// testovaci fce na typ objektu
  void checkHandle(const char* handle, ccReg::CheckHandleTypeSeq_out ch);

  /* disabled in FRED 2.3
  void fillInvoice(ccReg::Invoicing::Invoice *ci,
                   Register::Invoicing::Invoice *i);
  
   
  ccReg::Invoicing::Invoice* getInvoiceById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
   **/
  
  char* getCreditByZone(const char*registrarHandle, ccReg::TID zone);
  void generateLetters();
  bool setInZoneStatus(ccReg::TID domainId);

  ccReg::TID createPublicRequest(ccReg::PublicRequest::Type _type,
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

  ::CORBA::ULongLong countEnumDomainsByRegistrant(const char* name, ::CORBA::Boolean by_person, ::CORBA::Boolean by_org);
  ccReg::EnumDictList* getEnumDomainsByRegistrant(const char* name, ::CORBA::Boolean by_person, ::CORBA::Boolean by_org, ::CORBA::Long offset, ::CORBA::Long limit);
  ccReg::EnumDictList* getEnumDomainsRecentEntries(::CORBA::Long count);

  Registry::Registrar::Certification::Manager_ptr getCertificationManager();
  Registry::Registrar::Group::Manager_ptr getGroupManager();

private:
  std::string _createQueryForEnumDomainsByRegistrant(const std::string &select_part, const std::string &name, bool by_person, bool by_org);

};

class Registry_Registrar_Certification_Manager_i: public POA_Registry::Registrar::Certification::Manager {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  virtual ~Registry_Registrar_Certification_Manager_i();
public:
  // standard constructor
  Registry_Registrar_Certification_Manager_i();

  // methods corresponding to defined IDL attributes and operations
  ccReg::TID createCertification(ccReg::TID reg_id, const ccReg::DateType& from, const ccReg::DateType& to, ::CORBA::Short score, ccReg::TID evaluation_file_id);
  void shortenCertification(ccReg::TID cert_id, const ccReg::DateType& to);
  void updateCertification(ccReg::TID cert_id, ::CORBA::Short score, ccReg::TID evaluation_file_id);
  Registry::Registrar::Certification::CertificationList* getCertificationsByRegistrar(ccReg::TID registrar_id);
};

class Registry_Registrar_Group_Manager_i: public POA_Registry::Registrar::Group::Manager {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~Registry_Registrar_Group_Manager_i();
public:
  // standard constructor
  Registry_Registrar_Group_Manager_i();
  virtual ~Registry_Registrar_Group_Manager_i();

  // methods corresponding to defined IDL attributes and operations
  ccReg::TID createGroup(const char* name);
  void deleteGroup(ccReg::TID group_id);
  void updateGroup(ccReg::TID group_id, const char* name);
  ccReg::TID addRegistrarToGroup(ccReg::TID reg_id, ccReg::TID group_id);
  void removeRegistrarFromGroup(ccReg::TID reg_id, ccReg::TID group_id);
  Registry::Registrar::Group::GroupList* getGroups();
  Registry::Registrar::Group::MembershipByRegistrarList* getMembershipsByRegistar(ccReg::TID registrar_id);
  Registry::Registrar::Group::MembershipByGroupList* getMembershipsByGroup(ccReg::TID group_id);

};

#endif /*ADMIN_IMPL_H*/
