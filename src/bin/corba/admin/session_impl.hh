/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef SESSION_IMPL_HH_7C46029389E449B19F1094E0AB57B071
#define SESSION_IMPL_HH_7C46029389E449B19F1094E0AB57B071

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "src/bin/corba/admin/pagetable_registrars.hh"
#include "src/bin/corba/admin/pagetable_domains.hh"
#include "src/bin/corba/admin/pagetable_contacts.hh"
#include "src/bin/corba/admin/pagetable_nssets.hh"
#include "src/bin/corba/admin/pagetable_keysets.hh"
#include "src/bin/corba/admin/pagetable_publicrequests.hh"
#include "src/bin/corba/admin/pagetable_mails.hh"
#include "src/bin/corba/admin/pagetable_invoices.hh"
#include "src/bin/corba/admin/pagetable_filters.hh"
#include "src/bin/corba/admin/pagetable_files.hh"
// we don't need this at the momment, ccReg_Logger_i is not referenced directly, only through Pagetable_ptr
// #include "pagetable_logger.h"
#include "src/bin/corba/admin/pagetable_logsession.hh"
#include "src/bin/corba/admin/pagetable_zones.hh"
#include "src/bin/corba/admin/pagetable_messages.hh"

#include "src/bin/corba/admin/user_impl.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/corba/file_manager_client.hh"
#include "src/deprecated/libfred/registry.hh"
#include "src/deprecated/libfred/requests/request_manager.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/model/model_filters.hh"

#include "src/util/settings.hh"

#include "src/bin/corba/Logger.hh"


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
  ccReg_Filters_i* m_filters;
  ccReg_User_i* m_user;
  ccReg_Files_i* m_files;
  ccReg_LogSession_i* m_logsession;

  std::unique_ptr<LibFred::Manager> m_registry_manager;
  std::unique_ptr<LibFred::PublicRequest::Manager> m_publicrequest_manager;
  std::unique_ptr<LibFred::Document::Manager> m_document_manager;
  std::unique_ptr<LibFred::Invoicing::Manager> m_invoicing_manager;
  std::unique_ptr<LibFred::Mail::Manager> mail_manager_;
  std::unique_ptr<LibFred::File::Manager> file_manager_;
  std::unique_ptr<LibFred::Logger::Manager> m_logger_manager;
  std::unique_ptr<LibFred::Session::Manager> m_logsession_manager;
  std::unique_ptr<LibFred::Banking::Manager> m_banking_manager;
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
  Registry::Message::Detail* getMessageDetail(ccReg::TID _id);

  /*
   * TODO:
   * this should be rather in separate library - it is only general CORBA-to-Registry
   * mapping
   */

  Registry::Domain::Detail* createHistoryDomainDetail(LibFred::Domain::List* _list);
  Registry::Contact::Detail* createHistoryContactDetail(LibFred::Contact::List* _list);
  Registry::NSSet::Detail* createHistoryNSSetDetail(LibFred::Nsset::List* _list);
  Registry::KeySet::Detail* createHistoryKeySetDetail(LibFred::Keyset::List* _list);
  Registry::Registrar::Detail* createRegistrarDetail(LibFred::Registrar::Registrar* _registrar);
  Registry::PublicRequest::Detail* createPublicRequestDetail(LibFred::PublicRequest::PublicRequest* _request);
  Registry::Mailing::Detail* createMailDetail(LibFred::Mail::Mail *_mail);
  Registry::Invoicing::Detail* createInvoiceDetail(LibFred::Invoicing::Invoice *_invoice);
  Registry::Zone::Detail* createZoneDetail(LibFred::Zone::Zone* _registrar);
  Registry::Message::Detail* createMessageDetail(LibFred::Messages::MessagePtr _message);

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

class CompareSessionsByLastActivity {
public:
  bool operator()(const ccReg_Session_i *_left, const ccReg_Session_i *_right) {
    return _left->getLastActivity() < _right->getLastActivity();
  }
};


#endif /*SESSION_IMPL_H_*/
