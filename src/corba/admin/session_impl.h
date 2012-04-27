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
#include "pagetable_domains.h"
#include "pagetable_contacts.h"
#include "pagetable_nssets.h"
#include "pagetable_keysets.h"
#include "pagetable_publicrequests.h"
#include "pagetable_mails.h"
#include "pagetable_invoices.h"
#include "pagetable_bank_payments.h"
//#include "pagetable_bank_statements.h"
#include "pagetable_filters.h"
#include "pagetable_files.h"
// we don't need this at the momment, ccReg_Logger_i is not referenced directly, only through Pagetable_ptr
// #include "pagetable_logger.h"
#include "pagetable_logsession.h"
#include "pagetable_zones.h"
#include "pagetable_messages.h"

#include "user_impl.h"
#include "corba/mailer_manager.h"
#include "corba/file_manager_client.h"
#include "fredlib/registry.h"
#include "fredlib/requests/request_manager.h"
#include "old_utils/dbsql.h"
#include "model/model_filters.h"

//#include "conf/manager.h"

#include "settings.h"

#include <corba/Logger.hh>


using namespace boost::posix_time;
using namespace boost::gregorian;


class ccReg_Session_i: public POA_ccReg::Session,
                       public PortableServer::RefCountServantBase {
private:
  std::string session_id_;

  //conf
  bool restricted_handles_;
  std::string docgen_path_;
  std::string docgen_template_path_;
  std::string fileclient_path_;
  unsigned adifd_session_timeout_;

  NameService* m_ns;

  ccReg::BankingInvoicing_ptr m_banking_invoicing;

  ccReg_Messages_i* m_messages;
  ccReg_Zones_i* m_zones;
  ccReg_Registrars_i* m_registrars;
  ccReg_Domains_i* m_domains;
  ccReg_Contacts_i* m_contacts;
  ccReg_NSSets_i* m_nssets;
  ccReg_KeySets_i *m_keysets;
  ccReg_PublicRequests_i* m_publicrequests;
  ccReg_Mails_i* m_mails;
  ccReg_Invoices_i* m_invoices;
  ccReg_Payments_i* m_payments;
  // ccReg_Statement_i* m_statements;
  ccReg_Filters_i* m_filters;
  ccReg_User_i* m_user;
  ccReg_Files_i* m_files;  
  ccReg_LogSession_i* m_logsession;  

  std::auto_ptr<Fred::Manager> m_registry_manager;
  std::auto_ptr<Fred::PublicRequest::Manager> m_publicrequest_manager;
  std::auto_ptr<Fred::Document::Manager> m_document_manager;
  std::auto_ptr<Fred::Invoicing::Manager> m_invoicing_manager;
  std::auto_ptr<Fred::Mail::Manager> mail_manager_;
  std::auto_ptr<Fred::File::Manager> file_manager_;
  std::auto_ptr<Fred::Logger::Manager> m_logger_manager;
  std::auto_ptr<Fred::Session::Manager> m_logsession_manager;
  std::auto_ptr<Fred::Banking::Manager> m_banking_manager;
  MailerManager m_mailer_manager;
  FileManagerClient m_fm_client;

  /**
   * context with session object was created - need for futher call on object
   * which are done in separate threads
   */
  std::string base_context_;

  ptime m_last_activity;
  DBSharedPtr db_disconnect_guard_;



  Settings settings_;

  Registry::PageTable_ptr getLoggerPageTable();

  Registry::Domain::Detail* getDomainDetail(ccReg::TID _id);
  Registry::Contact::Detail* getContactDetail(ccReg::TID _id);
  Registry::NSSet::Detail* getNSSetDetail(ccReg::TID _id);
  Registry::KeySet::Detail *getKeySetDetail(ccReg::TID _id);
  Registry::Registrar::Detail* getRegistrarDetail(ccReg::TID _id);
  Registry::PublicRequest::Detail* getPublicRequestDetail(ccReg::TID _id);
  Registry::Mailing::Detail* getMailDetail(ccReg::TID _id);
  Registry::Invoicing::Detail* getInvoiceDetail(ccReg::TID _id);
  ccReg::Logger::Detail*  getLoggerDetail(ccReg::TID _id);
  Registry::Zone::Detail* getZoneDetail(ccReg::TID _id);
  Registry::Banking::BankItem::Detail * getPaymentDetail(ccReg::TID _id);
  //Registry::Banking::BankHead::Detail * getStatementDetail(ccReg::TID _id);
  Registry::Message::Detail* getMessageDetail(ccReg::TID _id);

  /*
   * TODO:
   * this should be rather in separate library - it is only general CORBA-to-Registry
   * mapping
   */

  Registry::Domain::Detail* createHistoryDomainDetail(Fred::Domain::List* _list);
  Registry::Contact::Detail* createHistoryContactDetail(Fred::Contact::List* _list);
  Registry::NSSet::Detail* createHistoryNSSetDetail(Fred::NSSet::List* _list);
  Registry::KeySet::Detail* createHistoryKeySetDetail(Fred::KeySet::List* _list);
  Registry::Registrar::Detail* createRegistrarDetail(Fred::Registrar::Registrar* _registrar);
  Registry::PublicRequest::Detail* createPublicRequestDetail(Fred::PublicRequest::PublicRequest* _request);
  Registry::Mailing::Detail* createMailDetail(Fred::Mail::Mail *_mail);
  Registry::Invoicing::Detail* createInvoiceDetail(Fred::Invoicing::Invoice *_invoice);
  Registry::Zone::Detail* createZoneDetail(Fred::Zone::Zone* _registrar);
  Registry::Banking::BankItem::Detail *createPaymentDetail(Fred::Banking::Payment *_payment);
  //Registry::Banking::BankHead::Detail *createStatementDetail(Fred::Banking::Statement *_statement);
  Registry::Message::Detail* createMessageDetail(Fred::Messages::MessagePtr _message);

public:
  ccReg_Session_i(const std::string& _session_id,
                  const std::string& database,
                  NameService *ns,

                  bool restricted_handles,
                  const std::string& docgen_path,
                  const std::string& docgen_template_path,
                  const std::string& fileclient_path,
                  unsigned adifd_session_timeout,

                  ccReg::BankingInvoicing_ptr _banking,
                  ccReg_User_i* _user);
  ~ccReg_Session_i();

  const std::string& getId() const;
  ccReg::BankingInvoicing_ptr getBankingInvoicing();

  void updateActivity();
  bool isTimeouted() const;
  const ptime& getLastActivity() const;

  Registry::User_ptr getUser();

  Registry::PageTable_ptr getPageTable(ccReg::FilterType _type);
  CORBA::Any* getDetail(ccReg::FilterType _type, ccReg::TID _id);

  ccReg::TID updateRegistrar(const ccReg::AdminRegistrar& _registrar);
  void createRegistrar(const ccReg::AdminRegistrar& _registrar);

  void setHistory(CORBA::Boolean _flag);
};

void fillPaymentDetail(Registry::Banking::BankItem::Detail &d, const Fred::Banking::Payment *_payment);

class CompareSessionsByLastActivity {
public:
  bool operator()(const ccReg_Session_i *_left, const ccReg_Session_i *_right) {
    return _left->getLastActivity() < _right->getLastActivity();
  }
};


#endif /*SESSION_IMPL_H_*/
