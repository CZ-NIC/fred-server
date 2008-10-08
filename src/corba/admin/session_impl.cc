#include <math.h>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <corba/ccReg.hh>

#include "session_impl.h"
#include "old_utils/log.h"
#include "old_utils/dbsql.h"
#include "register/register.h"
#include "register/notify.h"

#include "log/logger.h"
#include "log/context.h"
#include "util.h"

ccReg_Session_i::ccReg_Session_i(const std::string& _session_id,
                                 const std::string& database,
                                 NameService *ns,
                                 Config::Conf& cfg,
                                 ccReg_User_i* _user) :
  session_id_(_session_id), cfg_(cfg), m_user(_user), m_db_manager(database), m_mailer_manager(ns), m_last_activity(second_clock::local_time()) {

  base_context_ = Logging::Context::get() + "/" + session_id_;
  Logging::Context ctx(session_id_);

  db.OpenDatabase(database.c_str());
  m_register_manager.reset(Register::Manager::create(&db,
                                                     cfg.get<bool>("registry.restricted_handles")));
  m_register_manager->dbManagerInit(&m_db_manager);
  m_register_manager->initStates();

  m_document_manager.reset(Register::Document::Manager::create(cfg.get<std::string>("registry.docgen_path"),
                                                               cfg.get<std::string>("registry.docgen_template_path"),
                                                               cfg.get<std::string>("registry.fileclient_path"),
                                                               ns->getHostName()));
  m_publicrequest_manager.reset(Register::PublicRequest::Manager::create(&m_db_manager,
                                                                         m_register_manager->getDomainManager(),
                                                                         m_register_manager->getContactManager(),
                                                                         m_register_manager->getNSSetManager(),
                                                                         m_register_manager->getKeySetManager(),
                                                                         &m_mailer_manager,
                                                                         m_document_manager.get()));
  m_invoicing_manager.reset(Register::Invoicing::Manager::create(&db,
                                                                 m_document_manager.get(),
                                                                 &m_mailer_manager));

  
  mail_manager_.reset(Register::Mail::Manager::create(&m_db_manager));
  file_manager_.reset(Register::File::Manager::create(&m_db_manager));

  m_domains = new ccReg_Domains_i(m_register_manager->getDomainManager()->createList(), &settings_);
  m_contacts = new ccReg_Contacts_i(m_register_manager->getContactManager()->createList(), &settings_);
  m_nssets = new ccReg_NSSets_i(m_register_manager->getNSSetManager()->createList(), &settings_);
  m_keysets = new ccReg_KeySets_i(m_register_manager->getKeySetManager()->createList(), &settings_);
  m_registrars = new ccReg_Registrars_i(m_register_manager->getRegistrarManager()->getList());
  m_eppactions = new ccReg_EPPActions_i(m_register_manager->getRegistrarManager()->getEPPActionList());
  m_invoices = new ccReg_Invoices_i(m_invoicing_manager->createList());
  m_filters = new ccReg_Filters_i(m_register_manager->getFilterManager()->getList());
  m_publicrequests = new ccReg_PublicRequests_i(m_publicrequest_manager->createList());
  m_mails = new ccReg_Mails_i(mail_manager_->createList(), ns);
  m_files = new ccReg_Files_i(file_manager_->createList());

  m_eppactions->setDB(&m_db_manager);
  m_registrars->setDB(&m_db_manager);
  m_contacts->setDB(&m_db_manager);
  m_domains->setDB(&m_db_manager);
  m_nssets->setDB(&m_db_manager);
  m_keysets->setDB(&m_db_manager);
  m_publicrequests->setDB(&m_db_manager);
  m_invoices->setDB(&m_db_manager);

  settings_.set("filter.history", "off");

  updateActivity();
}

ccReg_Session_i::~ccReg_Session_i() {
  Logging::Context ctx(session_id_);

  TRACE("[CALL] ccReg_Session_i::~ccReg_Session_i()");

  delete m_registrars;
  delete m_eppactions;
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

  db.Disconnect();
}

ccReg::User_ptr ccReg_Session_i::getUser() {
  Logging::Context ctx(base_context_);

  return m_user->_this();
}

Registry::PageTable_ptr ccReg_Session_i::getPageTable(ccReg::FilterType _type) {
  Logging::Context ctx(base_context_);

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
    case ccReg::FT_ACTION:
      return m_eppactions->_this();
    case ccReg::FT_INVOICE:
      return m_invoices->_this();
    case ccReg::FT_PUBLICREQUEST:
      return m_publicrequests->_this();
    case ccReg::FT_MAIL:
      return m_mails->_this();
    case ccReg::FT_FILE:
      return m_files->_this();
      break;
  }
  LOGGER(PACKAGE).debug(boost::format("[ERROR] ccReg_Session_i::getPageTable(%1%): unknown type specified")
      % _type);
  return Registry::PageTable::_nil();
}

CORBA::Any* ccReg_Session_i::getDetail(ccReg::FilterType _type, ccReg::TID _id) {
  Logging::Context ctx(base_context_);

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

    case ccReg::FT_ACTION:
      *result <<= getEppActionDetail(_id);
      break;
      
    case ccReg::FT_FILTER:
    case ccReg::FT_OBJ:
    case ccReg::FT_FILE:
      LOGGER(PACKAGE).error("Calling method with not implemented parameter!");
    default:
      throw ccReg::Admin::OBJECT_NOT_FOUND();
      break;
  }

  return result;
}

const std::string& ccReg_Session_i::getId() const {
  return session_id_;
}

void ccReg_Session_i::updateActivity() {
  ptime tmp = m_last_activity;
  m_last_activity = second_clock::local_time();
  LOGGER(PACKAGE).debug(boost::format("session activity update: %1% (was %2%)")
      % m_last_activity % tmp);
}

bool ccReg_Session_i::isTimeouted() const {
  ptime threshold = second_clock::local_time() - seconds(cfg_.get<unsigned>("adifd.session_timeout"));
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
  // Register::Domain::Domain *domain = m_domains->findId(_id);
  // if (domain) {
  //   return createDomainDetail(domain);
  // } 
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing domain filter for object id=%1%' detail")
        % _id);
    std::auto_ptr<Register::Domain::List>
        tmp_domain_list(m_register_manager->getDomainManager()->createList());

    Database::Filters::Union uf(&settings_);
    Database::Filters::Domain *filter = new Database::Filters::DomainHistoryImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_domain_list->reload(uf, &m_db_manager);
    unsigned filter_count = tmp_domain_list->getCount();
    if (filter_count > 0) {
      return createHistoryDomainDetail(tmp_domain_list.get());
      // return createDomainDetail(tmp_domain_list->getDomain(filter_count - 1));
    }
    else {
      throw ccReg::Admin::ObjectNotFound();
    }
  }
}

Registry::Contact::Detail* ccReg_Session_i::getContactDetail(ccReg::TID _id) {
  // Register::Contact::Contact *contact = m_contacts->findId(_id);
  // if (contact) {
  //   return createContactDetail(contact);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing contact filter for object id=%1%' detail")
        % _id);
    std::auto_ptr<Register::Contact::List>
        tmp_contact_list(m_register_manager->getContactManager()->createList());

    Database::Filters::Union uf(&settings_);
    Database::Filters::Contact *filter = new Database::Filters::ContactHistoryImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_contact_list->reload(uf, &m_db_manager);

    unsigned filter_count = tmp_contact_list->getCount();
    if (filter_count > 0) {
      return createHistoryContactDetail(tmp_contact_list.get());
      // return createContactDetail(tmp_domain_list->getContact(filter_count - 1));
    }
    else {
      throw ccReg::Admin::ObjectNotFound();
    }
  }
}

Registry::NSSet::Detail* ccReg_Session_i::getNSSetDetail(ccReg::TID _id) {
  // Register::NSSet::NSSet *nsset = m_nssets->findId(_id);
  // if (nsset) {
  //   return createNSSetDetail(nsset);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing nsset filter for object id=%1%' detail")
        % _id);
    std::auto_ptr<Register::NSSet::List>
        tmp_nsset_list(m_register_manager->getNSSetManager()->createList());

    Database::Filters::Union uf(&settings_);
    Database::Filters::NSSet *filter = new Database::Filters::NSSetHistoryImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_nsset_list->reload(uf, &m_db_manager);

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
    // Register::KeySet::KeySet *keyset = m_keysets->findId(_id);
    // if (keyset)
    //     return createKeySetDetail(keyset);
    // else {
    /* disable cache */
    if (0) {
    }
    else {
        LOGGER(PACKAGE).debug(boost::format("constructing keyset filter for object id=%1%' detail") % _id);

        std::auto_ptr <Register::KeySet::List>
            tmp_keyset_list(m_register_manager->getKeySetManager()->createList());

        Database::Filters::Union uf(&settings_);
        Database::Filters::KeySet *filter = new Database::Filters::KeySetHistoryImpl();
        filter->addId().setValue(Database::ID(_id));
        uf.addFilter(filter);

        tmp_keyset_list->reload(uf, &m_db_manager);

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
  // Register::Registrar::Registrar *registrar = m_registrars->findId(_id);
  // if (registrar) {
  //   return createRegistrarDetail(registrar);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing registrar filter for object id=%1%' detail")
        % _id);
    Register::Registrar::RegistrarList * tmp_registrar_list =
        m_register_manager->getRegistrarManager()->getList();

    Database::Filters::Union uf;
    Database::Filters::Registrar *filter = new Database::Filters::RegistrarImpl();
    filter->addId().setValue(Database::ID(_id));
    uf.addFilter(filter);

    tmp_registrar_list->reload(uf, &m_db_manager);

    if (tmp_registrar_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createRegistrarDetail(tmp_registrar_list->get(0));
  }
}

Registry::PublicRequest::Detail* ccReg_Session_i::getPublicRequestDetail(ccReg::TID _id) {
  // Register::PublicRequest::PublicRequest *request = m_publicrequests->findId(_id);
  // if (request) {
  //   return createPublicRequestDetail(request);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing public request filter for object id=%1%' detail")
        % _id);
    std::auto_ptr<Register::PublicRequest::List> tmp_request_list(m_publicrequest_manager->createList());

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
  // Register::Invoicing::Invoice *invoice = m_invoices->findId(_id);
  // if (invoice && invoice->getActionCount()) {
  //   return createInvoiceDetail(invoice);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing invoice filter for object id=%1%' detail")
        % _id);
    std::auto_ptr<Register::Invoicing::List> tmp_invoice_list(m_invoicing_manager->createList());

    Database::Filters::Union union_filter;
    Database::Filters::Invoice *filter = new Database::Filters::InvoiceImpl();
    filter->addId().setValue(Database::ID(_id));
    union_filter.addFilter(filter);

    tmp_invoice_list->reload(union_filter, &m_db_manager);

    if (tmp_invoice_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createInvoiceDetail(tmp_invoice_list->get(0));
  }
}

Registry::Mailing::Detail* ccReg_Session_i::getMailDetail(ccReg::TID _id) {
  // Register::Mail::Mail *mail = m_mails->findId(_id);
  // if (mail) {
  //   return createMailDetail(mail);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing mail filter for object id=%1%' detail")
        % _id);
    std::auto_ptr<Register::Mail::List> tmp_mail_list(mail_manager_->createList());

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

Registry::EPPAction::Detail* ccReg_Session_i::getEppActionDetail(ccReg::TID _id) {
  // Register::Registrar::EPPAction *action = m_eppactions->findId(_id);
  // if (action && !action->getEPPMessageIn().empty()) {
  //   return createEppActionDetail(action);
  // } else {
  /* disable cache */
  if (0) {
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("constructing eppaction filter for object id=%1%' detail")
        % _id);
    std::auto_ptr<Register::Registrar::EPPActionList> tmp_action_list(m_register_manager->getRegistrarManager()->createEPPActionList());

    Database::Filters::Union union_filter;
    Database::Filters::EppAction *filter = new Database::Filters::EppActionImpl();
    filter->addId().setValue(Database::ID(_id));
    union_filter.addFilter(filter);

    tmp_action_list->reload(union_filter, &m_db_manager);

    if (tmp_action_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createEppActionDetail(tmp_action_list->get(0));
  }
}

ccReg::DomainDetail* ccReg_Session_i::createDomainDetail(Register::Domain::Domain* _domain) {
  TRACE("[CALL] ccReg_Session_i::createDomainDetail()");
  LOGGER(PACKAGE).debug(boost::format("generating domain detail for object id=%1%")
      % _domain->getId());
  ccReg::DomainDetail *detail = new ccReg::DomainDetail;

  detail->id = _domain->getId();
  detail->fqdn = DUPSTRFUN(_domain->getFQDN);
  detail->roid = DUPSTRFUN(_domain->getROID);
  detail->registrarHandle = DUPSTRFUN(_domain->getRegistrarHandle);
  detail->transferDate = DUPSTRDATE(_domain->getTransferDate);
  detail->updateDate = DUPSTRDATE(_domain->getUpdateDate);
  detail->createDate = DUPSTRDATE(_domain->getCreateDate);
  detail->createRegistrarHandle = DUPSTRFUN(_domain->getCreateRegistrarHandle);
  detail->updateRegistrarHandle = DUPSTRFUN(_domain->getUpdateRegistrarHandle);
  detail->authInfo = DUPSTRFUN(_domain->getAuthPw);
  detail->registrantHandle = DUPSTRFUN(_domain->getRegistrantHandle);
  detail->expirationDate = DUPSTRDATED(_domain->getExpirationDate);
  detail->valExDate = DUPSTRDATED(_domain->getValExDate);
  detail->nssetHandle = DUPSTRFUN(_domain->getNSSetHandle);
  detail->keysetHandle = DUPSTRFUN(_domain->getKeySetHandle);
  detail->admins.length(_domain->getAdminCount(1));
  detail->temps.length(_domain->getAdminCount(2));

  std::vector<unsigned> status_list;
  for (unsigned i = 0; i < _domain->getStatusCount(); i++) {
    if (m_register_manager->getStatusDesc(_domain->getStatusByIdx(i)->getStatusId())->getExternal())
      status_list.push_back(_domain->getStatusByIdx(i)->getStatusId());
  }
  detail->statusList.length(status_list.size());
  for (unsigned i = 0; i < status_list.size(); i++)
    detail->statusList[i] = status_list[i];

  try {
    for (unsigned i = 0; i < _domain->getAdminCount(1); i++)
    detail->admins[i] = DUPSTRC(_domain->getAdminHandleByIdx(i,1));
    for (unsigned i = 0; i < _domain->getAdminCount(2); i++)
    detail->temps[i] = DUPSTRC(_domain->getAdminHandleByIdx(i,2));
  }
  catch (Register::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
  return detail;
}


Registry::Domain::Detail* ccReg_Session_i::createHistoryDomainDetail(Register::Domain::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryDomainDetail()");
  Registry::Domain::Detail *detail = new Registry::Domain::Detail();

  /* array for object external states already assigned */
  std::vector<Register::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    Register::Domain::Domain *act  = _list->getDomain(n);
    Register::Domain::Domain *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getDomain(n + 1));

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
      const Register::Status *tmp = act->getStatusByIdx(s);

      LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<Register::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
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
    MAP_HISTORY_OID(nsset, getNSSetId, getNSSetHandle, ccReg::FT_NSSET)
    MAP_HISTORY_OID(keyset, getKeySetId, getKeySetHandle, ccReg::FT_KEYSET)

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
    catch (Register::NOT_FOUND) {
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
    catch (Register::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("domain id=%1% detail lib -> CORBA: request for temp contact out of range 0..%2%")
                                           % act->getId() % act->getAdminCount(2));
    }
  }

  return detail;
}

ccReg::ContactDetail* ccReg_Session_i::createContactDetail(Register::Contact::Contact* _contact) {
  TRACE("[CALL] ccReg_Session_i::createContactDetail()");
  LOGGER(PACKAGE).debug(boost::format("generating contact detail for object id=%1%")
      % _contact->getId());
  ccReg::ContactDetail *detail = new ccReg::ContactDetail;

  detail->id = _contact->getId();
  detail->handle = DUPSTRFUN(_contact->getHandle);
  detail->roid = DUPSTRFUN(_contact->getROID);
  detail->registrarHandle = DUPSTRFUN(_contact->getRegistrarHandle);
  detail->transferDate = DUPSTRDATE(_contact->getTransferDate);
  detail->updateDate = DUPSTRDATE(_contact->getUpdateDate);
  detail->createDate = DUPSTRDATE(_contact->getCreateDate);
  detail->createRegistrarHandle = DUPSTRFUN(_contact->getCreateRegistrarHandle);
  detail->updateRegistrarHandle = DUPSTRFUN(_contact->getUpdateRegistrarHandle);
  detail->authInfo = DUPSTRFUN(_contact->getAuthPw);
  detail->name = DUPSTRFUN(_contact->getName);
  detail->organization = DUPSTRFUN(_contact->getOrganization);
  detail->street1 = DUPSTRFUN(_contact->getStreet1);
  detail->street2 = DUPSTRFUN(_contact->getStreet2);
  detail->street3 = DUPSTRFUN(_contact->getStreet3);
  detail->province = DUPSTRFUN(_contact->getProvince);
  detail->postalcode = DUPSTRFUN(_contact->getPostalCode);
  detail->city = DUPSTRFUN(_contact->getCity);
  detail->country = DUPSTRFUN(_contact->getCountry);
  detail->telephone = DUPSTRFUN(_contact->getTelephone);
  detail->fax = DUPSTRFUN(_contact->getFax);
  detail->email = DUPSTRFUN(_contact->getEmail);
  detail->notifyEmail = DUPSTRFUN(_contact->getNotifyEmail);
  detail->ssn = DUPSTRFUN(_contact->getSSN);
  detail->ssnType = DUPSTRFUN(_contact->getSSNType);
  detail->vat = DUPSTRFUN(_contact->getVAT);
  detail->discloseName = _contact->getDiscloseName();
  detail->discloseOrganization = _contact->getDiscloseOrganization();
  detail->discloseAddress = _contact->getDiscloseAddr();
  detail->discloseEmail = _contact->getDiscloseEmail();
  detail->discloseTelephone = _contact->getDiscloseTelephone();
  detail->discloseFax = _contact->getDiscloseFax();
  detail->discloseIdent = _contact->getDiscloseIdent();
  detail->discloseVat = _contact->getDiscloseVat();
  detail->discloseNotifyEmail = _contact->getDiscloseNotifyEmail();

  std::vector<unsigned> status_list;
  for (unsigned i = 0; i < _contact->getStatusCount(); i++) {
    if (m_register_manager->getStatusDesc(
        _contact->getStatusByIdx(i)->getStatusId())->getExternal()) {
      status_list.push_back(_contact->getStatusByIdx(i)->getStatusId());
    }
  }
  detail->statusList.length(status_list.size());
  for (unsigned i = 0; i < status_list.size(); i++)
    detail->statusList[i] = status_list[i];

  return detail;
}

Registry::Contact::Detail* ccReg_Session_i::createHistoryContactDetail(Register::Contact::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryContactDetail()");
  Registry::Contact::Detail *detail = new Registry::Contact::Detail();
  
  /* array for object external states already assigned */
  std::vector<Register::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    Register::Contact::Contact *act  = _list->getContact(n);
    Register::Contact::Contact *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getContact(n + 1));

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
      const Register::Status *tmp = act->getStatusByIdx(s);

      LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<Register::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
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

  }

  return detail;
}


ccReg::NSSetDetail* ccReg_Session_i::createNSSetDetail(Register::NSSet::NSSet* _nsset) {
  TRACE("[CALL] ccReg_Session_i::createNSSetDetail()");
  LOGGER(PACKAGE).debug(boost::format("generating nsset detail for object id=%1%")
      % _nsset->getId());
  ccReg::NSSetDetail *detail = new ccReg::NSSetDetail;

  detail->id = _nsset->getId();
  detail->handle = DUPSTRFUN(_nsset->getHandle);
  detail->roid = DUPSTRFUN(_nsset->getROID);
  detail->registrarHandle = DUPSTRFUN(_nsset->getRegistrarHandle);
  detail->transferDate = DUPSTRDATE(_nsset->getTransferDate);
  detail->updateDate = DUPSTRDATE(_nsset->getUpdateDate);
  detail->createDate = DUPSTRDATE(_nsset->getCreateDate);
  detail->createRegistrarHandle = DUPSTRFUN(_nsset->getCreateRegistrarHandle);
  detail->updateRegistrarHandle = DUPSTRFUN(_nsset->getUpdateRegistrarHandle);
  detail->authInfo = DUPSTRFUN(_nsset->getAuthPw);
  detail->admins.length(_nsset->getAdminCount());

  try {
    for (unsigned i = 0; i < _nsset->getAdminCount(); i++)
    detail->admins[i] = DUPSTRC(_nsset->getAdminByIdx(i));
  }
  catch (Register::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }

  detail->hosts.length(_nsset->getHostCount());
  for (unsigned i = 0; i < _nsset->getHostCount(); i++) {
    detail->hosts[i].fqdn = DUPSTRFUN(_nsset->getHostByIdx(i)->getName);
    detail->hosts[i].inet.length(_nsset->getHostByIdx(i)->getAddrCount());
    for (unsigned j = 0; j < _nsset->getHostByIdx(i)->getAddrCount(); j++)
      detail->hosts[i].inet[j] = DUPSTRC(_nsset->getHostByIdx(i)->getAddrByIdx(j));
  }
  std::vector<unsigned> status_list;
  for (unsigned i = 0; i < _nsset->getStatusCount(); i++) {
    if (m_register_manager->getStatusDesc(
        _nsset->getStatusByIdx(i)->getStatusId()
    )->getExternal()) {
      status_list.push_back(_nsset->getStatusByIdx(i)->getStatusId());
    }
  }
  detail->statusList.length(status_list.size());
  for (unsigned i = 0; i < status_list.size(); i++)
    detail->statusList[i] = status_list[i];

  return detail;
}

Registry::NSSet::Detail* ccReg_Session_i::createHistoryNSSetDetail(Register::NSSet::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryNSSetDetail()");
  Registry::NSSet::Detail *detail = new Registry::NSSet::Detail();

  /* array for object external states already assigned */
  std::vector<Register::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    Register::NSSet::NSSet *act  = _list->getNSSet(n);
    Register::NSSet::NSSet *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getNSSet(n + 1));

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
      const Register::Status *tmp = act->getStatusByIdx(s);

      LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<Register::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
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
    }
    catch (Register::NOT_FOUND) {
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
    catch (Register::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("nsset id=%1% detail lib -> CORBA: request for host out of range 0..%2%")
                                           % act->getId() % act->getHostCount());
    }

  }

  return detail;
}

ccReg::KeySetDetail *
ccReg_Session_i::createKeySetDetail(Register::KeySet::KeySet *_keyset)
{
    TRACE("[CALL] ccReg_Session_i::createKeySetDetail()");
    LOGGER(PACKAGE).debug(boost::format(
                "generating keyset detail for object id=%1%")
            % _keyset->getId());
    ccReg::KeySetDetail *detail = new ccReg::KeySetDetail;

    detail->id = _keyset->getId();
    detail->handle = DUPSTRFUN(_keyset->getHandle);
    detail->roid = DUPSTRFUN(_keyset->getROID);
    detail->registrarHandle = DUPSTRFUN(_keyset->getRegistrarHandle);
    detail->transferDate = DUPSTRDATE(_keyset->getTransferDate);
    detail->updateDate = DUPSTRDATE(_keyset->getUpdateDate);
    detail->createDate = DUPSTRDATE(_keyset->getCreateDate);
    detail->createRegistrarHandle = DUPSTRFUN(_keyset->getCreateRegistrarHandle);
    detail->updateRegistrarHandle = DUPSTRFUN(_keyset->getUpdateRegistrarHandle);
    detail->authInfo = DUPSTRFUN(_keyset->getAuthPw);
    detail->admins.length(_keyset->getAdminCount());

    try {
        for (unsigned int i = 0; i < _keyset->getAdminCount(); i++)
            detail->admins[i] = DUPSTRC(_keyset->getAdminByIdx(i));
    }
    catch (Register::NOT_FOUND) {
        // TODO implement error handling
    }

    // TODO XXX have keyset detail dsrecords list?!?
    // detail->DSRecords.length(_keyset->getDSrecordCount());
    // for (unsigned int i = 0; i < _keyset->getDSrecordCount(); i++) {
    // }

    std::vector<unsigned int> status_list;
    for (unsigned int i = 0; i < _keyset->getStatusCount(); i++) {
        if (m_register_manager->getStatusDesc(
                    _keyset->getStatusByIdx(i)->getStatusId()
                    )->getExternal())
            status_list.push_back(
                    _keyset->getStatusByIdx(i)->getStatusId());
    }

    detail->statusList.length(status_list.size());
    for (unsigned int i = 0; i < status_list.size(); i++)
        detail->statusList[i] = status_list[i];

    return detail;
}

Registry::KeySet::Detail* ccReg_Session_i::createHistoryKeySetDetail(Register::KeySet::List* _list) {
  TRACE("[CALL] ccReg_Session_i::createHistoryKeySetDetail()");
  Registry::KeySet::Detail *detail = new Registry::KeySet::Detail();

  /* array for object external states already assigned */
  std::vector<Register::TID> ext_states_ids;

  /* we going backwards because at the end there are latest data */
  for (int n = _list->size() - 1; n >= 0; --n) {
    Register::KeySet::KeySet *act  = _list->getKeySet(n);
    Register::KeySet::KeySet *prev = ((unsigned)n == _list->size() - 1 ? act : _list->getKeySet(n + 1));

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
      const Register::Status *tmp = act->getStatusByIdx(s);

      LOGGER(PACKAGE).debug(boost::format("history detail -- (id=%1%) checking state %2%") % tmp->getId() % tmp->getStatusId());
      std::vector<Register::TID>::const_iterator it = find(ext_states_ids.begin(), ext_states_ids.end(), tmp->getId());
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
    catch (Register::NOT_FOUND) {
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
    catch (Register::NOT_FOUND) {
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
    catch (Register::NOT_FOUND) {
      LOGGER(PACKAGE).error(boost::format("keyset id=%1% detail lib -> CORBA: request for dnskey out of range 0..%2%")
                                           % act->getId() % act->getDNSKeyCount());
    }

  }

  return detail;
}

Registry::Registrar::Detail* ccReg_Session_i::createRegistrarDetail(Register::Registrar::Registrar* _registrar) {
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
  detail->credit = DUPSTRC(formatMoney(_registrar->getCredit()*100));

  detail->access.length(_registrar->getACLSize());
  for (unsigned i = 0; i < _registrar->getACLSize(); i++) {
    detail->access[i].md5Cert = DUPSTRFUN(_registrar->getACL(i)->getCertificateMD5);
    detail->access[i].password = DUPSTRFUN(_registrar->getACL(i)->getPassword);
  }
  detail->hidden = _registrar->getSystem();

  return detail;
}

void ccReg_Session_i::updateRegistrar(const ccReg::Registrar& _registrar) {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Session_i::updateRegistrar()");
  Register::Registrar::RegistrarList *tmp_registrar_list =
      m_register_manager->getRegistrarManager()->getList();
  Register::Registrar::Registrar *update_registrar; // registrar to be created or updated
  if (!_registrar.id) {
    LOGGER(PACKAGE).debug("no registrar id specified; creating new registrar...");
    update_registrar = tmp_registrar_list->create();
  }
  else {
    LOGGER(PACKAGE).debug(boost::format("registrar '%1%' id=%2% specified; updating registrar...") 
      % _registrar.handle % _registrar.id);
    Database::Filters::Union uf;
    Database::Filters::Registrar *filter = new Database::Filters::RegistrarImpl();
    filter->addId().setValue(Database::ID(_registrar.id));
    uf.addFilter(filter);

    tmp_registrar_list->reload(uf, &m_db_manager);

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
  for (unsigned i = 0; i < _registrar.access.length(); i++) {
    Register::Registrar::ACL *registrar_acl = update_registrar->newACL();
    registrar_acl->setCertificateMD5((const char *)_registrar.access[i].md5Cert);
    registrar_acl->setPassword((const char *)_registrar.access[i].password);
  }
  try {
    update_registrar->save();
  } catch (...) {
    throw ccReg::Admin::UpdateFailed();
  }
  LOGGER(PACKAGE).debug(boost::format("registrar with id=%1% saved") 
      % update_registrar->getId());
}

Registry::PublicRequest::Detail* ccReg_Session_i::createPublicRequestDetail(Register::PublicRequest::PublicRequest* _request) {
  Registry::PublicRequest::Detail *detail = new Registry::PublicRequest::Detail();
  
  detail->id = _request->getId();

  switch (_request->getStatus()) {
    case Register::PublicRequest::PRS_NEW:
      detail->status = Registry::PublicRequest::PRS_NEW;
      break;
    case Register::PublicRequest::PRS_ANSWERED:
      detail->status = Registry::PublicRequest::PRS_ANSWERED;
      break;
    case Register::PublicRequest::PRS_INVALID:
      detail->status = Registry::PublicRequest::PRS_INVALID;
      break;
  }
  
  switch (_request->getType()) {
    case Register::PublicRequest::PRT_AUTHINFO_AUTO_RIF:
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_AUTO_RIF;
      break;
    case Register::PublicRequest::PRT_AUTHINFO_AUTO_PIF:
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_AUTO_PIF;
      break;
    case Register::PublicRequest::PRT_AUTHINFO_EMAIL_PIF:
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_AUTHINFO_POST_PIF:
      detail->type = Registry::PublicRequest::PRT_AUTHINFO_POST_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_CHANGES_EMAIL_PIF:
      detail->type = Registry::PublicRequest::PRT_BLOCK_CHANGES_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_CHANGES_POST_PIF:
      detail->type = Registry::PublicRequest::PRT_BLOCK_CHANGES_POST_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_TRANSFER_EMAIL_PIF:
      detail->type = Registry::PublicRequest::PRT_BLOCK_TRANSFER_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_TRANSFER_POST_PIF:
      detail->type = Registry::PublicRequest::PRT_BLOCK_TRANSFER_POST_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_CHANGES_EMAIL_PIF:
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_CHANGES_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_CHANGES_POST_PIF:
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_CHANGES_POST_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_TRANSFER_EMAIL_PIF:
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_TRANSFER_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_TRANSFER_POST_PIF:
      detail->type = Registry::PublicRequest::PRT_UNBLOCK_TRANSFER_POST_PIF;
      break;
      
  }
  
  detail->createTime = DUPSTRDATE(_request->getCreateTime);
  detail->resolveTime = DUPSTRDATE(_request->getResolveTime);
  detail->reason = _request->getReason().c_str();
  detail->email = _request->getEmailToAnswer().c_str();

  detail->answerEmail.id     = _request->getAnswerEmailId();
  detail->answerEmail.handle = DUPSTRC(Conversion<long long unsigned>::to_string(_request->getAnswerEmailId())); 
  detail->answerEmail.type   = ccReg::FT_MAIL;

  detail->action.id     = _request->getEppActionId();
  detail->action.handle = DUPSTRFUN(_request->getSvTRID);
  detail->action.type   = ccReg::FT_ACTION;

  detail->registrar.id     = _request->getRegistrarId();
  detail->registrar.handle = DUPSTRFUN(_request->getRegistrarHandle);
  detail->registrar.type   = ccReg::FT_REGISTRAR;

  unsigned objects_size = _request->getObjectSize();
  detail->objects.length(objects_size);
  for (unsigned i = 0; i < objects_size; ++i) {  
    Register::PublicRequest::OID oid = _request->getObject(0);
    detail->objects[i].id = oid.id;
    detail->objects[i].handle = oid.handle.c_str();
    switch (oid.type) {
      case Register::PublicRequest::OT_DOMAIN:
        detail->objects[i].type = ccReg::FT_DOMAIN;
        break;
      case Register::PublicRequest::OT_CONTACT:
        detail->objects[i].type = ccReg::FT_CONTACT;
        break;
      case Register::PublicRequest::OT_NSSET:
        detail->objects[i].type = ccReg::FT_NSSET;
        break;
      case Register::PublicRequest::OT_KEYSET:
        detail->objects[i].type = ccReg::FT_KEYSET;
        break;

      default:
        LOGGER(PACKAGE).error("Not allowed object type for PublicRequest detail!");
        break;
    }
  }
  
  return detail;
}

Registry::Invoicing::Detail* ccReg_Session_i::createInvoiceDetail(Register::Invoicing::Invoice *_invoice) {
  Registry::Invoicing::Detail *detail = new Registry::Invoicing::Detail();
  
  detail->id = _invoice->getId();
  detail->zone = _invoice->getZone();
  detail->createTime = DUPSTRDATE(_invoice->getCrTime);
  detail->taxDate = DUPSTRDATED(_invoice->getTaxDate);
  detail->fromDate = DUPSTRDATED(_invoice->getAccountPeriod().begin);
  detail->toDate = DUPSTRDATED(_invoice->getAccountPeriod().end);
  detail->type = (_invoice->getType() == Register::Invoicing::IT_DEPOSIT ? Registry::Invoicing::IT_ADVANCE
                                                                         : Registry::Invoicing::IT_ACCOUNT);
  detail->number = DUPSTRC(Conversion<long long unsigned>::to_string(_invoice->getNumber()));
  detail->credit = DUPSTRC(formatMoney(_invoice->getCredit()));
  detail->price = DUPSTRC(formatMoney(_invoice->getPrice()));
  detail->vatRate = _invoice->getVatRate();
  detail->total = DUPSTRC(formatMoney(_invoice->getTotal()));
  detail->totalVAT = DUPSTRC(formatMoney(_invoice->getTotalVAT()));
  detail->varSymbol = DUPSTRC(_invoice->getVarSymbol());
  
  detail->registrar.id     = _invoice->getRegistrar();
  detail->registrar.handle = DUPSTRC(_invoice->getClient()->getHandle());
  detail->registrar.type   = ccReg::FT_REGISTRAR;

  detail->filePDF.id     = _invoice->getFilePDF();
  detail->filePDF.handle = _invoice->getFileNamePDF().c_str(); 
  detail->filePDF.type   = ccReg::FT_FILE;

  detail->fileXML.id     = _invoice->getFileXML();
  detail->fileXML.handle = _invoice->getFileNameXML().c_str();
  detail->fileXML.type   = ccReg::FT_FILE;

  detail->payments.length(_invoice->getSourceCount());
  for (unsigned n = 0; n < _invoice->getSourceCount(); ++n) {
    const Register::Invoicing::PaymentSource *ps = _invoice->getSource(n);
    detail->payments[n].id = ps->getId();
    detail->payments[n].price = DUPSTRC(formatMoney(ps->getPrice()));
    detail->payments[n].balance = DUPSTRC(formatMoney(ps->getCredit()));
    detail->payments[n].number = DUPSTRC(Conversion<long long unsigned>::to_string(ps->getNumber()));
  }
  
  detail->paymentActions.length(_invoice->getActionCount());
  for (unsigned n = 0; n < _invoice->getActionCount(); ++n) {
    const Register::Invoicing::PaymentAction *pa = _invoice->getAction(n);
    detail->paymentActions[n].paidObject.id     = pa->getObjectId();
    detail->paymentActions[n].paidObject.handle = DUPSTRFUN(pa->getObjectName);
    detail->paymentActions[n].paidObject.type   = ccReg::FT_DOMAIN;

    detail->paymentActions[n].actionTime = DUPSTRDATE(pa->getActionTime);
    detail->paymentActions[n].expirationDate = DUPSTRDATED(pa->getExDate);
    detail->paymentActions[n].actionType = pa->getAction();
    detail->paymentActions[n].unitsCount = pa->getUnitsCount();
    detail->paymentActions[n].pricePerUnit = DUPSTRC(formatMoney(pa->getPricePerUnit()));
    detail->paymentActions[n].price = DUPSTRC(formatMoney(pa->getPrice()));
  }
  
  return detail;
}

Registry::Mailing::Detail* ccReg_Session_i::createMailDetail(Register::Mail::Mail *_mail) {
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
    const Register::OID attachment = _mail->getAttachment(i);

    detail->attachments[i].id     = attachment.id;
    detail->attachments[i].handle = attachment.handle.c_str();
    detail->attachments[i].type   = ccReg::FT_FILE;
  }
     
  return detail;
}

Registry::EPPAction::Detail* ccReg_Session_i::createEppActionDetail(Register::Registrar::EPPAction *_action) {
  Registry::EPPAction::Detail *detail = new Registry::EPPAction::Detail();
  
  detail->id               = _action->getId();
  detail->xml              = DUPSTRFUN(_action->getEPPMessageIn);
  detail->xml_out          = DUPSTRFUN(_action->getEPPMessageOut);
  detail->time             = DUPSTRDATE(_action->getStartTime);
  detail->type             = DUPSTRFUN(_action->getTypeName);
  detail->objectHandle     = DUPSTRFUN(_action->getHandle);
  detail->result           = _action->getResult();
  detail->clTRID           = DUPSTRFUN(_action->getClientTransactionId);
  detail->svTRID           = DUPSTRFUN(_action->getServerTransactionId);
  
  detail->registrar.id     = _action->getRegistrarId();
  detail->registrar.handle = DUPSTRFUN(_action->getRegistrarHandle);
  detail->registrar.type   = ccReg::FT_REGISTRAR;
  
  return detail;
}

void ccReg_Session_i::setHistory(CORBA::Boolean _flag) {
  Logging::Context ctx(base_context_);

  if (_flag) {
    settings_.set("filter.history", "on");
  }
  else {
    settings_.set("filter.history", "off");
  }
}

