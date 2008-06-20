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
#include "db/dbs.h"
#include "model/model_filters.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

class ccReg_Session_i: public POA_ccReg::Session,
                       public PortableServer::RefCountServantBase {
private:
  ccReg_Registrars_i* m_registrars;
  ccReg_EPPActions_i* m_eppactions;
  ccReg_Domains_i* m_domains;
  ccReg_Contacts_i* m_contacts;
  ccReg_NSSets_i* m_nssets;
  ccReg_PublicRequests_i* m_publicrequests;
  ccReg_Mails_i* m_mails;
  ccReg_Invoices_i* m_invoices;
  ccReg_Filters_i* m_filters;
  ccReg_User_i* m_user;
  ccReg_Files_i* m_files;

  std::auto_ptr<DBase::Manager> m_db_manager;
  std::auto_ptr<Register::Manager> m_register_manager;
  std::auto_ptr<Register::PublicRequest::Manager> m_publicrequest_manager;
  std::auto_ptr<Register::Document::Manager> m_document_manager;
  std::auto_ptr<Register::Invoicing::Manager> m_invoicing_manager;
  std::auto_ptr<Register::Mail::Manager> mail_manager_;
  std::auto_ptr<Register::File::Manager> file_manager_;
  MailerManager m_mailer_manager;

  ptime m_last_activity;
  DB db;

  ccReg::DomainDetail* getDomainDetail(ccReg::TID _id);
  ccReg::ContactDetail* getContactDetail(ccReg::TID _id);
  ccReg::NSSetDetail* getNSSetDetail(ccReg::TID _id);
  ccReg::Registrar* getRegistrarDetail(ccReg::TID _id);
  ccReg::PublicRequest::Detail* getPublicRequestDetail(ccReg::TID _id);
  ccReg::Invoicing::Invoice* getInvoiceDetail(ccReg::TID _id);
  ccReg::Mailing::Detail* getMailDetail(ccReg::TID _id);
  
  ccReg::DomainDetail* createDomainDetail(Register::Domain::Domain* _domain);
  ccReg::ContactDetail* createContactDetail(Register::Contact::Contact* _contact);
  ccReg::NSSetDetail* createNSSetDetail(Register::NSSet::NSSet* _contact);
  ccReg::Registrar* createRegistrarDetail(Register::Registrar::Registrar* _registrar);
  ccReg::PublicRequest::Detail* createPublicRequestDetail(Register::PublicRequest::PublicRequest* _request);
  ccReg::Invoicing::Invoice* createInvoiceDetail(Register::Invoicing::Invoice *_invoice);
  ccReg::Mailing::Detail* createMailDetail(Register::Mail::Mail *_mail);
  
  void _createUpdateRegistrar(const ccReg::Registrar& _registrar);

public:
  ccReg_Session_i(const std::string& database,
                  NameService *ns,
                  Conf& cfg,
                  ccReg_User_i* _user);
  ~ccReg_Session_i();

  void updateActivity();
  bool isTimeouted() const;
  
  ccReg::User_ptr getUser();
  
  ccReg::PageTable_ptr getPageTable(ccReg::FilterType _type);
  CORBA::Any* getDetail(ccReg::FilterType _type, ccReg::TID _id);
  
  void updateRegistrar(const ccReg::Registrar& _registrar);
  void createRegistrar(const ccReg::Registrar& _registrar);
};

#endif /*SESSION_IMPL_H_*/
