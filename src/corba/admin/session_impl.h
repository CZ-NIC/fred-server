#ifndef SESSION_IMPL_H_
#define SESSION_IMPL_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "pagetable_impl.h"
#include "pagetable_registrars.h"
#include "pagetable_eppactions.h"
#include "pagetable_domains.h"
#include "pagetable_contacts.h"
#include "pagetable_nssets.h"
#include "pagetable_keysets.h"
#include "pagetable_publicrequests.h"
#include "pagetable_mails.h"
#include "pagetable_invoices.h"
#include "pagetable_filters.h"
#include "pagetable_files.h"

#include "user_impl.h"
#include "corba/mailer_manager.h"
#include "register/register.h"
#include "old_utils/dbsql.h"
#include "old_utils/conf.h"
#include "model/model_filters.h"

#include "conf/manager.h"

#include "settings.h"


using namespace boost::posix_time;
using namespace boost::gregorian;


class ccReg_Session_i: public POA_ccReg::Session,
                       public PortableServer::RefCountServantBase {
private:
  std::string session_id_;
  Config::Conf& cfg_;

  ccReg_Registrars_i* m_registrars;
  ccReg_EPPActions_i* m_eppactions;
  ccReg_Domains_i* m_domains;
  ccReg_Contacts_i* m_contacts;
  ccReg_NSSets_i* m_nssets;
  ccReg_KeySets_i *m_keysets;
  ccReg_PublicRequests_i* m_publicrequests;
  ccReg_Mails_i* m_mails;
  ccReg_Invoices_i* m_invoices;
  ccReg_Filters_i* m_filters;
  ccReg_User_i* m_user;
  ccReg_Files_i* m_files;

  Database::Manager m_db_manager;
  std::auto_ptr<Register::Manager> m_register_manager;
  std::auto_ptr<Register::PublicRequest::Manager> m_publicrequest_manager;
  std::auto_ptr<Register::Document::Manager> m_document_manager;
  std::auto_ptr<Register::Invoicing::Manager> m_invoicing_manager;
  std::auto_ptr<Register::Mail::Manager> mail_manager_;
  std::auto_ptr<Register::File::Manager> file_manager_;
  MailerManager m_mailer_manager;

  /**
   * context with session object was created - need for futher call on object
   * which are done in separate threads 
   */
  std::string base_context_;

  ptime m_last_activity;
  DB db;

  Settings settings_;

  Registry::Domain::Detail* getDomainDetail(ccReg::TID _id);
  Registry::Contact::Detail* getContactDetail(ccReg::TID _id);
  Registry::NSSet::Detail* getNSSetDetail(ccReg::TID _id);
  Registry::KeySet::Detail *getKeySetDetail(ccReg::TID _id);
  Registry::Registrar::Detail* getRegistrarDetail(ccReg::TID _id);
  Registry::EPPAction::Detail* getEppActionDetail(ccReg::TID _id);
  Registry::PublicRequest::Detail* getPublicRequestDetail(ccReg::TID _id);
  Registry::Mailing::Detail* getMailDetail(ccReg::TID _id);
  Registry::Invoicing::Detail* getInvoiceDetail(ccReg::TID _id);

  
  
  /*
   * TODO:
   * this should be rather in separate library - it is only general CORBA-to-Register
   * mapping
   */
  ccReg::DomainDetail* createDomainDetail(Register::Domain::Domain* _domain);
  ccReg::ContactDetail* createContactDetail(Register::Contact::Contact* _contact);
  ccReg::NSSetDetail* createNSSetDetail(Register::NSSet::NSSet* _contact);
  ccReg::KeySetDetail *createKeySetDetail(Register::KeySet::KeySet *_contact);

  Registry::Domain::Detail* createHistoryDomainDetail(Register::Domain::List* _list); 
  Registry::Contact::Detail* createHistoryContactDetail(Register::Contact::List* _list);
  Registry::NSSet::Detail* createHistoryNSSetDetail(Register::NSSet::List* _list);
  Registry::KeySet::Detail* createHistoryKeySetDetail(Register::KeySet::List* _list); 
  Registry::Registrar::Detail* createRegistrarDetail(Register::Registrar::Registrar* _registrar);
  Registry::EPPAction::Detail* createEppActionDetail(Register::Registrar::EPPAction *_action); 
  Registry::PublicRequest::Detail* createPublicRequestDetail(Register::PublicRequest::PublicRequest* _request);
  Registry::Mailing::Detail* createMailDetail(Register::Mail::Mail *_mail);
  Registry::Invoicing::Detail* createInvoiceDetail(Register::Invoicing::Invoice *_invoice);

  void _createUpdateRegistrar(const ccReg::Registrar& _registrar);

public:
  ccReg_Session_i(const std::string& _session_id, 
                  const std::string& database,
                  NameService *ns,
                  Config::Conf& cfg,
                  ccReg_User_i* _user);
  ~ccReg_Session_i();

  const std::string& getId() const;

  void updateActivity();
  bool isTimeouted() const;
  const ptime& getLastActivity() const;
  
  ccReg::User_ptr getUser();
  
  Registry::PageTable_ptr getPageTable(ccReg::FilterType _type);
  CORBA::Any* getDetail(ccReg::FilterType _type, ccReg::TID _id);
  
  void updateRegistrar(const ccReg::Registrar& _registrar);
  void createRegistrar(const ccReg::Registrar& _registrar);

  void setHistory(CORBA::Boolean _flag);
};


class CompareSessionsByLastActivity {
public:
  bool operator()(const ccReg_Session_i *_left, const ccReg_Session_i *_right) {
    return _left->getLastActivity() < _right->getLastActivity();
  }
};


#endif /*SESSION_IMPL_H_*/
