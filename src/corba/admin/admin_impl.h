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
#include "fredlib/registry.h"
#include "fredlib/invoicing/invoice.h"
#include "old_utils/dbsql.h"
#include "model/model_filters.h"
#include "bankinginvoicing_impl.h"

class NameService;

class ccReg_Admin_i : public POA_ccReg::Admin,
  public PortableServer::RefCountServantBase {
private:
  std::string m_connection_string;
  NameService *ns;


  //conf
  bool restricted_handles_;
  std::string docgen_path_;
  std::string docgen_template_path_;
  unsigned int docgen_domain_count_limit_;
  std::string fileclient_path_;

  unsigned adifd_session_max_;
  unsigned adifd_session_timeout_;
  unsigned adifd_session_garbage_;

  ccReg_BankingInvoicing_i bankingInvoicing;
  DBSharedPtr  db_disconnect_guard_;
  std::auto_ptr<Fred::Manager> registry_manager_;

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

  void fillRegistrar(ccReg::AdminRegistrar& creg,
                     Fred::Registrar::Registrar *reg);
  void garbageSession();

public:
  struct DB_CONNECT_FAILED : public std::runtime_error
  {
      DB_CONNECT_FAILED()
              : std::runtime_error("Database connection failed")
      {}
  };
  // TEMP: bool _session_garbage - until splitting Whois and Admin interface 
  ccReg_Admin_i(const std::string database, NameService *ns
          , bool restricted_handles
          , const std::string& docgen_path
          , const std::string& docgen_template_path
          , unsigned int docgen_domain_count_limit
          , const std::string& fileclient_path
          , unsigned adifd_session_max
          , unsigned adifd_session_timeout
          , unsigned adifd_session_garbage
          , bool _session_garbage = true);
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
  ccReg::AdminRegistrar* getRegistrarById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound, ccReg::Admin::SQL_ERROR);

  // statistics
  CORBA::Long getDomainCount(const char *zone);
  CORBA::Long getSignedDomainCount(const char *_fqdn);
  CORBA::Long getEnumNumberCount();
  // counters
  Registry::CountryDescSeq* getCountryDescList();
  char* getDefaultCountry();
  Registry::ObjectStatusDescSeq* getObjectStatusDescList(const char *lang);

  /// testovaci fce na typ objektu
  void checkHandle(const char* handle, ccReg::CheckHandleTypeSeq_out ch);

  /* disabled in FRED 2.3
  void fillInvoice(ccReg::Invoicing::Invoice *ci,
                   Fred::Invoicing::Invoice *i);
  
   
  ccReg::Invoicing::Invoice* getInvoiceById(ccReg::TID id)
      throw (ccReg::Admin::ObjectNotFound);
   **/
  
  char* getCreditByZone(const char*registrarHandle, ccReg::TID zone);
  void generateLetters();
  bool setInZoneStatus(ccReg::TID domainId);

  ccReg::TID createPublicRequest(Registry::PublicRequest::Type _type,
                                 const char *_reason,
                                 const char *_email_to_answer,
                                 const ccReg::Admin::ObjectIdList& _object_ids,
                                 const ccReg::TID requestId)
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

  ccReg::EnumList* getBankAccounts();

  /* get last counted number of epp request done by registrar
   * for now it returns data of last poll request fee message */
  ccReg::RegistrarRequestCountInfo* getRegistrarRequestCount(const char* _registrar);

  bool isRegistrarBlocked(ccReg::TID reg_id) throw (
          ccReg::Admin::InternalServerError, ccReg::Admin::ObjectNotFound);

  bool blockRegistrar(ccReg::TID reg_id) throw (
          ccReg::Admin::InternalServerError, ccReg::Admin::ObjectNotFound);

  void unblockRegistrar(ccReg::TID reg_id, ccReg::TID request_id) throw (
          ccReg::Admin::InternalServerError, ccReg::Admin::ObjectNotFound, ccReg::Admin::ObjectNotBlocked);

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
