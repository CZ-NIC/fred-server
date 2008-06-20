#include <math.h>
#include <memory>
#include <iomanip>
#include <corba/ccReg.hh>

#include "session_impl.h"
#include "old_utils/log.h"
#include "old_utils/dbsql.h"
#include "register/register.h"
#include "register/notify.h"

#include "log/logger.h"
#include "util.h"

ccReg_Session_i::ccReg_Session_i(const std::string& database,
                                 NameService *ns,
                                 Conf& cfg,
                                 ccReg_User_i* _user) :
  m_user(_user), m_mailer_manager(ns) {
  
  db.OpenDatabase(database.c_str());
  m_db_manager.reset(new DBase::PSQLManager(cfg.GetDBconninfo()));

  m_register_manager.reset(Register::Manager::create(&db,
                                                     cfg.GetRestrictedHandles()));
  m_register_manager->dbManagerInit(m_db_manager.get());
  m_register_manager->initStates();

  m_document_manager.reset(Register::Document::Manager::create(cfg.GetDocGenPath(),
                                                               cfg.GetDocGenTemplatePath(),
                                                               cfg.GetFileClientPath(),
                                                               ns->getHostName()));
  m_publicrequest_manager.reset(Register::PublicRequest::Manager::create(m_db_manager.get(),
                                                                         m_register_manager->getDomainManager(),
                                                                         m_register_manager->getContactManager(),
                                                                         m_register_manager->getNSSetManager(),
                                                                         &m_mailer_manager,
                                                                         m_document_manager.get()));
  m_invoicing_manager.reset(Register::Invoicing::Manager::create(&db,
                                                                 m_document_manager.get(),
                                                                 &m_mailer_manager));
  
  mail_manager_.reset(Register::Mail::Manager::create(m_db_manager.get()));
  file_manager_.reset(Register::File::Manager::create(m_db_manager.get()));

  m_registrars = new ccReg_Registrars_i(m_register_manager->getRegistrarManager()->getList());
  m_eppactions = new ccReg_EPPActions_i(m_register_manager->getRegistrarManager()->getEPPActionList());
  m_domains = new ccReg_Domains_i(m_register_manager->getDomainManager()->createList());
  m_contacts = new ccReg_Contacts_i(m_register_manager->getContactManager()->createList());
  m_nssets = new ccReg_NSSets_i(m_register_manager->getNSSetManager()->createList());
  m_invoices = new ccReg_Invoices_i(m_invoicing_manager->createList());
  m_filters = new ccReg_Filters_i(m_register_manager->getFilterManager()->getList());
  m_publicrequests = new ccReg_PublicRequests_i(m_publicrequest_manager->createList());
  m_mails = new ccReg_Mails_i(mail_manager_->createList(), ns);
  m_files = new ccReg_Files_i(file_manager_->createList());

  // m_user = new ccReg_User_i(1, "superuser", "Pepa", "Zdepa");  

  m_eppactions->setDB(m_db_manager.get());
  m_registrars->setDB(m_db_manager.get());
  m_contacts->setDB(m_db_manager.get());
  m_domains->setDB(m_db_manager.get());
  m_nssets->setDB(m_db_manager.get());
  m_publicrequests->setDB(m_db_manager.get());
  m_invoices->setDB(m_db_manager.get());

  updateActivity();
}

ccReg_Session_i::~ccReg_Session_i() {
  TRACE("[CALL] ccReg_Session_i::~ccReg_Session_i()");

  delete m_registrars;
  delete m_eppactions;
  delete m_domains;
  delete m_contacts;
  delete m_nssets;
  delete m_publicrequests;
  delete m_mails;
  delete m_invoices;
  delete m_filters;
  delete m_user;
  delete m_files;

  db.Disconnect();
}

ccReg::User_ptr ccReg_Session_i::getUser() {
  return m_user->_this();
}

ccReg::PageTable_ptr ccReg_Session_i::getPageTable(ccReg::FilterType _type) {
  TRACE(boost::format("[CALL] ccReg_Session_i::getPageTable(%1%)") % _type);
  switch (_type) {
    case ccReg::FT_FILTER:
      return m_filters->_this();
    case ccReg::FT_REGISTRAR:
      return m_registrars->_this();
    case ccReg::FT_OBJ:
      return ccReg::PageTable::_nil();
    case ccReg::FT_CONTACT:
      return m_contacts->_this();
    case ccReg::FT_NSSET:
      return m_nssets->_this();
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
  LOGGER("corba").debug(boost::format("[ERROR] ccReg_Session_i::getPageTable(%1%): unknown type specified")
      % _type);
  return ccReg::PageTable::_nil();
}

CORBA::Any* ccReg_Session_i::getDetail(ccReg::FilterType _type, ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_Session_i::getDetail(%1%, %2%)") % _type
      % _id);
  CORBA::Any *result = new CORBA::Any;
  
  ccReg::ContactDetail *c_detail = 0;
  ccReg::NSSetDetail *n_detail = 0;
  ccReg::DomainDetail *d_detail = 0;
  ccReg::Registrar *r_detail = 0;
  ccReg::PublicRequest::Detail *pr_detail = 0;
  ccReg::Invoicing::Invoice *i_detail = 0;
  ccReg::Mailing::Detail *m_detail = 0;

  switch (_type) {
    case ccReg::FT_CONTACT:
      c_detail = getContactDetail(_id);
      *result <<= c_detail;
      break;

    case ccReg::FT_NSSET:
      n_detail = getNSSetDetail(_id);
      *result <<= n_detail;
      break;

    case ccReg::FT_DOMAIN:
      d_detail = getDomainDetail(_id);
      *result <<= d_detail;
      break;

    case ccReg::FT_REGISTRAR:
      r_detail = getRegistrarDetail(_id);
      *result <<= r_detail;
      break;
      
    case ccReg::FT_PUBLICREQUEST:
      pr_detail = getPublicRequestDetail(_id);
      *result <<= pr_detail;
      break;
      
    case ccReg::FT_INVOICE:
      i_detail = getInvoiceDetail(_id);
      *result <<= i_detail;
      break;
      
    case ccReg::FT_MAIL:
      m_detail = getMailDetail(_id);
      *result <<= m_detail;
      break;

    case ccReg::FT_FILTER:
    case ccReg::FT_OBJ:
    case ccReg::FT_ACTION:
    case ccReg::FT_FILE:
      LOGGER("corba").error("Calling method with not implemented parameter!");
    default:
      throw ccReg::Admin::OBJECT_NOT_FOUND();
      break;
  }

  return result;
}

void ccReg_Session_i::updateActivity() {
  ptime tmp = m_last_activity;
  m_last_activity = second_clock::local_time();
  LOGGER("corba").debug(boost::format("session activity update: %1% (was %2%)")
      % m_last_activity % tmp);
}

bool ccReg_Session_i::isTimeouted() const {
  /* TODO: Timeout value should be in configuration */
  ptime threshold = second_clock::local_time() - minutes(60);
  bool timeout = m_last_activity < threshold;
  LOGGER("corba").debug(boost::format("session last activity: %1% timeout threshold: %2% -- session %3%")
      % to_simple_string(m_last_activity) % to_simple_string(threshold)
      % (timeout ? "timeout" : "alive"));
  return timeout;
}

ccReg::DomainDetail* ccReg_Session_i::getDomainDetail(ccReg::TID _id) {
  Register::Domain::Domain *domain = m_domains->findId(_id);
  if (domain) {
    return createDomainDetail(domain);
  } else {
    LOGGER("corba").debug(boost::format("constructing domain filter for object id=%1%' detail")
        % _id);
    std::auto_ptr <Register::Domain::List>
        tmp_domain_list(m_register_manager->getDomainManager()->createList());

    DBase::Filters::Union uf;
    DBase::Filters::Domain *filter = new DBase::Filters::DomainHistoryImpl();
    filter->addId().setValue(DBase::ID(_id));
    uf.addFilter(filter);

    tmp_domain_list->reload2(uf, m_db_manager.get());

    if (tmp_domain_list->getCount() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createDomainDetail(tmp_domain_list->getDomain(0));
  }
}

ccReg::ContactDetail* ccReg_Session_i::getContactDetail(ccReg::TID _id) {
  Register::Contact::Contact *contact = m_contacts->findId(_id);
  if (contact) {
    return createContactDetail(contact);
  } else {
    LOGGER("corba").debug(boost::format("constructing contact filter for object id=%1%' detail")
        % _id);
    std::auto_ptr <Register::Contact::List>
        tmp_contact_list(m_register_manager->getContactManager()->createList());

    DBase::Filters::Union uf;
    DBase::Filters::Contact *filter = new DBase::Filters::ContactHistoryImpl();
    filter->addId().setValue(DBase::ID(_id));
    uf.addFilter(filter);

    tmp_contact_list->reload2(uf, m_db_manager.get());

    if (tmp_contact_list->getCount() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createContactDetail(tmp_contact_list->getContact(0));
  }
}

ccReg::NSSetDetail* ccReg_Session_i::getNSSetDetail(ccReg::TID _id) {
  Register::NSSet::NSSet *nsset = m_nssets->findId(_id);
  if (nsset) {
    return createNSSetDetail(nsset);
  } else {
    LOGGER("corba").debug(boost::format("constructing nsset filter for object id=%1%' detail")
        % _id);
    std::auto_ptr <Register::NSSet::List>
        tmp_nsset_list(m_register_manager->getNSSetManager()->createList());

    DBase::Filters::Union uf;
    DBase::Filters::NSSet *filter = new DBase::Filters::NSSetHistoryImpl();
    filter->addId().setValue(DBase::ID(_id));
    uf.addFilter(filter);

    tmp_nsset_list->reload2(uf, m_db_manager.get());

    if (tmp_nsset_list->getCount() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createNSSetDetail(tmp_nsset_list->getNSSet(0));
  }
}

ccReg::Registrar* ccReg_Session_i::getRegistrarDetail(ccReg::TID _id) {
  Register::Registrar::Registrar *registrar = m_registrars->findId(_id);
  if (registrar) {
    return createRegistrarDetail(registrar);
  } else {
    LOGGER("corba").debug(boost::format("constructing registrar filter for object id=%1%' detail")
        % _id);
    Register::Registrar::RegistrarList * tmp_registrar_list =
        m_register_manager->getRegistrarManager()->getList();

    DBase::Filters::Union uf;
    DBase::Filters::Registrar *filter = new DBase::Filters::RegistrarImpl();
    filter->addId().setValue(DBase::ID(_id));
    uf.addFilter(filter);

    tmp_registrar_list->reload2(uf, m_db_manager.get());

    if (tmp_registrar_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createRegistrarDetail(tmp_registrar_list->get(0));
  }
}

ccReg::PublicRequest::Detail* ccReg_Session_i::getPublicRequestDetail(ccReg::TID _id) {
  Register::PublicRequest::PublicRequest *request = m_publicrequests->findId(_id);
  if (request) {
    return createPublicRequestDetail(request);
  } else {
    LOGGER("corba").debug(boost::format("constructing public request filter for object id=%1%' detail")
        % _id);
    Register::PublicRequest::List *tmp_request_list = m_publicrequest_manager->createList();

    DBase::Filters::Union union_filter;
    DBase::Filters::PublicRequest *filter = new DBase::Filters::PublicRequestImpl();
    filter->addId().setValue(DBase::ID(_id));
    union_filter.addFilter(filter);

    tmp_request_list->reload(union_filter);

    if (tmp_request_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createPublicRequestDetail(tmp_request_list->get(0));
  }
}

ccReg::Invoicing::Invoice* ccReg_Session_i::getInvoiceDetail(ccReg::TID _id) {
  Register::Invoicing::Invoice *invoice = m_invoices->findId(_id);
  if (invoice && invoice->getActionCount()) {
    return createInvoiceDetail(invoice);
  } else {
    LOGGER("corba").debug(boost::format("constructing invoice filter for object id=%1%' detail")
        % _id);
    Register::Invoicing::List *tmp_invoice_list = m_invoicing_manager->createList();

    DBase::Filters::Union union_filter;
    DBase::Filters::Invoice *filter = new DBase::Filters::InvoiceImpl();
    filter->addId().setValue(DBase::ID(_id));
    union_filter.addFilter(filter);

    tmp_invoice_list->reload(union_filter, m_db_manager.get());

    if (tmp_invoice_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createInvoiceDetail(tmp_invoice_list->get(0));
  }
}

ccReg::Mailing::Detail* ccReg_Session_i::getMailDetail(ccReg::TID _id) {
  Register::Mail::Mail *mail = m_mails->findId(_id);
  if (mail) {
    return createMailDetail(mail);
  } else {
    LOGGER("corba").debug(boost::format("constructing mail filter for object id=%1%' detail")
        % _id);
    Register::Mail::List *tmp_mail_list = mail_manager_->createList();

    DBase::Filters::Union union_filter;
    DBase::Filters::Mail *filter = new DBase::Filters::MailImpl();
    filter->addId().setValue(DBase::ID(_id));
    union_filter.addFilter(filter);

    tmp_mail_list->reload(union_filter);

    if (tmp_mail_list->size() != 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    return createMailDetail(tmp_mail_list->get(0));
  }
}

ccReg::DomainDetail* ccReg_Session_i::createDomainDetail(Register::Domain::Domain* _domain) {
  TRACE("[CALL] ccReg_Session_i::createDomainDetail()");
  LOGGER("corba").debug(boost::format("generating domain detail for object id=%1%")
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

ccReg::ContactDetail* ccReg_Session_i::createContactDetail(Register::Contact::Contact* _contact) {
  TRACE("[CALL] ccReg_Session_i::createContactDetail()");
  LOGGER("corba").debug(boost::format("generating contact detail for object id=%1%")
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

ccReg::NSSetDetail* ccReg_Session_i::createNSSetDetail(Register::NSSet::NSSet* _nsset) {
  TRACE("[CALL] ccReg_Session_i::createNSSetDetail()");
  LOGGER("corba").debug(boost::format("generating nsset detail for object id=%1%")
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

ccReg::Registrar* ccReg_Session_i::createRegistrarDetail(Register::Registrar::Registrar* _registrar) {
  TRACE("[CALL] ccReg_Session_i::createRegistrarDetail()");
  LOGGER("corba").debug(boost::format("generating registrar detail for object id=%1%")
      % _registrar->getId());
  ccReg::Registrar *detail = new ccReg::Registrar;

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
  TRACE("[CALL] ccReg_Session_i::updateRegistrar()");
  Register::Registrar::RegistrarList *tmp_registrar_list =
      m_register_manager->getRegistrarManager()->getList();
  Register::Registrar::Registrar *update_registrar; // registrar to be created or updated
  if (!_registrar.id) {
    LOGGER("corba").debug("no registrar id specified; creating new registrar...");
    update_registrar = tmp_registrar_list->create();
  }
  else {
    LOGGER("corba").debug(boost::format("registrar '%1%' id=%2% specified; updating registrar...") 
      % _registrar.handle % _registrar.id);
    DBase::Filters::Union uf;
    DBase::Filters::Registrar *filter = new DBase::Filters::RegistrarImpl();
    filter->addId().setValue(DBase::ID(_registrar.id));
    uf.addFilter(filter);

    tmp_registrar_list->reload2(uf, m_db_manager.get());

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
  LOGGER("corba").debug(boost::format("registrar with id=%1% saved") 
      % update_registrar->getId());
}

ccReg::PublicRequest::Detail* ccReg_Session_i::createPublicRequestDetail(Register::PublicRequest::PublicRequest* _request) {
  ccReg::PublicRequest::Detail *detail = new ccReg::PublicRequest::Detail;
  
  detail->id = _request->getId();

  switch (_request->getStatus()) {
    case Register::PublicRequest::PRS_NEW:
      detail->status = ccReg::PublicRequest::PRS_NEW;
      break;
    case Register::PublicRequest::PRS_ANSWERED:
      detail->status = ccReg::PublicRequest::PRS_ANSWERED;
      break;
    case Register::PublicRequest::PRS_INVALID:
      detail->status = ccReg::PublicRequest::PRS_INVALID;
      break;
  }
  
  switch (_request->getType()) {
    case Register::PublicRequest::PRT_AUTHINFO_AUTO_RIF:
      detail->type = ccReg::PublicRequest::PRT_AUTHINFO_AUTO_RIF;
      break;
    case Register::PublicRequest::PRT_AUTHINFO_AUTO_PIF:
      detail->type = ccReg::PublicRequest::PRT_AUTHINFO_AUTO_PIF;
      break;
    case Register::PublicRequest::PRT_AUTHINFO_EMAIL_PIF:
      detail->type = ccReg::PublicRequest::PRT_AUTHINFO_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_AUTHINFO_POST_PIF:
      detail->type = ccReg::PublicRequest::PRT_AUTHINFO_POST_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_CHANGES_EMAIL_PIF:
      detail->type = ccReg::PublicRequest::PRT_BLOCK_CHANGES_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_CHANGES_POST_PIF:
      detail->type = ccReg::PublicRequest::PRT_BLOCK_CHANGES_POST_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_TRANSFER_EMAIL_PIF:
      detail->type = ccReg::PublicRequest::PRT_BLOCK_TRANSFER_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_BLOCK_TRANSFER_POST_PIF:
      detail->type = ccReg::PublicRequest::PRT_BLOCK_TRANSFER_POST_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_CHANGES_EMAIL_PIF:
      detail->type = ccReg::PublicRequest::PRT_UNBLOCK_CHANGES_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_CHANGES_POST_PIF:
      detail->type = ccReg::PublicRequest::PRT_UNBLOCK_CHANGES_POST_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_TRANSFER_EMAIL_PIF:
      detail->type = ccReg::PublicRequest::PRT_UNBLOCK_TRANSFER_EMAIL_PIF;
      break;
    case Register::PublicRequest::PRT_UNBLOCK_TRANSFER_POST_PIF:
      detail->type = ccReg::PublicRequest::PRT_UNBLOCK_TRANSFER_POST_PIF;
      break;
      
  }
  
  detail->createTime = DUPSTRDATE(_request->getCreateTime);
  detail->resolveTime = DUPSTRDATE(_request->getResolveTime);
  detail->reason = _request->getReason().c_str();
  detail->svTRID = _request->getSvTRID().c_str();
  detail->email = _request->getEmailToAnswer().c_str();
  detail->answerEmailId = _request->getAnswerEmailId();
  detail->registrar = _request->getRegistrarHandle().c_str();
  
  unsigned objects_size = _request->getObjectSize();
  detail->objects.length(objects_size);
  for (unsigned i = 0; i < objects_size; ++i) {  
    Register::PublicRequest::OID oid = _request->getObject(0);
    detail->objects[i].id = oid.id;
    detail->objects[i].handle = oid.handle.c_str();
    switch (oid.type) {
      case Register::PublicRequest::OT_DOMAIN:
        detail->objects[i].type = ccReg::PublicRequest::OT_DOMAIN;
        break;
      case Register::PublicRequest::OT_CONTACT:
        detail->objects[i].type = ccReg::PublicRequest::OT_CONTACT;
        break;
      case Register::PublicRequest::OT_NSSET:
        detail->objects[i].type = ccReg::PublicRequest::OT_NSSET;
        break;
      case Register::PublicRequest::OT_UNKNOWN:
        LOGGER("corba").error("Not allowed object type for PublicRequest detail!");
        break;
    }
  }
  
  return detail;
}

ccReg::Invoicing::Invoice* ccReg_Session_i::createInvoiceDetail(Register::Invoicing::Invoice *_invoice) {
  ccReg::Invoicing::Invoice *detail = new ccReg::Invoicing::Invoice;
  
  detail->id = _invoice->getId();
  detail->zone = _invoice->getZone();
  detail->crTime = DUPSTRDATE(_invoice->getCrTime);
  detail->taxDate = DUPSTRDATED(_invoice->getTaxDate);
  detail->fromDate = DUPSTRDATED(_invoice->getAccountPeriod().begin);
  detail->toDate = DUPSTRDATED(_invoice->getAccountPeriod().end);
  detail->type = (_invoice->getType() == Register::Invoicing::IT_DEPOSIT ? ccReg::Invoicing::IT_ADVANCE
                                                                         : ccReg::Invoicing::IT_ACCOUNT);
  detail->number = DUPSTRC(Util::stream_cast<std::string>(_invoice->getNumber()));
  detail->registrarId = _invoice->getRegistrar();
  detail->registrarHandle = DUPSTRC(_invoice->getClient()->getHandle());
  detail->credit = DUPSTRC(formatMoney(_invoice->getCredit()));
  detail->price = DUPSTRC(formatMoney(_invoice->getPrice()));
  detail->vatRate = _invoice->getVatRate();
  detail->total = DUPSTRC(formatMoney(_invoice->getTotal()));
  detail->totalVAT = DUPSTRC(formatMoney(_invoice->getTotalVAT()));
  detail->varSymbol = DUPSTRC(_invoice->getVarSymbol());
  detail->filePDF = _invoice->getFilePDF();
  detail->fileXML = _invoice->getFileXML();
  
  detail->payments.length(_invoice->getSourceCount());
  for (unsigned n = 0; n < _invoice->getSourceCount(); ++n) {
    const Register::Invoicing::PaymentSource *ps = _invoice->getSource(n);
    detail->payments[n].id = ps->getId();
    detail->payments[n].price = DUPSTRC(formatMoney(ps->getPrice()));
    detail->payments[n].balance = DUPSTRC(formatMoney(ps->getCredit()));
    detail->payments[n].number = DUPSTRC(Util::stream_cast<std::string>(ps->getNumber()));
  }
  
  detail->actions.length(_invoice->getActionCount());
  for (unsigned n = 0; n < _invoice->getActionCount(); ++n) {
    const Register::Invoicing::PaymentAction *pa = _invoice->getAction(n);
    detail->actions[n].objectId = pa->getObjectId();
    detail->actions[n].objectName = DUPSTRFUN(pa->getObjectName);
    detail->actions[n].actionTime = DUPSTRDATE(pa->getActionTime);
    detail->actions[n].exDate = DUPSTRDATED(pa->getExDate);
    detail->actions[n].actionType = pa->getAction();
    detail->actions[n].unitsCount = pa->getUnitsCount();
    detail->actions[n].pricePerUnit = DUPSTRC(formatMoney(pa->getPricePerUnit()));
    detail->actions[n].price = DUPSTRC(formatMoney(pa->getPrice()));
  }
  
  return detail;
}

ccReg::Mailing::Detail* ccReg_Session_i::createMailDetail(Register::Mail::Mail *_mail) {
  ccReg::Mailing::Detail *detail = new ccReg::Mailing::Detail;
  
  detail->id = _mail->getId();
  detail->type = _mail->getType();
  detail->status = _mail->getStatus();
  detail->createTime = DUPSTRDATE(_mail->getCreateTime);
  detail->modTime = DUPSTRDATE(_mail->getModTime);
  detail->content = DUPSTRC(_mail->getContent());
  
  detail->handles.length(_mail->getHandleSize());
  for (unsigned i = 0; i < _mail->getHandleSize(); ++i)
    detail->handles[i] = DUPSTRC(_mail->getHandle(i));
  
  detail->attachments.length(_mail->getAttachmentSize());
  for (unsigned i = 0; i < _mail->getAttachmentSize(); ++i)
    detail->attachments[i] = _mail->getAttachment(i);
  
  return detail;
}
