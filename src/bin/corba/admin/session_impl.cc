/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <utility>
#include "src/bin/corba/Admin.hh"

#include "src/bin/corba/admin/public_request_mojeid.hh"
#include "src/bin/corba/admin/session_impl.hh"
#include "src/deprecated/util/log.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/libfred/registry.hh"
#include "src/libfred/notify.hh"
#include "src/libfred/registrar.hh"
#include "src/bin/corba/admin/usertype_conv.hh"
#include "src/bin/corba/admin/common.hh"
#include "src/libfred/public_request/public_request_authinfo_impl.hh"
#include "src/libfred/public_request/public_request_block_impl.hh"
#include "src/libfred/public_request/public_request_personalinfo_impl.hh"
#include "src/backend/contact_verification/public_request_contact_verification_impl.hh"
#include "src/util/log/logger.hh"
#include "src/util/log/context.hh"
#include "src/util/util.hh"

#include "src/bin/corba/connection_releaser.hh"


ccReg_Session_i::ccReg_Session_i(const std::string& _session_id,
                                 const std::string& database,
                                 NameService *ns,

                                 bool restricted_handles,
                                 const std::string& docgen_path,
                                 const std::string& docgen_template_path,
                                 const std::string& fileclient_path,
                                 unsigned adifd_session_timeout,

                                 ccReg::BankingInvoicing_ptr _banking,
                                 ccReg_User_i* _user)
                               : session_id_(_session_id),
                                 restricted_handles_(restricted_handles),
                                 docgen_path_(docgen_path),
                                 docgen_template_path_(docgen_template_path),
                                 fileclient_path_(fileclient_path),
                                 adifd_session_timeout_(adifd_session_timeout),
                                 m_ns (ns),
                                 m_banking_invoicing(_banking),
                                 m_user(_user),
                                 m_mailer_manager(ns),
                                 m_fm_client(ns),
                                 m_last_activity(second_clock::local_time()),
                                 db_disconnect_guard_ ()
{
    Database::Connection conn = Database::Manager::acquire();
    db_disconnect_guard_.reset(new DB(conn));

  base_context_ = Logging::Context::get() + "/" + session_id_;
  Logging::Context ctx(session_id_);

  m_registry_manager.reset(LibFred::Manager::create(db_disconnect_guard_,
                                                     restricted_handles_));

  m_logsession_manager.reset(LibFred::Session::Manager::create());

  m_registry_manager->dbManagerInit();
  m_registry_manager->initStates();

  m_document_manager = LibFred::Document::Manager::create(docgen_path_,
                                                           docgen_template_path_,
                                                           fileclient_path_,
                                                               ns->getHostName());
  m_publicrequest_manager.reset(LibFred::PublicRequest::Manager::create(m_registry_manager->getDomainManager(),
                                                                         m_registry_manager->getContactManager(),
                                                                         m_registry_manager->getNssetManager(),
                                                                         m_registry_manager->getKeysetManager(),
                                                                         &m_mailer_manager,
                                                                         m_document_manager.get(),
                                                                         m_registry_manager->getMessageManager()));
  m_invoicing_manager.reset(LibFred::Invoicing::Manager::create(m_document_manager.get(),
                                                                 &m_mailer_manager));

  mail_manager_.reset(LibFred::Mail::Manager::create());
  file_manager_.reset(LibFred::File::Manager::create(&m_fm_client));
  m_banking_manager.reset(LibFred::Banking::Manager::create(file_manager_.get()));

  m_domains = new ccReg_Domains_i(m_registry_manager->getDomainManager()->createList(), &settings_);
  m_contacts = new ccReg_Contacts_i(m_registry_manager->getContactManager()->createList(), &settings_);
  m_nssets = new ccReg_NSSets_i(m_registry_manager->getNssetManager()->createList(), &settings_);
  m_keysets = new ccReg_KeySets_i(m_registry_manager->getKeysetManager()->createList(), &settings_);
  m_registrars = new ccReg_Registrars_i(m_registry_manager->getRegistrarManager()->createList()
          ,m_registry_manager->getZoneManager()->createList());
  m_invoices = new ccReg_Invoices_i(m_invoicing_manager->createList());
  m_filters = new ccReg_Filters_i(m_registry_manager->getFilterManager()->getList());
  m_publicrequests = new ccReg_PublicRequests_i(m_publicrequest_manager->createList());
  m_payments = new ccReg_Payments_i(m_banking_manager->createPaymentList());
  // m_statementheads = new ccReg_StatementHeads_i(m_banking_manager->createList());
  m_mails = new ccReg_Mails_i(mail_manager_->createList(), ns);
  m_files = new ccReg_Files_i(file_manager_->createList());
  m_logsession = new ccReg_LogSession_i(m_logsession_manager->createList());
  m_zones = new ccReg_Zones_i(m_registry_manager->getZoneManager()->createList());
  m_messages = new ccReg_Messages_i(m_registry_manager->getMessageManager()->createList());

  m_registrars->setDB();
  m_contacts->setDB();
  m_domains->setDB();
  m_nssets->setDB();
  m_keysets->setDB();
  m_publicrequests->setDB();
  m_invoices->setDB();
  m_logsession->setDB();
  m_payments->setDB();
  // m_statementheads->setDB();
  m_zones->setDB();

  settings_.set("filter.history", "off");

  updateActivity();
}

ccReg_Session_i::~ccReg_Session_i() {
  Logging::Context ctx(session_id_);

  TRACE("[CALL] ccReg_Session_i::~ccReg_Session_i()");

  delete m_registrars;
  delete m_domains;
  delete m_contacts;
  delete m_nssets;
  delete m_keysets;
  delete m_publicrequests;
  delete m_mails;
  delete m_invoices;
  delete m_filters;
  delete m_user;
  delete m_files;
  delete m_logsession;
  delete m_payments;
    // delete m_statementheads;
  delete m_zones;
  delete m_messages;

    try {
        ccReg::Logger_ptr logger = ccReg::Logger::_narrow(m_ns->resolve("Logger"));

        if (CORBA::is_nil(logger)) {
            LOGGER(PACKAGE).debug(boost::format("ccReg_Session_i::~ccReg_Session_i: logd isn't running."));
        } else {
            LOGGER(PACKAGE).debug(boost::format("ccReg_Session_i::~ccReg_Session_i: deleting logger pagetable."));
            logger->deletePageTable(session_id_.c_str());

        }
    } catch (CORBA::COMM_FAILURE&) {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Session_i::~ccReg_Session_i: logd isn't running. CORBA exception caught."));
    } catch (CORBA::TRANSIENT&) {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Session_i::~ccReg_Session_i: logd isn't running. CORBA exception caught."));
    } catch (CORBA::SystemException&) {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Session_i::~ccReg_Session_i: logd isn't running. CORBA exception caught."));
    } catch (...) {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Session_i::~ccReg_Session_i: Exception caught."));
    }

}

Registry::User_ptr ccReg_Session_i::getUser() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  return m_user->_this();
}


Registry::PageTable_ptr ccReg_Session_i::getLoggerPageTable()
{
    ccReg::Logger_ptr logger;
    //TODO substitute "Logger"
    try {
        logger = ccReg::Logger::_narrow(m_ns->resolve("Logger"));
    } catch (...) {
        throw ccReg::Admin::ServiceUnavailable();
    }

    if(CORBA::is_nil(logger)) {
        throw ccReg::Admin::ServiceUnavailable();
    }

    Registry::PageTable_ptr pagetable;

    try {
        pagetable = logger->createPageTable(session_id_.c_str());
    } catch(...) {
        throw ccReg::Admin::ServiceUnavailable();
    }

    return Registry::PageTable::_duplicate(pagetable);
}

Registry::PageTable_ptr ccReg_Session_i::getPageTable(ccReg::FilterType _type) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Session_i::getPageTable(%1%)") % _type);
  switch (_type) {
    case ccReg::FT_FILTER:
      return m_filters->_this();
    case ccReg::FT_REGISTRAR:
      return m_registrars->_this();
    case ccReg::FT_OBJ:
      return Registry::PageTable::_nil();
    case ccReg::FT_CONTACT:
      return m_contacts->_this();
    case ccReg::FT_NSSET:
      return m_nssets->_this();
    case ccReg::FT_KEYSET:
      return m_keysets->_this();
    case ccReg::FT_DOMAIN:
      return m_domains->_this();
    case ccReg::FT_INVOICE:
      return m_invoices->_this();
    case ccReg::FT_STATEMENTITEM:
      return m_payments->_this();
    // case ccReg::FT_STATEMENTHEAD:
    //   return m_statementheads->_this();
    case ccReg::FT_PUBLICREQUEST:
      return m_publicrequests->_this();
    case ccReg::FT_MAIL:
      return m_mails->_this();
    case ccReg::FT_FILE:
      return m_files->_this();
    case ccReg::FT_LOGGER:
      return getLoggerPageTable();
    case ccReg::FT_ZONE:
      return m_zones->_this();
    case ccReg::FT_MESSAGE:
      return m_messages->_this();

    default:
      break;
  }
  LOGGER(PACKAGE).debug(boost::format("[ERROR] ccReg_Session_i::getPageTable(%1%): unknown type specified")
      % _type);
  return Registry::PageTable::_nil();
}

CORBA::Any* ccReg_Session_i::getDetail(ccReg::FilterType _type, ccReg::TID _id) {
    try {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Session_i::getDetail(%1%, %2%)") % _type
      % _id);
  CORBA::Any *result = new CORBA::Any;

  switch (_type) {
    case ccReg::FT_CONTACT:
      *result <<= getContactDetail(_id);
      break;

    case ccReg::FT_NSSET:
      *result <<= getNSSetDetail(_id);
      break;

    case ccReg::FT_KEYSET:
      *result <<= getKeySetDetail(_id);
      break;

    case ccReg::FT_DOMAIN:
      *result <<= getDomainDetail(_id);
      break;

    case ccReg::FT_REGISTRAR:
      *result <<= getRegistrarDetail(_id);
      break;

    case ccReg::FT_PUBLICREQUEST:
      *result <<= getPublicRequestDetail(_id);
      break;

    case ccReg::FT_INVOICE:
      *result <<= getInvoiceDetail(_id);
      break;

    case ccReg::FT_MAIL:
      *result <<= getMailDetail(_id);
      break;

    case ccReg::FT_LOGGER:
      *result <<= getLoggerDetail(_id);
      break;

    // case ccReg::FT_STATEMENTHEAD:
    //   *result <<= getStatementDetail(_id);
    //   break;

    case ccReg::FT_STATEMENTITEM:
      *result <<= getPaymentDetail(_id);
      break;

    case ccReg::FT_MESSAGE:
      *result <<= getMessageDetail(_id);
      break;


    case ccReg::FT_SESSION:
      LOGGER(PACKAGE).error("Unimplemented filter type used in getDetail(): FT_SESSION");
      break;
    case ccReg::FT_ZONE:
      LOGGER(PACKAGE).error("Unimplemented filter type used in getDetail(): FT_ZONE");
      break;
    case ccReg::FT_FILE:
      LOGGER(PACKAGE).error("Unimplemented filter type used in getDetail(): FT_FILE");
      break;
    case ccReg::FT_FILTER:
      LOGGER(PACKAGE).error("Unimplemented filter type used in getDetail(): FT_FILTER");
      break;
    case ccReg::FT_OBJ:
      LOGGER(PACKAGE).error("Unimplemented filter type used in getDetail(): FT_OBJ");
      break;
    case ccReg::FT_STATEMENTHEAD:
      LOGGER(PACKAGE).error("Unimplemented filter type used in getDetail(): FT_STATEMENTHEAD");
      break;
    default:
      LOGGER(PACKAGE).error("Invalid filter type used in getDetail()");
      break;
  }

  return result;
    }//try
    catch(Registry::SqlQueryTimeout& ex)
    {
        LOGGER(PACKAGE).error("ccReg_Session_i::getDetail ex: Registry::SqlQueryTimeout");
        throw;
    }
    catch(ccReg::Admin::ServiceUnavailable& ex)
    {
        LOGGER(PACKAGE).error("ccReg_Session_i::getDetail ex: ccReg::Admin::ServiceUnavailable");
        throw;
    }
    catch(ccReg::Admin::ObjectNotFound& ex)
    {
        LOGGER(PACKAGE).warning("ccReg_Session_i::getDetail ex: ccReg::Admin::ObjectNotFound");
        throw;
    }
    catch (ccReg::Logger::OBJECT_NOT_FOUND) {
        throw ccReg::Admin::ObjectNotFound();
    }
    catch (ccReg::Logger::INTERNAL_SERVER_ERROR) {
        throw ccReg::Admin::ServiceUnavailable();
    }
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Session_i::getDetail ex: %1%")
            % ex.what());
        throw ccReg::Admin::ServiceUnavailable();
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Session_i::getDetail unknown exception");
        throw ccReg::Admin::ServiceUnavailable();
    }
}

const std::string& ccReg_Session_i::getId() const {
  return session_id_;
}

ccReg::BankingInvoicing_ptr ccReg_Session_i::getBankingInvoicing() {
    return ccReg::BankingInvoicing::_duplicate(m_banking_invoicing);
}

void ccReg_Session_i::updateActivity() {
  ptime tmp = m_last_activity;
  m_last_activity = second_clock::local_time();
  LOGGER(PACKAGE).debug(boost::format("session activity update: %1% (was %2%)")
      % m_last_activity % tmp);
}

bool ccReg_Session_i::isTimeouted() const {
  ptime threshold = second_clock::local_time() - seconds(adifd_session_timeout_);
  bool timeout = m_last_activity < threshold;
  LOGGER(PACKAGE).debug(boost::format("session `%1%' will timeout in %2% -- session %3%")
                                      % session_id_
                                      % to_simple_string(m_last_activity - threshold)
                                      % (timeout ? "timeout" : "alive"));
  return timeout;
}

const ptime& ccReg_Session_i::getLastActivity() const {
  return m_last_activity;
}

Registry::Domain::Detail* ccReg_Session_i::getDomainDetail(ccReg::TID _id) {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing domain filter for object id=%1%' detail")
        % _id);
    std::unique_ptr<LibFred::Domain::List>
        tmp_domain_list(m_registry_manager->getDomainManager()->createList());

    Database::Filters::Union uf(&settings_);
    Database::Filters::Domain *filter = new Database::Filters::DomainHistoryImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_domain_list->reload(uf);
    unsigned filter_count = tmp_domain_list->getCount();
    if (filter_count > 0) {
      return createHistoryDomainDetail(tmp_domain_list.get());
    }
    else {
      throw ccReg::Admin::ObjectNotFound();
    }
  }
}

Registry::Contact::Detail* ccReg_Session_i::getContactDetail(ccReg::TID _id) {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing contact filter for object id=%1%' detail")
        % _id);
    std::unique_ptr<LibFred::Contact::List>
        tmp_contact_list(m_registry_manager->getContactManager()->createList());

    Database::Filters::Union uf(&settings_);
    Database::Filters::Contact *filter = new Database::Filters::ContactHistoryImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_contact_list->reload(uf);

    unsigned filter_count = tmp_contact_list->getCount();
    if (filter_count > 0) {
      return createHistoryContactDetail(tmp_contact_list.get());
    }
    else {
      throw ccReg::Admin::ObjectNotFound();
    }
  }
}

Registry::NSSet::Detail* ccReg_Session_i::getNSSetDetail(ccReg::TID _id) {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing nsset filter for object id=%1%' detail")
        % _id);
    std::unique_ptr<LibFred::Nsset::List>
        tmp_nsset_list(m_registry_manager->getNssetManager()->createList());

    Database::Filters::Union uf(&settings_);
    Database::Filters::NSSet *filter = new Database::Filters::NSSetHistoryImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_nsset_list->reload(uf);

    unsigned filter_count = tmp_nsset_list->getCount();
    if (filter_count > 0) {
      return createHistoryNSSetDetail(tmp_nsset_list.get());
    }
    else {
      throw ccReg::Admin::ObjectNotFound();
    }
  }
}

Registry::KeySet::Detail *
ccReg_Session_i::getKeySetDetail(ccReg::TID _id)
{
    /* disable cache */
    if (0) {
    }
    else {
        LOGGER(PACKAGE).debug(boost::format("constructing keyset filter for object id=%1%' detail") % _id);

        std::unique_ptr <LibFred::Keyset::List>
            tmp_keyset_list(m_registry_manager->getKeysetManager()->createList());

        Database::Filters::Union uf(&settings_);
        Database::Filters::KeySet *filter = new Database::Filters::KeySetHistoryImpl();
        filter->addId().setValue(Database::ID(_id));
        uf.addFilter(filter);

        tmp_keyset_list->reload(uf);

        unsigned filter_count = tmp_keyset_list->getCount();
        if (filter_count > 0) {
          return createHistoryKeySetDetail(tmp_keyset_list.get());
        }
        else {
          throw ccReg::Admin::ObjectNotFound();
        }
    }
}

Registry::Registrar::Detail* ccReg_Session_i::getRegistrarDetail(ccReg::TID _id) {
  // LibFred::Registrar::Registrar *registrar = m_registrars->findId(_id);
  // if (registrar) {
  //   return createRegistrarDetail(registrar);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing registrar filter for object id=%1%' detail")
        % _id);
    LibFred::Registrar::RegistrarList::AutoPtr tmp_registrar_list =
        m_registry_manager->getRegistrarManager()->createList();

    Database::Filters::Union uf;
    Database::Filters::Registrar *filter = new Database::Filters::RegistrarImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_registrar_list->reload(uf);

    if (tmp_registrar_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createRegistrarDetail(tmp_registrar_list->get(0));
  }
}


Registry::Message::Detail* ccReg_Session_i::getMessageDetail(ccReg::TID _id)
{

    LOGGER(PACKAGE).debug(boost::format("constructing message filter for object id=%1%' detail")
        % _id);
    LibFred::Messages::Manager::MessageListPtr tmp_message_list =
            m_registry_manager->getMessageManager()->createList();

    Database::Filters::Union uf;
    Database::Filters::Message *filter = new Database::Filters::MessageImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_message_list->reload(uf);

    if (tmp_message_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createMessageDetail(tmp_message_list->get(0));
}


Registry::PublicRequest::Detail* ccReg_Session_i::getPublicRequestDetail(ccReg::TID _id) {
  // LibFred::PublicRequest::PublicRequest *request = m_publicrequests->findId(_id);
  // if (request) {
  //   return createPublicRequestDetail(request);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing public request filter for object id=%1%' detail")
        % _id);
    std::unique_ptr<LibFred::PublicRequest::List> tmp_request_list(m_publicrequest_manager->createList());

    Database::Filters::Union union_filter;
    Database::Filters::PublicRequest *filter = new Database::Filters::PublicRequestImpl();
    filter->addId().setValue(Database::ID(_id));
    union_filter.addFilter(filter);

    tmp_request_list->reload(union_filter);

    if (tmp_request_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createPublicRequestDetail(tmp_request_list->get(0));
  }
}

Registry::Invoicing::Detail* ccReg_Session_i::getInvoiceDetail(ccReg::TID _id) {
  // LibFred::Invoicing::Invoice *invoice = m_invoices->findId(_id);
  // if (invoice && invoice->getActionCount()) {
  //   return createInvoiceDetail(invoice);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing invoice filter for object id=%1%' detail")
        % _id);
    std::unique_ptr<LibFred::Invoicing::List> tmp_invoice_list(m_invoicing_manager->createList());

    Database::Filters::Union union_filter;
    Database::Filters::Invoice *filter = new Database::Filters::InvoiceImpl();
    filter->addId().setValue(Database::ID(_id));
    union_filter.addFilter(filter);

    tmp_invoice_list->reload(union_filter);

    if (tmp_invoice_list->getSize() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createInvoiceDetail(tmp_invoice_list->get(0));
  }
}

Registry::Mailing::Detail* ccReg_Session_i::getMailDetail(ccReg::TID _id) {
  // LibFred::Mail::Mail *mail = m_mails->findId(_id);
  // if (mail) {
  //   return createMailDetail(mail);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing mail filter for object id=%1%' detail")
        % _id);
    std::unique_ptr<LibFred::Mail::List> tmp_mail_list(mail_manager_->createList());

    Database::Filters::Union union_filter;
    Database::Filters::Mail *filter = new Database::Filters::MailImpl();
    filter->addId().setValue(Database::ID(_id));
    union_filter.addFilter(filter);

    tmp_mail_list->reload(union_filter);

    if (tmp_mail_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createMailDetail(tmp_mail_list->get(0));
  }
}


// ccReg::Logger::Detail*  ccReg_Session_i::getRequestDetail(ccReg::TID _id) {
ccReg::Logger::Detail*  ccReg_Session_i::getLoggerDetail(ccReg::TID _id) {

        ccReg::Logger_ptr logger;

	LOGGER(PACKAGE).debug(boost::format("constructing request filter for object id=%1% detail") % _id);

        try {
            logger = ccReg::Logger::_narrow(m_ns->resolve("Logger"));
        } catch (...) {
            throw ccReg::Admin::ServiceUnavailable();
        }

        if (CORBA::is_nil(logger)) throw ccReg::Admin::ServiceUnavailable();

        return logger->getDetail(_id);

}

Registry::Banking::BankItem::Detail *ccReg_Session_i::getPaymentDetail(ccReg::TID _id) {
	LOGGER(PACKAGE).debug(boost::format("constructing bank item filter for object id=%1% detail") % _id);

	std::unique_ptr<LibFred::Banking::PaymentList> item_list(m_banking_manager->createPaymentList());

	Database::Filters::Union union_filter;
	Database::Filters::BankPayment *filter = new Database::Filters::BankPaymentImpl();

	filter->addId().setValue(Database::ID(_id));
	union_filter.addFilter(filter);

        // TODO
	// item_list->setPartialLoad(false);
	item_list->reload(union_filter);

	if(item_list->size() != 1) {
		throw ccReg::Admin::ObjectNotFound();
	}
	return createPaymentDetail(item_list->get(0));

}

/*
Registry::Banking::BankHead::Detail *ccReg_Session_i::getStatementDetail(ccReg::TID _id) {
	LOGGER(PACKAGE).debug(boost::format("constructing bank item filter for object id=%1% detail") % _id);

	std::auto_ptr<LibFred::Banking::StatementList> list(m_banking_manager->createStatementList());

	Database::Filters::Union union_filter;
	Database::Filters::BankStatement *filter = new Database::Filters::BankStatementImpl();

	filter->addId().setValue(Database::ID(_id));
	union_filter.addFilter(filter);

	list->setPartialLoad(false);
	list->reload(union_filter);

	if (list->size() != 1) {
		throw ccReg::Admin::ObjectNotFound();
	}
	return createStatementDetail(head_list->get(0));

}
*/

Registry::Zone::Detail* ccReg_Session_i::getZoneDetail(ccReg::TID _id) {
  // LibFred::Zone::Zone *zone = m_zones->findId(_id);
  // if (zone) {
  //   return createZoneDetail(zone);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing zone filter for object id=%1%' detail")
        % _id);
    LibFred::Zone::Manager::ZoneListPtr tmp_zone_list =
        m_registry_manager->getZoneManager()->createList();

    Database::Filters::Union uf;
    Database::Filters::Zone *filter = new Database::Filters::ZoneImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_zone_list->reload(uf);

    if (tmp_zone_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createZoneDetail(dynamic_cast<LibFred::Zone::Zone*>(tmp_zone_list->get(0)));
  }
}

void fillPaymentDetail(Registry::Banking::BankItem::Detail &d, const LibFred::Banking::Payment *_payment)
{
        d.id              = _payment->getId();
        d.statementId     = _payment->getStatementId();
        d.accountNumber   = DUPSTRFUN(_payment->getAccountNumber);
        d.bankCodeId      = DUPSTRFUN(_payment->getBankCode);
        d.code            = _payment->getCode();
        d.type            = _payment->getType();
        d.konstSym        = DUPSTRFUN(_payment->getKonstSym);
        d.varSymb         = DUPSTRFUN(_payment->getVarSymb);
        d.specSymb        = DUPSTRFUN(_payment->getSpecSymb);
        d.price           = DUPSTRC(formatMoney(_payment->getPrice()));
        d.accountEvid     = DUPSTRFUN(_payment->getAccountEvid);
        d.accountDate     = DUPSTRDATED(_payment->getAccountDate);
        d.accountMemo     = DUPSTRFUN(_payment->getAccountMemo);
        d.invoiceId       = _payment->getAdvanceInvoiceId();
        d.accountName     = DUPSTRFUN(_payment->getAccountName);
        d.crTime          = DUPSTRDATE(_payment->getCrTime);
        d.destAccountNumber = DUPSTRFUN(_payment->getDestAccount);
}

Registry::Banking::BankItem::Detail *ccReg_Session_i::createPaymentDetail(LibFred::Banking::Payment *_payment) {
        Registry::Banking::BankItem::Detail *detail = new Registry::Banking::BankItem::Detail();

        fillPaymentDetail(*detail, _payment);
	return detail;
}

Registry::Domain::Detail* ccReg_Session_i::createHistoryDomainDetail(LibFred::Domain::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryDomainDetail()");
  Registry::Domain::Detail *detail = new Registry::Domain::Detail();

  /* array for object external states already assigned */
  std::vector<LibFred::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    LibFred::Domain::Domain *act  = _list->getDomain(n);
    LibFred::Domain::Domain *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getDomain(n + 1));

    /* just copy static data */
    if (act == prev) {
      detail->id = act->getId();
      detail->handle = DUPSTRFUN(act->getHandle);
      detail->roid = DUPSTRFUN(act->getROID);
      detail->transferDate = DUPSTRDATE(act->getTransferDate);
      detail->updateDate = DUPSTRDATE(act->getUpdateDate);
      detail->createDate = DUPSTRDATE(act->getCreateDate);
      /**
       * we want to display date of past deletion or future "cancel" date value
       * it is what getCancelDate() method do
       */
      detail->deleteDate  = DUPSTRDATE(act->getCancelDate);
      detail->outZoneDate = DUPSTRDATE(act->getOutZoneDate);

      detail->createRegistrar.id     = act->getCreateRegistrarId();
      detail->createRegistrar.handle = DUPSTRFUN(act->getCreateRegistrarHandle);
      detail->createRegistrar.type   = ccReg::FT_REGISTRAR;

      detail->updateRegistrar.id     = act->getUpdateRegistrarId();
      detail->updateRegistrar.handle = DUPSTRFUN(act->getUpdateRegistrarHandle);
      detail->updateRegistrar.type   = ccReg::FT_REGISTRAR;
    }

    /* macros are defined in common.h */

    MAP_HISTORY_OID(registrar, getRegistrarId, getRegistrarHandle, ccReg::FT_REGISTRAR)
    MAP_HISTORY_STRING(authInfo, getAuthPw)

    /* object status */
    for (unsigned s = 0; s < act->getStatusCount(); ++s) {
      const LibFred::Status *tmp = act->getStatusByIdx(s);

      LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<LibFred::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
      if (it == ext_states_ids.end()) {
        ext_states_ids.push_back(tmp->getId());
        LOGGER(PACKAGE).debug(boost::format("history detail -- state %1% is added for output") % tmp->getStatusId());

        unsigned sl = detail->states.length();
        detail->states.length(sl + 1);
        detail->states[sl].id    = tmp->getStatusId();
        detail->states[sl].from  = makeCorbaTime(tmp->getFrom(), true);
        detail->states[sl].to    = makeCorbaTime(tmp->getTo(), true);
      }
    }

    /* domain specific follows */
    MAP_HISTORY_OID(registrant, getRegistrantId, getRegistrantHandle, ccReg::FT_CONTACT)
    MAP_HISTORY_DATE(expirationDate, getExpirationDate)
    MAP_HISTORY_DATE(valExDate, getValExDate)
    MAP_HISTORY_BOOL(publish, getPublish)
    MAP_HISTORY_OID(nsset, getNssetId, getNssetHandle, ccReg::FT_NSSET)
    MAP_HISTORY_OID(keyset, getKeysetId, getKeysetHandle, ccReg::FT_KEYSET)

    /* admin list */
    try {
      bool alist_changed = (act->getAdminCount(1) != prev->getAdminCount(1)) || (act == prev);
      for (unsigned n = 0; alist_changed != true && n < act->getAdminCount(1); ++n) {
        if (act->getAdminIdByIdx(n, 1) != prev->getAdminIdByIdx(n, 1)) {
          alist_changed = true;
          break;
        }
      }
      if (alist_changed) {
        Registry::OIDSeq oid_seq;
        oid_seq.length(act->getAdminCount(1));
        for (unsigned n = 0; n < act->getAdminCount(1); ++n) {
          oid_seq[n].id     = act->getAdminIdByIdx(n, 1);
          oid_seq[n].handle = DUPSTRC(act->getAdminHandleByIdx(n, 1));
          oid_seq[n].type   = ccReg::FT_CONTACT;
        }
        ADD_NEW_HISTORY_RECORD(admins, oid_seq)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(admins)
      }
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("domain id=%1% detail lib -> CORBA: request for admin contact out of range 0..%2%")
                                           % act->getId() % act->getAdminCount(1));
    }

    /* temp list */
    try {
      bool tlist_changed = (act->getAdminCount(2) != prev->getAdminCount(2)) || (act == prev);
      for (unsigned n = 0; tlist_changed != true && n < act->getAdminCount(2); ++n) {
        if (act->getAdminIdByIdx(n, 2) != prev->getAdminIdByIdx(n, 2)) {
          tlist_changed = true;
          break;
        }
      }
      if (tlist_changed) {
        Registry::OIDSeq oid_seq;
        oid_seq.length(act->getAdminCount(2));
        for (unsigned n = 0; n < act->getAdminCount(2); ++n) {
          oid_seq[n].id     = act->getAdminIdByIdx(n, 2);
          oid_seq[n].handle = DUPSTRC(act->getAdminHandleByIdx(n, 2));
          oid_seq[n].type   = ccReg::FT_CONTACT;
        }
        ADD_NEW_HISTORY_RECORD(temps, oid_seq)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(temps)
      }
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("domain id=%1% detail lib -> CORBA: request for temp contact out of range 0..%2%")
                                           % act->getId() % act->getAdminCount(2));
    }
  }

  return detail;
}


Registry::Contact::Detail* ccReg_Session_i::createHistoryContactDetail(LibFred::Contact::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryContactDetail()");
  Registry::Contact::Detail *detail = new Registry::Contact::Detail();

  /* array for object external states already assigned */
  std::vector<LibFred::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    LibFred::Contact::Contact *act  = _list->getContact(n);
    LibFred::Contact::Contact *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getContact(n + 1));

    LOGGER(PACKAGE).debug(boost::format("history detail -- iteration left %1%, history id is %2%") % n % act->getHistoryId());

    /* just copy static data */
    if (act == prev) {
      detail->id = act->getId();
      detail->handle = DUPSTRFUN(act->getHandle);
      detail->roid = DUPSTRFUN(act->getROID);
      detail->transferDate = DUPSTRDATE(act->getTransferDate);
      detail->updateDate = DUPSTRDATE(act->getUpdateDate);
      detail->createDate = DUPSTRDATE(act->getCreateDate);
      detail->deleteDate = DUPSTRDATE(act->getDeleteDate);

      detail->createRegistrar.id     = act->getCreateRegistrarId();
      detail->createRegistrar.handle = DUPSTRFUN(act->getCreateRegistrarHandle);
      detail->createRegistrar.type   = ccReg::FT_REGISTRAR;

      detail->updateRegistrar.id     = act->getUpdateRegistrarId();
      detail->updateRegistrar.handle = DUPSTRFUN(act->getUpdateRegistrarHandle);
      detail->updateRegistrar.type   = ccReg::FT_REGISTRAR;
    }

    /* macros are defined in common.h */

    MAP_HISTORY_OID(registrar, getRegistrarId, getRegistrarHandle, ccReg::FT_REGISTRAR)
    MAP_HISTORY_STRING(authInfo, getAuthPw)

    /* object status */
    for (unsigned s = 0; s < act->getStatusCount(); ++s) {
      const LibFred::Status *tmp = act->getStatusByIdx(s);
LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<LibFred::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
      if (it == ext_states_ids.end()) {
        ext_states_ids.push_back(tmp->getId());
        LOGGER(PACKAGE).debug(boost::format("history detail -- state %1% is added for output") % tmp->getStatusId());

        unsigned sl = detail->states.length();
        detail->states.length(sl + 1);
        detail->states[sl].id    = tmp->getStatusId();
        detail->states[sl].from  = makeCorbaTime(tmp->getFrom(), true);
        detail->states[sl].to    = makeCorbaTime(tmp->getTo(), true);
      }
    }

    /* contact specific data follows */

    MAP_HISTORY_STRING(name, getName)
    MAP_HISTORY_STRING(organization, getOrganization)
    MAP_HISTORY_STRING(street1, getStreet1)
    MAP_HISTORY_STRING(street2, getStreet2)
    MAP_HISTORY_STRING(street3, getStreet3)
    MAP_HISTORY_STRING(province, getProvince)
    MAP_HISTORY_STRING(postalcode, getPostalCode)
    MAP_HISTORY_STRING(city, getCity)
    MAP_HISTORY_STRING(country, getCountry)
    MAP_HISTORY_STRING(telephone, getTelephone)
    MAP_HISTORY_STRING(fax, getFax)
    MAP_HISTORY_STRING(email, getEmail)
    MAP_HISTORY_STRING(notifyEmail, getNotifyEmail)

    /* TODO: rename `SSN' methods in library to `ident' */
    MAP_HISTORY_STRING(ident, getSSN)
    MAP_HISTORY_STRING(identType, getSSNType)

    MAP_HISTORY_STRING(vat, getVAT)
    MAP_HISTORY_BOOL(discloseName, getDiscloseName)
    MAP_HISTORY_BOOL(discloseOrganization, getDiscloseOrganization)
    MAP_HISTORY_BOOL(discloseEmail, getDiscloseEmail)
    MAP_HISTORY_BOOL(discloseAddress, getDiscloseAddr)
    MAP_HISTORY_BOOL(discloseTelephone, getDiscloseTelephone)
    MAP_HISTORY_BOOL(discloseFax, getDiscloseFax)
    MAP_HISTORY_BOOL(discloseIdent, getDiscloseIdent)
    MAP_HISTORY_BOOL(discloseVat, getDiscloseVat)
    MAP_HISTORY_BOOL(discloseNotifyEmail, getDiscloseNotifyEmail)

    /* address list */
    try {
      bool addr_list_changed = (act->getAddressCount() != prev->getAddressCount()) || (act == prev);
      if (!addr_list_changed) {
        for (unsigned i = 0; i < act->getAddressCount(); ++i) {
          if (*(act->getAddressByIdx(i)) != *(prev->getAddressByIdx(i))) {
            addr_list_changed = true;
            break;
          }
        }
      }
      if (addr_list_changed) {
        Registry::Contact::AddressSeq addresses;
        addresses.length(act->getAddressCount());
        for (unsigned k = 0; k < act->getAddressCount(); ++k) {
          addresses[k].type        = DUPSTRFUN(act->getAddressByIdx(k)->getType);
          addresses[k].companyName = DUPSTRFUN(act->getAddressByIdx(k)->getCompanyName);
          addresses[k].street1     = DUPSTRFUN(act->getAddressByIdx(k)->getStreet1);
          addresses[k].street2     = DUPSTRFUN(act->getAddressByIdx(k)->getStreet2);
          addresses[k].street3     = DUPSTRFUN(act->getAddressByIdx(k)->getStreet3);
          addresses[k].province    = DUPSTRFUN(act->getAddressByIdx(k)->getProvince);
          addresses[k].postalcode  = DUPSTRFUN(act->getAddressByIdx(k)->getPostalCode);
          addresses[k].city        = DUPSTRFUN(act->getAddressByIdx(k)->getCity);
          addresses[k].country     = DUPSTRFUN(act->getAddressByIdx(k)->getCountry);
          LOGGER(PACKAGE).debug(boost::format("contact id=%1% add address to output type=%2%")
                  % act->getId() % act->getAddressByIdx(k)->getType());
        }
        ADD_NEW_HISTORY_RECORD(addresses, addresses)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(addresses)
      }
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("contact id=%1% detail lib -> CORBA: request for address out of range 0..%2%")
                                           % act->getId() % act->getAddressCount());
    }
  }

  return detail;
}

Registry::NSSet::Detail* ccReg_Session_i::createHistoryNSSetDetail(LibFred::Nsset::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryNSSetDetail()");
  Registry::NSSet::Detail *detail = new Registry::NSSet::Detail();

  /* array for object external states already assigned */
  std::vector<LibFred::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    LibFred::Nsset::Nsset *act  = _list->getNsset(n);
    LibFred::Nsset::Nsset *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getNsset(n + 1));

    /* just copy static data */
    if (act == prev) {
      detail->id = act->getId();
      detail->handle = DUPSTRFUN(act->getHandle);
      detail->roid = DUPSTRFUN(act->getROID);
      detail->transferDate = DUPSTRDATE(act->getTransferDate);
      detail->updateDate = DUPSTRDATE(act->getUpdateDate);
      detail->createDate = DUPSTRDATE(act->getCreateDate);
      detail->deleteDate = DUPSTRDATE(act->getDeleteDate);

      detail->createRegistrar.id     = act->getCreateRegistrarId();
      detail->createRegistrar.handle = DUPSTRFUN(act->getCreateRegistrarHandle);
      detail->createRegistrar.type   = ccReg::FT_REGISTRAR;

      detail->updateRegistrar.id     = act->getUpdateRegistrarId();
      detail->updateRegistrar.handle = DUPSTRFUN(act->getUpdateRegistrarHandle);
      detail->updateRegistrar.type   = ccReg::FT_REGISTRAR;
    }

    /* macros are defined in common.h */

    MAP_HISTORY_OID(registrar, getRegistrarId, getRegistrarHandle, ccReg::FT_REGISTRAR)
    MAP_HISTORY_STRING(authInfo, getAuthPw)

    /* object status */
    for (unsigned s = 0; s < act->getStatusCount(); ++s) {
      const LibFred::Status *tmp = act->getStatusByIdx(s);

      LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<LibFred::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
      if (it == ext_states_ids.end()) {
        ext_states_ids.push_back(tmp->getId());
        LOGGER(PACKAGE).debug(boost::format("history detail -- state %1% is added for output") % tmp->getStatusId());

        unsigned sl = detail->states.length();
        detail->states.length(sl + 1);
        detail->states[sl].id    = tmp->getStatusId();
        detail->states[sl].from  = makeCorbaTime(tmp->getFrom(), true);
        detail->states[sl].to    = makeCorbaTime(tmp->getTo(), true);
      }
    }

    /* nsset specific data follows */

    /* admin list */
    try {
      bool alist_changed = (act->getAdminCount() != prev->getAdminCount()) || (act == prev);
      for (unsigned n = 0; alist_changed != true && n < act->getAdminCount(); ++n) {
        if (act->getAdminIdByIdx(n) != prev->getAdminIdByIdx(n)) {
          alist_changed = true;
          break;
        }
      }
      if (alist_changed) {
        Registry::OIDSeq oid_seq;
        oid_seq.length(act->getAdminCount());
        for (unsigned n = 0; n < act->getAdminCount(); ++n) {
          oid_seq[n].id     = act->getAdminIdByIdx(n);
          oid_seq[n].handle = DUPSTRC(act->getAdminHandleByIdx(n));
          oid_seq[n].type   = ccReg::FT_CONTACT;
        }
        ADD_NEW_HISTORY_RECORD(admins, oid_seq)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(admins)
      }

      MAP_HISTORY_STRING(reportLevel, getCheckLevel)
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("nsset id=%1% detail lib -> CORBA: request for admin contact out of range 0..%2%")
                                           % act->getId() % act->getAdminCount());
    }

    /* dns host list */
    try {
      bool hlist_changed = (act->getHostCount() != prev->getHostCount()) || (act == prev);
      for (unsigned i = 0; hlist_changed != true && i < act->getHostCount(); ++i) {
        if (*(act->getHostByIdx(i)) != *(prev->getHostByIdx(i))) {
          hlist_changed = true;
          break;
        }
      }
      if (hlist_changed) {
        ccReg::DNSHost dns_hosts;
        dns_hosts.length(act->getHostCount());
        for (unsigned k = 0; k < act->getHostCount(); ++k) {
          dns_hosts[k].fqdn = DUPSTRFUN(act->getHostByIdx(k)->getName);
          dns_hosts[k].inet.length(act->getHostByIdx(k)->getAddrCount());
          LOGGER(PACKAGE).debug(boost::format("nsset id=%1% detail lib -> CORBA: dns host `%2%' has %3% ip addresses")
                                               % act->getId() % act->getHostByIdx(k)->getName() % act->getHostByIdx(k)->getAddrCount());
          for (unsigned j = 0; j < act->getHostByIdx(k)->getAddrCount(); ++j) {
            dns_hosts[k].inet[j] = DUPSTRC(act->getHostByIdx(k)->getAddrByIdx(j));
          }
        }
        ADD_NEW_HISTORY_RECORD(hosts, dns_hosts)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(hosts)
      }
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("nsset id=%1% detail lib -> CORBA: request for host out of range 0..%2%")
                                           % act->getId() % act->getHostCount());
    }

  }

  return detail;
}


Registry::KeySet::Detail* ccReg_Session_i::createHistoryKeySetDetail(LibFred::Keyset::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryKeySetDetail()");
  Registry::KeySet::Detail *detail = new Registry::KeySet::Detail();

  /* array for object external states already assigned */
  std::vector<LibFred::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    LibFred::Keyset::Keyset *act  = _list->getKeyset(n);
    LibFred::Keyset::Keyset *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getKeyset(n + 1));

    /* just copy static data */
    if (act == prev) {
      detail->id = act->getId();
      detail->handle = DUPSTRFUN(act->getHandle);
      detail->roid = DUPSTRFUN(act->getROID);
      detail->transferDate = DUPSTRDATE(act->getTransferDate);
      detail->updateDate = DUPSTRDATE(act->getUpdateDate);
      detail->createDate = DUPSTRDATE(act->getCreateDate);
      detail->deleteDate = DUPSTRDATE(act->getDeleteDate);

      detail->createRegistrar.id     = act->getCreateRegistrarId();
      detail->createRegistrar.handle = DUPSTRFUN(act->getCreateRegistrarHandle);
      detail->createRegistrar.type   = ccReg::FT_REGISTRAR;

      detail->updateRegistrar.id     = act->getUpdateRegistrarId();
      detail->updateRegistrar.handle = DUPSTRFUN(act->getUpdateRegistrarHandle);
      detail->updateRegistrar.type   = ccReg::FT_REGISTRAR;
    }

    /* macros are defined in common.h */

    MAP_HISTORY_OID(registrar, getRegistrarId, getRegistrarHandle, ccReg::FT_REGISTRAR)
    MAP_HISTORY_STRING(authInfo, getAuthPw)

    /* object status */
    for (unsigned s = 0; s < act->getStatusCount(); ++s) {
      const LibFred::Status *tmp = act->getStatusByIdx(s);

      LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<LibFred::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
      if (it == ext_states_ids.end()) {
        ext_states_ids.push_back(tmp->getId());
        LOGGER(PACKAGE).debug(boost::format("history detail -- state %1% is added for output") % tmp->getStatusId());

        unsigned sl = detail->states.length();
        detail->states.length(sl + 1);
        detail->states[sl].id    = tmp->getStatusId();
        detail->states[sl].from  = makeCorbaTime(tmp->getFrom(), true);
        detail->states[sl].to    = makeCorbaTime(tmp->getTo(), true);
      }
    }

    /* keyset specific data follows */

    /* admin list */
    try {
      bool alist_changed = (act->getAdminCount() != prev->getAdminCount()) || (act == prev);
      for (unsigned n = 0; alist_changed != true && n < act->getAdminCount(); ++n) {
        if (act->getAdminIdByIdx(n) != prev->getAdminIdByIdx(n)) {
          alist_changed = true;
          break;
        }
      }
      if (alist_changed) {
        Registry::OIDSeq oid_seq;
        oid_seq.length(act->getAdminCount());
        for (unsigned n = 0; n < act->getAdminCount(); ++n) {
          oid_seq[n].id     = act->getAdminIdByIdx(n);
          oid_seq[n].handle = DUPSTRC(act->getAdminHandleByIdx(n));
          oid_seq[n].type   = ccReg::FT_CONTACT;
        }
        ADD_NEW_HISTORY_RECORD(admins, oid_seq)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(admins)
      }
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("keyset id=%1% detail lib -> CORBA: request for admin contact out of range 0..%2%")
                                           % act->getId() % act->getAdminCount());
    }

    /* dsrecord list */
    try {
      bool dslist_changed = (act->getDSRecordCount() != prev->getDSRecordCount()) || (act == prev);
      for (unsigned i = 0; dslist_changed != true && i < act->getDSRecordCount(); ++i) {
        if (*(act->getDSRecordByIdx(i)) != *(prev->getDSRecordByIdx(i))) {
          dslist_changed = true;
          break;
        }
      }
      if (dslist_changed) {
        ccReg::DSRecord dsrecords;
        dsrecords.length(act->getDSRecordCount());
        for (unsigned k = 0; k < act->getDSRecordCount(); ++k) {
          dsrecords[k].keyTag     = act->getDSRecordByIdx(k)->getKeyTag();
          dsrecords[k].alg        = act->getDSRecordByIdx(k)->getAlg();
          dsrecords[k].digestType = act->getDSRecordByIdx(k)->getDigestType();
          dsrecords[k].digest     = DUPSTRFUN(act->getDSRecordByIdx(k)->getDigest);
          dsrecords[k].maxSigLife = act->getDSRecordByIdx(k)->getMaxSigLife();
        }
        ADD_NEW_HISTORY_RECORD(dsrecords, dsrecords)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(dsrecords)
      }
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("keyset id=%1% detail lib -> CORBA: request for dsrecord out of range 0..%2%")
                                           % act->getId() % act->getDSRecordCount());
    }

    /* dnskey list */
    try {
      bool dnslist_changed = (act->getDNSKeyCount() != prev->getDNSKeyCount()) || (act == prev);
      for (unsigned i = 0; dnslist_changed != true && i < act->getDNSKeyCount(); ++i) {
        if (*(act->getDNSKeyByIdx(i)) != *(prev->getDNSKeyByIdx(i))) {
          dnslist_changed = true;
          break;
        }
      }
      if (dnslist_changed) {
        ccReg::DNSKey dnskeys;
        dnskeys.length(act->getDNSKeyCount());
        for (unsigned k = 0; k < act->getDNSKeyCount(); ++k) {
          dnskeys[k].flags     = act->getDNSKeyByIdx(k)->getFlags();
          dnskeys[k].protocol  = act->getDNSKeyByIdx(k)->getProtocol();
          dnskeys[k].alg       = act->getDNSKeyByIdx(k)->getAlg();
          dnskeys[k].key       = DUPSTRFUN(act->getDNSKeyByIdx(k)->getKey);
          LOGGER(PACKAGE).debug(boost::format("keyset id=%1% detail lib -> CORBA: dnskey added to output "
                                              "(flags=%2% protocol=%3% alg=%4% key=%5%)")
                                              % act->getId()
                                              % act->getDNSKeyByIdx(k)->getFlags()
                                              % act->getDNSKeyByIdx(k)->getProtocol()
                                              % act->getDNSKeyByIdx(k)->getAlg()
                                              % act->getDNSKeyByIdx(k)->getKey());
        }
        ADD_NEW_HISTORY_RECORD(dnskeys, dnskeys)
      }
      else {
        MODIFY_LAST_HISTORY_RECORD(dnskeys)
      }
    }
    catch (LibFred::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("keyset id=%1% detail lib -> CORBA: request for dnskey out of range 0..%2%")
                                           % act->getId() % act->getDNSKeyCount());
    }

  }

  return detail;
}

Registry::Registrar::Detail* ccReg_Session_i::createRegistrarDetail(LibFred::Registrar::Registrar* _registrar) {
  TRACE("[CALL] ccReg_Session_i::createRegistrarDetail()");
  LOGGER(PACKAGE).debug(boost::format("generating registrar detail for object id=%1%")
      % _registrar->getId());
  Registry::Registrar::Detail *detail = new Registry::Registrar::Detail();

  detail->id = _registrar->getId();
  detail->ico = DUPSTRFUN(_registrar->getIco);
  detail->dic = DUPSTRFUN(_registrar->getDic);
  detail->varSymb = DUPSTRFUN(_registrar->getVarSymb);
  detail->vat = _registrar->getVat();
  detail->name = DUPSTRFUN(_registrar->getName);
  detail->handle = DUPSTRFUN(_registrar->getHandle);
  detail->url = DUPSTRFUN(_registrar->getURL);
  detail->organization = DUPSTRFUN(_registrar->getOrganization);
  detail->street1 = DUPSTRFUN(_registrar->getStreet1);
  detail->street2 = DUPSTRFUN(_registrar->getStreet2);
  detail->street3 = DUPSTRFUN(_registrar->getStreet3);
  detail->city = DUPSTRFUN(_registrar->getCity);
  detail->postalcode = DUPSTRFUN(_registrar->getPostalCode);
  detail->stateorprovince = DUPSTRFUN(_registrar->getProvince);
  detail->country = DUPSTRFUN(_registrar->getCountry);
  detail->telephone = DUPSTRFUN(_registrar->getTelephone);
  detail->fax = DUPSTRFUN(_registrar->getFax);
  detail->email = DUPSTRFUN(_registrar->getEmail);
  detail->credit = DUPSTRC(formatMoney(_registrar->getCredit()));
  detail->unspec_credit = DUPSTRC(formatMoney(_registrar->getCredit(0)));

  detail->access.length(_registrar->getACLSize());
  for (unsigned i = 0; i < _registrar->getACLSize(); i++) {
    detail->access[i].id = _registrar->getACL(i)->getId();
    detail->access[i].md5Cert = DUPSTRFUN(_registrar->getACL(i)->getCertificateMD5);
    detail->access[i].password = "";
  }
  detail->zones.length(_registrar->getZoneAccessSize());
  for (unsigned i = 0; i < _registrar->getZoneAccessSize(); i++)
  {
    detail->zones[i].id = _registrar->getZoneAccess(i)->id;
    detail->zones[i].name = CORBA::string_dup(_registrar->getZoneAccess(i)->name.c_str());
    detail->zones[i].credit = CORBA::string_dup(C_STR(_registrar->getZoneAccess(i)->credit));//_registrar->getCredit(__registrar->getAZone(i)->id);
    detail->zones[i].fromDate = makeCorbaDate(_registrar->getZoneAccess(i)->fromdate);//CORBA::string_dup(_registrar->getZoneAccess(i)->fromdate.to_string().c_str());
    detail->zones[i].toDate = makeCorbaDate(_registrar->getZoneAccess(i)->todate);
  }

  detail->hidden = _registrar->getSystem();

  return detail;
}

ccReg::TID ccReg_Session_i::updateRegistrar(const ccReg::AdminRegistrar& _registrar)
{
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_Session_i::updateRegistrar()");
  LibFred::Registrar::RegistrarList::AutoPtr tmp_registrar_list =
      m_registry_manager->getRegistrarManager()->createList();
  LibFred::Registrar::Registrar::AutoPtr  update_registrar_guard;//delete at the end
  LibFred::Registrar::Registrar* update_registrar; // registrar to be created or updated

  if (!_registrar.id)
  {
    LOGGER(PACKAGE).debug("no registrar id specified; creating new registrar...");
    update_registrar_guard = m_registry_manager->getRegistrarManager()->createRegistrar();
    update_registrar = update_registrar_guard.get();
  }
  else
  {
    LOGGER(PACKAGE).debug(boost::format("registrar '%1%' id=%2% specified; updating registrar...")
      % _registrar.handle % _registrar.id);
    Database::Filters::Union uf;
    Database::Filters::Registrar *filter = new Database::Filters::RegistrarImpl();
    filter->addId().setValue(Database::ID(_registrar.id));
    uf.addFilter(filter);

    tmp_registrar_list->reload(uf);

    if (tmp_registrar_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    update_registrar = tmp_registrar_list->get(0);
  }
  update_registrar->setIco((const char *)_registrar.ico);
  update_registrar->setDic((const char *)_registrar.dic);
  update_registrar->setVarSymb((const char *)_registrar.varSymb);
  update_registrar->setVat((bool)_registrar.vat);
  update_registrar->setHandle((const char *)_registrar.handle);
  update_registrar->setURL((const char *)_registrar.url);
  update_registrar->setName((const char *)_registrar.name);
  update_registrar->setOrganization((const char *)_registrar.organization);
  update_registrar->setStreet1((const char *)_registrar.street1);
  update_registrar->setStreet2((const char *)_registrar.street2);
  update_registrar->setStreet3((const char *)_registrar.street3);
  update_registrar->setCity((const char *)_registrar.city);
  update_registrar->setProvince((const char *)_registrar.stateorprovince);
  update_registrar->setPostalCode((const char *)_registrar.postalcode);
  update_registrar->setCountry((const char *)_registrar.country);
  update_registrar->setTelephone((const char *)_registrar.telephone);
  update_registrar->setFax((const char *)_registrar.fax);
  update_registrar->setEmail((const char *)_registrar.email);
  update_registrar->setSystem((bool)_registrar.hidden);


  update_registrar->clearACLList();
  for (unsigned i = 0; i < _registrar.access.length(); i++)
  {
    LibFred::Registrar::ACL *registrar_acl = update_registrar->newACL();

    LOGGER(PACKAGE).debug(boost::format
            ("ccReg_Session_i::updateRegistrar : i: %1% setRegistrarId: %2%")
                % i % update_registrar->getId());

    const unsigned long long acl_id = _registrar.access[i].id;
    const std::string password = std::string(_registrar.access[i].password);
    const std::string md5cert = std::string(_registrar.access[i].md5Cert);

    registrar_acl->setCertificateMD5(md5cert);
    if (!password.empty())
    {
        registrar_acl->set_password(password);
    }

    const bool is_new_registrar = (_registrar.id == 0);
    if (!is_new_registrar)
    {
        const std::string md5_cert2 = std::string(_registrar.access[i].md5Cert2SamePasswd);
        registrar_acl->setRegistrarId(update_registrar->getId());

        const bool is_new_acl_record = (acl_id == 0);
        if (!is_new_acl_record)
        {
            registrar_acl->setId(acl_id);

            // add second record with same password
            if (!md5_cert2.empty())
            {
                LibFred::Registrar::ACL *registrar_acl_same_password = update_registrar->newACL();
                registrar_acl_same_password->setRegistrarId(update_registrar->getId());
                registrar_acl_same_password->setCertificateMD5(md5_cert2);
                registrar_acl_same_password->set_password_same_as_acl_id(acl_id);
            }
        }
        else
        {
            if (password.empty())
            {
                throw ccReg::Admin::UpdateFailed();
            }
            // add second record with same password but - we don't have id of acl record yet
            // so we use same password value (due to password hashing, records in database will
            // be different)
            else if (!md5_cert2.empty())
            {
                LibFred::Registrar::ACL *registrar_acl_same_password = update_registrar->newACL();
                registrar_acl_same_password->setRegistrarId(update_registrar->getId());
                registrar_acl_same_password->setCertificateMD5(md5_cert2);
                registrar_acl_same_password->set_password(password);
            }
        }
    }
    else
    {
        const bool new_registrar_has_mandatory_data = !password.empty() && !md5cert.empty();
        if (!new_registrar_has_mandatory_data)
        {
            throw ccReg::Admin::UpdateFailed();
        }
    }

  }

  update_registrar->clearZoneAccessList();
  for (unsigned i = 0; i < _registrar.zones.length();i++)
    {
      LibFred::Registrar::ZoneAccess *registrar_azone = update_registrar->newZoneAccess();

      LOGGER(PACKAGE).debug(boost::format
              ("ccReg_Session_i::updateRegistrar azone : i: %1% "
              "id: %2% name: %3% "
              "fromdate: %4% - %5% - %6% "
              "todate %7% - %8% - %9% "
              )
                  % i
                  % _registrar.zones[i].id
                  % _registrar.zones[i].name //3

                  % _registrar.zones[i].fromDate.year
                  % _registrar.zones[i].fromDate.month
                  % _registrar.zones[i].fromDate.day

                  % _registrar.zones[i].toDate.year //7
                  % _registrar.zones[i].toDate.month
                  % _registrar.zones[i].toDate.day

      );

      registrar_azone->id = _registrar.zones[i].id;
      registrar_azone->name = _registrar.zones[i].name;
      try
      {
          date fdate = makeBoostDate( _registrar.zones[i].fromDate);
          registrar_azone->fromdate = fdate;
      }
      catch(...)
      {//no date is NOT ok, webadmin should check this
          LOGGER(PACKAGE).error(boost::format
                  ("ccReg_Session_i::updateRegistrar Invalid fromDate "
                  "in azone: i: %1% "
                  "id: %2% name: %3% "
                  "fromdate: %4% - %5% - %6% "
                  "todate %7% - %8% - %9% "
                  )
                      % i
                      % _registrar.zones[i].id
                      % _registrar.zones[i].name //3

                      % _registrar.zones[i].fromDate.year
                      % _registrar.zones[i].fromDate.month
                      % _registrar.zones[i].fromDate.day

                      % _registrar.zones[i].toDate.year //7
                      % _registrar.zones[i].toDate.month
                      % _registrar.zones[i].toDate.day
          );

          throw;
      }//catch all fromdate

      try
      {
          date tdate = makeBoostDate( _registrar.zones[i].toDate);
          registrar_azone->todate = tdate;
      }
      catch(...){}//no date is ok

    }//for i

  try {
    update_registrar->save();
  } catch (...) {
    throw ccReg::Admin::UpdateFailed();
  }

  ccReg::TID rid = update_registrar->getId();

  LOGGER(PACKAGE).debug(boost::format("registrar with id=%1% saved")
      % rid);

  return rid;

}

Registry::PublicRequest::Detail* ccReg_Session_i::createPublicRequestDetail(LibFred::PublicRequest::PublicRequest* _request) {
  Registry::PublicRequest::Detail *detail = new Registry::PublicRequest::Detail();

  detail->id = _request->getId();

  switch (_request->getStatus()) {
    case LibFred::PublicRequest::PRS_OPENED:
      detail->status = Registry::PublicRequest::PRS_OPENED;
      break;
    case LibFred::PublicRequest::PRS_RESOLVED:
      detail->status = Registry::PublicRequest::PRS_RESOLVED;
      break;
    case LibFred::PublicRequest::PRS_INVALIDATED:
      detail->status = Registry::PublicRequest::PRS_INVALIDATED;
      break;
  }

  if (_request->getType() == LibFred::PublicRequest::PRT_AUTHINFO_AUTO_RIF) {
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_AUTO_RIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_AUTHINFO_AUTO_PIF) {
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_AUTO_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_AUTHINFO_EMAIL_PIF) {
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_EMAIL_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_AUTHINFO_POST_PIF) {
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_POST_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_BLOCK_CHANGES_EMAIL_PIF) {
      detail->type = Registry::PublicRequest::PRT_BLOCK_CHANGES_EMAIL_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_BLOCK_CHANGES_POST_PIF) {
      detail->type = Registry::PublicRequest::PRT_BLOCK_CHANGES_POST_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_BLOCK_TRANSFER_EMAIL_PIF) {
      detail->type = Registry::PublicRequest::PRT_BLOCK_TRANSFER_EMAIL_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_BLOCK_TRANSFER_POST_PIF) {
      detail->type = Registry::PublicRequest::PRT_BLOCK_TRANSFER_POST_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_UNBLOCK_CHANGES_EMAIL_PIF) {
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_CHANGES_EMAIL_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_UNBLOCK_CHANGES_POST_PIF) {
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_CHANGES_POST_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_UNBLOCK_TRANSFER_EMAIL_PIF) {
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_TRANSFER_EMAIL_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_UNBLOCK_TRANSFER_POST_PIF) {
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_TRANSFER_POST_PIF;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_CONTACT_IDENTIFICATION) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_CONTACT_VALIDATION) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION;
  }
  else if (_request->getType() == Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION ) {
      detail->type = Registry::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION ;
  }
  else if (_request->getType() == Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION) {
      detail->type = Registry::PublicRequest::PRT_CONTACT_IDENTIFICATION;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_CONTACT_REIDENTIFICATION) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_CONTACT_PREVALIDATED_UNIDENTIFIED_TRANSFER) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_PREVALIDATED_UNIDENTIFIED_CONTACT_TRANSFER;
  }
  else if (_request->getType() == CorbaConversion::Admin::PRT_MOJEID_CONTACT_PREVALIDATED_TRANSFER) {
      detail->type = Registry::PublicRequest::PRT_MOJEID_PREVALIDATED_CONTACT_TRANSFER;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_PERSONALINFO_AUTO_PIF) {
      detail->type = Registry::PublicRequest::PRT_PERSONALINFO_AUTO_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_PERSONALINFO_EMAIL_PIF) {
      detail->type = Registry::PublicRequest::PRT_PERSONALINFO_EMAIL_PIF;
  }
  else if (_request->getType() == LibFred::PublicRequest::PRT_PERSONALINFO_POST_PIF) {
      detail->type = Registry::PublicRequest::PRT_PERSONALINFO_POST_PIF;
  }
  else {
      throw std::runtime_error("unknown public request type");
  }

  detail->createTime = DUPSTRDATE(_request->getCreateTime);
  detail->resolveTime = DUPSTRDATE(_request->getResolveTime);
  detail->reason = _request->getReason().c_str();
  detail->email = _request->getEmailToAnswer().c_str();

  detail->answerEmail.id     = _request->getAnswerEmailId();
  detail->answerEmail.handle = DUPSTRC(stringify(_request->getAnswerEmailId()));
  detail->answerEmail.type   = ccReg::FT_MAIL;

  detail->registrar.id     = _request->getRegistrarId();
  detail->registrar.handle = DUPSTRFUN(_request->getRegistrarHandle);
  detail->registrar.type   = ccReg::FT_REGISTRAR;

  unsigned objects_size = _request->getObjectSize();
  detail->objects.length(objects_size);
  for (unsigned i = 0; i < objects_size; ++i) {
    LibFred::PublicRequest::OID oid = _request->getObject(0);
    detail->objects[i].id = oid.id;
    detail->objects[i].handle = oid.handle.c_str();
    switch (oid.type) {
      case LibFred::PublicRequest::OT_DOMAIN:
        detail->objects[i].type = ccReg::FT_DOMAIN;
        break;
      case LibFred::PublicRequest::OT_CONTACT:
        detail->objects[i].type = ccReg::FT_CONTACT;
        break;
      case LibFred::PublicRequest::OT_NSSET:
        detail->objects[i].type = ccReg::FT_NSSET;
        break;
      case LibFred::PublicRequest::OT_KEYSET:
        detail->objects[i].type = ccReg::FT_KEYSET;
        break;

      default:
        LOGGER(PACKAGE).error("Not allowed object type for PublicRequest detail!");
        break;
    }
  }

  return detail;
}


Registry::Invoicing::Detail* ccReg_Session_i::createInvoiceDetail(LibFred::Invoicing::Invoice *_invoice) {
  Registry::Invoicing::Detail *detail = new Registry::Invoicing::Detail();

  detail->id = _invoice->getId();
  detail->zone = _invoice->getZoneId();
  detail->createTime = DUPSTRDATE_NOLTCONVERT(_invoice->getCrDate);
  detail->taxDate = DUPSTRDATED(_invoice->getTaxDate);
  detail->fromDate = DUPSTRDATED(_invoice->getAccountPeriod().begin);
  detail->toDate = DUPSTRDATED(_invoice->getAccountPeriod().end);
  detail->type = (_invoice->getType() == LibFred::Invoicing::IT_DEPOSIT ? Registry::Invoicing::IT_ADVANCE
                                                                         : Registry::Invoicing::IT_ACCOUNT);
  detail->number = DUPSTRC(stringify(_invoice->getPrefix()));
  detail->credit = DUPSTRC(formatMoney(_invoice->getCredit()));
  detail->price = DUPSTRC(formatMoney(_invoice->getPrice()));
  detail->vatRate = DUPSTRC(_invoice->getVat().get_string());
  detail->total = DUPSTRC(formatMoney(_invoice->getTotal()));
  detail->totalVAT = DUPSTRC(formatMoney(_invoice->getTotalVat()));
  detail->varSymbol = DUPSTRC(_invoice->getVarSymbol());

  detail->registrar.id     = _invoice->getRegistrarId();
  detail->registrar.handle = DUPSTRC(_invoice->getClient()->getHandle());
  detail->registrar.type   = ccReg::FT_REGISTRAR;

  detail->filePDF.id     = _invoice->getFileId();
  // detail->filePDF.handle = _invoice->getFileNamePDF().c_str();

  detail->filePDF.handle = DUPSTRC(_invoice->getFileHandle());
  detail->filePDF.type   = ccReg::FT_FILE;

  detail->fileXML.id     = _invoice->getFileXmlId();
  detail->fileXML.handle = DUPSTRC(_invoice->getFileXmlHandle());

  detail->fileXML.type   = ccReg::FT_FILE;

  detail->payments.length(_invoice->getSourceCount());
  for (unsigned n = 0; n < _invoice->getSourceCount(); ++n) {
    const LibFred::Invoicing::PaymentSource *ps = _invoice->getSource(n);
    detail->payments[n].id = ps->getId();
    detail->payments[n].price = DUPSTRC(formatMoney(ps->getPrice()));
    detail->payments[n].balance = DUPSTRC(formatMoney(ps->getCredit()));
    detail->payments[n].number = DUPSTRC(stringify(ps->getNumber()));
  }

  detail->paymentActions.length(_invoice->getActionCount());
  for (unsigned n = 0; n < _invoice->getActionCount(); ++n) {
    const LibFred::Invoicing::PaymentAction *pa = _invoice->getAction(n);
    detail->paymentActions[n].paidObject.id     = pa->getObjectId();
    detail->paymentActions[n].paidObject.handle = DUPSTRFUN(pa->getObjectName);
    detail->paymentActions[n].paidObject.type   = ccReg::FT_DOMAIN;

    detail->paymentActions[n].actionTime = DUPSTRDATE_NOLTCONVERT(pa->getActionTime);
    detail->paymentActions[n].expirationDate = DUPSTRDATED(pa->getExDate);
    detail->paymentActions[n].actionType = pa->getAction();
    detail->paymentActions[n].unitsCount = pa->getUnitsCount();
    detail->paymentActions[n].pricePerUnit = DUPSTRC(formatMoney(pa->getPricePerUnit()));
    detail->paymentActions[n].price = DUPSTRC(formatMoney(pa->getPrice()));
  }

  return detail;

}

Registry::Mailing::Detail* ccReg_Session_i::createMailDetail(LibFred::Mail::Mail *_mail) {
  Registry::Mailing::Detail *detail = new Registry::Mailing::Detail();

  detail->id = _mail->getId();
  detail->type = _mail->getType();
  detail->status = _mail->getStatus();
  detail->createTime = DUPSTRDATE(_mail->getCreateTime);
  detail->modifyTime = DUPSTRDATE(_mail->getModTime);
  detail->content = DUPSTRC(_mail->getContent());

  detail->objects.length(_mail->getHandleSize());
  for (unsigned i = 0; i < _mail->getHandleSize(); ++i) {
    /* TODO: don't know object id and type - add support for this to database */
    // detail->objects[i].id     = 0;
    // detail->objects[i].handle = DUPSTRC(_mail->getHandle(i));
    // detail->objects[i].type   = ccReg::FT_OBJ;
    detail->objects[i] = DUPSTRC(_mail->getHandle(i));
  }

  detail->attachments.length(_mail->getAttachmentSize());
  for (unsigned i = 0; i < _mail->getAttachmentSize(); ++i) {
    const LibFred::OID attachment = _mail->getAttachment(i);

    detail->attachments[i].id     = attachment.id;
    detail->attachments[i].handle = attachment.handle.c_str();
    detail->attachments[i].type   = ccReg::FT_FILE;
  }

  return detail;
}

Registry::Zone::Detail* ccReg_Session_i::createZoneDetail(LibFred::Zone::Zone* _zone)
{
  TRACE("[CALL] ccReg_Session_i::createZoneDetail()");

  if (_zone == 0)
  {
      LOGGER(PACKAGE).error("_zone is null");
      throw ccReg::Admin::OBJECT_NOT_FOUND();
  }

  LOGGER(PACKAGE).debug(boost::format("generating zone detail for object id=%1%")
      % _zone->getId());
  Registry::Zone::Detail *detail = new Registry::Zone::Detail();

  detail->id = _zone->getId();
  detail->fqdn = DUPSTRFUN(_zone->getFqdn);
  detail->ex_period_min = _zone->getExPeriodMin();
  detail->ex_period_max = _zone->getExPeriodMax();
  detail->ttl = _zone->getTtl();
  detail->hostmaster = DUPSTRFUN(_zone->getHostmaster);
  detail->refresh = _zone->getRefresh();
  detail->update_retr = _zone->getUpdateRetr();
  detail->expiry = _zone->getExpiry();
  detail->minimum = _zone->getMinimum();
  detail->ns_fqdn = DUPSTRFUN(_zone->getNsFqdn);

  detail->ns.length(_zone->getZoneNsSize());
  for (unsigned i = 0; i < _zone->getZoneNsSize(); i++)
  {
      detail->ns[i].id = _zone->getZoneNs(i)->getId();
      detail->ns[i].fqdn = DUPSTRFUN(_zone->getZoneNs(i)->getFqdn);
      detail->ns[i].addr = DUPSTRFUN(_zone->getZoneNs(i)->getAddrs);
  }

  return detail;
}

Registry::Message::Detail* ccReg_Session_i::createMessageDetail(LibFred::Messages::MessagePtr _message) {
  TRACE("[CALL] ccReg_Session_i::createMessageDetail()");
  LOGGER(PACKAGE).debug(boost::format("generating message detail for object id=%1%")
      % _message->get_id());
  Registry::Message::Detail *detail = new Registry::Message::Detail();

  LOGGER(PACKAGE).debug(boost::format(
          "ccReg_Session_i::createMessageDetail "
          " id: %1% "
          " crdate: %2% "
          " moddate: %3% "
          " attempt: %4% "
          " status_name: %5% "
          " message_type: %6% "
          " comm_type: %7% "

          )
                % _message->conv_get(LibFred::Messages::MessageMetaInfo::MT_ID)
                % _message->conv_get(LibFred::Messages::MessageMetaInfo::MT_CRDATE)
                % _message->conv_get(LibFred::Messages::MessageMetaInfo::MT_MODDATE)
                % _message->conv_get(LibFred::Messages::MessageMetaInfo::MT_ATTEMPT)
                % _message->conv_get(LibFred::Messages::MessageMetaInfo::MT_STATUS)
                % _message->conv_get(LibFred::Messages::MessageMetaInfo::MT_MSGTYPE)
                % _message->conv_get(LibFred::Messages::MessageMetaInfo::MT_COMMTYPE)
                );


  detail->id = _message->get_id();

  //date time conversion from iso string

  detail->createDate = CORBA::string_dup((_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_CRDATE)).c_str());
  detail->modifyDate = CORBA::string_dup((_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_MODDATE)).c_str());

  detail->attempt = boost::lexical_cast<long>(_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_ATTEMPT));

  LibFred::Messages::ManagerPtr msg_mgr
      = m_registry_manager->getMessageManager();

  //enumlists
  //status names
  LibFred::Messages::EnumList status_names_ = msg_mgr->getStatusList();
  std::map<std::string, std::size_t > status_id;
  for (LibFred::Messages::EnumList::const_iterator i = status_names_.begin()
          ; i != status_names_.end(); ++i)
      status_id[i->name] = i->id;

  //communication types
  LibFred::Messages::EnumList comm_types_ = msg_mgr->getCommTypeList();
  std::map< std::string, std::size_t> comm_type_id;
  for (LibFred::Messages::EnumList::const_iterator i = comm_types_.begin()
          ; i != comm_types_.end(); ++i)
      comm_type_id[i->name] = i->id;

  //message types
  LibFred::Messages::EnumList msg_types_ = msg_mgr->getMessageTypeList();
  std::map<std::string, std::size_t > msg_type_id;
  for (LibFred::Messages::EnumList::const_iterator i = msg_types_.begin()
          ; i != msg_types_.end(); ++i)
      msg_type_id[i->name] = i->id;

  detail->status_id = status_id[_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_STATUS)];
  detail->comm_type_id = comm_type_id[_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_COMMTYPE)];
  detail->message_type_id = msg_type_id[_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_MSGTYPE)];

  //message types
    if((_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_COMMTYPE)).compare("sms") == 0)
    {

        //detail->message_content._d(1);//sms
        Registry::Message::SMSDetail sms_detail;

        //load from db
        LibFred::Messages::SmsInfo si
            = msg_mgr->get_sms_info_by_id(_message->get_id());

        sms_detail.content = CORBA::string_dup(si.content.c_str());
        sms_detail.phone_number = CORBA::string_dup(si.phone_number.c_str());

        detail->message_content.sms(sms_detail);

        LOGGER(PACKAGE).debug(boost::format(
                "ccReg_Session_i::createMessageDetail sms content "
                " _d(): %1% "
                " content: %2% "
                " phone_number: %3% "
                )
                      % detail->message_content._d()
                      % detail->message_content.sms().content
                      % detail->message_content.sms().phone_number
              );

    }//if sms
    else
    if(((_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_COMMTYPE))
            .compare("letter") == 0)
        || ((_message->conv_get(LibFred::Messages::MessageMetaInfo::MT_COMMTYPE))
                .compare("registered_letter") == 0))
    {
        //detail->message_content._d(2);//letter
        Registry::Message::LetterDetail letter_detail;

        //load from db

        LibFred::Messages::LetterInfo li
            = msg_mgr->get_letter_info_by_id(_message->get_id());


        letter_detail.file.id = li.file_id;
        letter_detail.file.handle = CORBA::string_dup(li.fname.c_str());//filename
        letter_detail.file.type = ccReg::FT_FILE;

        letter_detail.batch_id = CORBA::string_dup(li.batch_id.c_str());
        letter_detail.postal_address_name = CORBA::string_dup(li.postal_address.name.c_str());
        letter_detail.postal_address_organization = CORBA::string_dup(li.postal_address.org.c_str());
        letter_detail.postal_address_street1 = CORBA::string_dup(li.postal_address.street1.c_str());
        letter_detail.postal_address_street2 = CORBA::string_dup(li.postal_address.street2.c_str());
        letter_detail.postal_address_street3 = CORBA::string_dup(li.postal_address.street3.c_str());
        letter_detail.postal_address_city = CORBA::string_dup(li.postal_address.city.c_str());
        letter_detail.postal_address_stateorprovince = CORBA::string_dup(li.postal_address.state.c_str());
        letter_detail.postal_address_postalcode = CORBA::string_dup(li.postal_address.code.c_str());
        letter_detail.postal_address_country = CORBA::string_dup(li.postal_address.country.c_str());

        detail->message_content.letter(letter_detail);

        LOGGER(PACKAGE).debug(boost::format(
                "ccReg_Session_i::createMessageDetail letter content "
                " _d(): %1% "
                " file.id: %2% "
                " file.handle: %3% "
                " file.type: %4% "
                " batch_id: %5% "
                " postal_address_name: %6% "
                " postal_address_organization: %7% "
                " postal_address_street1: %8% "
                " postal_address_street2: %9% "
                " postal_address_street3: %10% "
                " postal_address_city: %11% "
                " postal_address_stateorprovince: %12% "
                " postal_address_postalcode: %13% "
                " postal_address_country: %14% "
                )
              % detail->message_content._d()
              % detail->message_content.letter().file.id
              % detail->message_content.letter().file.handle
              % detail->message_content.letter().file.type
              % detail->message_content.letter().batch_id
              % detail->message_content.letter().postal_address_name
              % detail->message_content.letter().postal_address_organization
              % detail->message_content.letter().postal_address_street1
              % detail->message_content.letter().postal_address_street2
              % detail->message_content.letter().postal_address_street3
              % detail->message_content.letter().postal_address_city
              % detail->message_content.letter().postal_address_stateorprovince
              % detail->message_content.letter().postal_address_postalcode
              % detail->message_content.letter().postal_address_country
              );

    }//if letter
    else
    {
        throw std::runtime_error("ccReg_Session_i::createMessageDetail message_content");
    }//union set error
    LOGGER(PACKAGE).debug(boost::format("ccReg_Session_i::createMessageDetail return detail for object id=%1%")
          % _message->get_id());
  return detail;
}



void ccReg_Session_i::setHistory(CORBA::Boolean _flag) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  if (_flag) {
    settings_.set("filter.history", "on");
  }
  else {
    settings_.set("filter.history", "off");
  }
}

