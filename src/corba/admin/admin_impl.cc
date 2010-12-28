/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <memory>
#include <iomanip>
#include <corba/ccReg.hh>

#include "common.h"
#include "admin_impl.h"
#include "old_utils/log.h"
#include "old_utils/dbsql.h"
#include "fredlib/registry.h"
#include "fredlib/notify.h"
#include "corba/mailer_manager.h"
#include "fredlib/messages/messages_impl.h"
#include "fredlib/object_states.h"
#include "bank_payment.h"

#include "log/logger.h"
#include "log/context.h"

#include "random.h"

#include "corba/connection_releaser.h"

#ifdef ADIF
#endif

class Registry_RegistrarCertification_i;
class Registry_RegistrarGroup_i;

ccReg_Admin_i::ccReg_Admin_i(const std::string _database,
                             NameService *_ns,
                             Config::Conf& _cfg,
                             bool _session_garbage) throw (DB_CONNECT_FAILED) :
  m_connection_string(_database), ns(_ns), cfg(_cfg), bankingInvoicing(_ns) {

  /* HACK: to recognize ADIFD and PIFD until separation of objects */
  if (_session_garbage) {
    server_name_ = ("adifd");
  }
  else {
    server_name_ = ("pifd");
  }

  //instances held until deactivation
  Registry_Registrar_Certification_Manager_i * reg_cert_mgr_i
      = new Registry_Registrar_Certification_Manager_i();
  reg_cert_mgr_ref_ = reg_cert_mgr_i->_this();
  reg_cert_mgr_i->_remove_ref();
  Registry_Registrar_Group_Manager_i * reg_grp_mgr_i
      = new Registry_Registrar_Group_Manager_i();
  reg_grp_mgr_ref_ = reg_grp_mgr_i->_this();
  reg_grp_mgr_i->_remove_ref();

  Logging::Context ctx(server_name_);

  // these object are shared between threads (CAUTION)
  if (!db.OpenDatabase(m_connection_string.c_str())) {
    LOG(ALERT_LOG,
        "cannot connect to DATABASE %s",
        m_connection_string.c_str());
    throw DB_CONNECT_FAILED();
  }

  registry_manager_.reset(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  registry_manager_->initStates();

  if (_session_garbage) {
    session_garbage_active_ = true;
    session_garbage_thread_ = new boost::thread(boost::bind(&ccReg_Admin_i::garbageSession, this));
  }
}

ccReg_Admin_i::~ccReg_Admin_i() {
  TRACE("[CALL] ccReg_Admin_i::~ccReg_Admin_i()");
  db.Disconnect();

  session_garbage_active_ = false;
  cond_.notify_one();
  session_garbage_thread_->join();
  // session_garbage_thread_->join();
  delete session_garbage_thread_;
  
  /// sessions cleanup
  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::iterator it = m_session_list.begin();
  while (it != m_session_list.end()) {
    std::string session_id = it->first;
    delete it->second;
    m_session_list.erase(it++);
    LOGGER(PACKAGE).debug(boost::format("session '%1%' destroyed") % session_id);
  }

  TRACE("Admin object completely destroyed.");
}

#define SWITCH_CONVERT(x) case Fred::x : ch->handleClass = ccReg::x; break
#define SWITCH_CONVERT_T(x) case Fred::x : ch->hType = ccReg::x; break
void ccReg_Admin_i::checkHandle(const char* handle,
                                ccReg::CheckHandleTypeSeq_out chso) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB ldb;
  ldb.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&ldb, cfg.get<bool>("registry.restricted_handles")));
  ccReg::CheckHandleTypeSeq* chs = new ccReg::CheckHandleTypeSeq;
  Fred::CheckHandleList chl;
  r->checkHandle(handle, chl, true); // allow IDN in whois queries
  chs->length(chl.size());
  for (unsigned i=0; i< chl.size(); i++) {
    ccReg::CheckHandleType *ch = &(*chs)[i];
    Fred::CheckHandle& chd = chl[i];
    ch->newHandle = CORBA::string_dup(chd.newHandle.c_str());
    ch->conflictHandle = CORBA::string_dup(chd.conflictHandle.c_str());
    switch (chd.type) {
      SWITCH_CONVERT_T(HT_ENUM_NUMBER);
      SWITCH_CONVERT_T(HT_ENUM_DOMAIN);
      SWITCH_CONVERT_T(HT_DOMAIN);
      SWITCH_CONVERT_T(HT_CONTACT);
      SWITCH_CONVERT_T(HT_NSSET);
      SWITCH_CONVERT_T(HT_KEYSET);
      SWITCH_CONVERT_T(HT_REGISTRAR);
      SWITCH_CONVERT_T(HT_OTHER);
    }
    switch (chd.handleClass) {
      SWITCH_CONVERT(CH_UNREGISTRABLE);
      SWITCH_CONVERT(CH_UNREGISTRABLE_LONG);
      SWITCH_CONVERT(CH_REGISTRED);
      SWITCH_CONVERT(CH_REGISTRED_PARENT);
      SWITCH_CONVERT(CH_REGISTRED_CHILD);
      SWITCH_CONVERT(CH_PROTECTED);
      SWITCH_CONVERT(CH_FREE);
    }
  }
  chso = chs;
  ldb.Disconnect();
}

void ccReg_Admin_i::garbageSession() {
  Logging::Context ctx(server_name_);
  Logging::Context ctx2("session-garbage");

  LOGGER(PACKAGE).debug("thread started...");
  
  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  while (session_garbage_active_) {
    LOGGER(PACKAGE).debug("procedure sleeped");
    
    boost::xtime sleep_time;
    boost::xtime_get(&sleep_time, boost::TIME_UTC);
    sleep_time.sec += cfg.get<unsigned>("adifd.session_garbage");
    cond_.timed_wait(scoped_lock, sleep_time);
    
    LOGGER(PACKAGE).debug("procedure invoked");
  
    SessionListType::iterator it = m_session_list.begin();
    while (it != m_session_list.end()) {
      if (it->second->isTimeouted()) {
        std::string session_id = it->first;
        delete it->second;
        m_session_list.erase(it++);
        LOGGER(PACKAGE).debug(boost::format("session '%1%' deleted -- remains '%2%'") 
            % session_id % m_session_list.size());
      }
      else {
        ++it;
      }
    }
  }
  LOGGER(PACKAGE).debug("thread stopped");
}

void ccReg_Admin_i::authenticateUser(const char* _username,
                                     const char* _password)
    throw (ccReg::Admin::AuthFailed) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::authenticateUser('%1%', '******')")
      % _username);

  /* for now we let everybody in :) */

}

char* ccReg_Admin_i::createSession(const char* username) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::createSession('%1%')") % username);

  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  unsigned sess_max = cfg.get<unsigned>("adifd.session_max");
  if (sess_max && m_session_list.size() == sess_max) {
    LOGGER(PACKAGE).info(boost::format("session limit (max=%1%) exceeded") % sess_max);

    SessionListType::iterator it        = m_session_list.begin();
    SessionListType::iterator it_oldest = m_session_list.begin();
    for (; it != m_session_list.end(); ++it) {
      LOGGER(PACKAGE).debug(boost::format("session %1% - last activity at %2%")
                                          % it->second->getId()
                                          % it->second->getLastActivity());
      if (it_oldest->second->getLastActivity() > it->second->getLastActivity()) {
        it_oldest = it;
      }
    }

    LOGGER(PACKAGE).info(boost::format("destroying oldest session -- %1% (last activity at %2%)") 
                                       % it_oldest->second->getId()
                                       % it_oldest->second->getLastActivity());
    delete it_oldest->second;
    m_session_list.erase(it_oldest);
  }

  ccReg_User_i *user_info = new ccReg_User_i(1 /* dummy id until user management */, username, username, username);

  std::string session_id = "sessid#" + Random::string_alphanum(5) + "-" + username;

  
  ccReg_Session_i *session = new ccReg_Session_i(session_id, m_connection_string, ns, cfg, bankingInvoicing._this(), user_info);
  m_session_list[session_id] = session; 

#ifdef ADIF
  LOGGER(PACKAGE).notice(boost::format("admin session '%1%' created -- total number of sessions is '%2%'")
      % session_id % m_session_list.size());
#endif

#ifdef LOGD
  LOGGER(PACKAGE).notice(boost::format("admin session '%1%' created -- total number of sessions is '%2%'")
      % session_id % m_session_list.size());
#endif

  return CORBA::string_dup(session_id.c_str());
}

void ccReg_Admin_i::destroySession(const char* _session_id) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::destroySession('%1%')") % _session_id);
  
  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::iterator it = m_session_list.find(_session_id);
  if (it == m_session_list.end()) {
    LOGGER(PACKAGE).debug(boost::format("session '%1%' not found -- already destroyed")
        % _session_id);
    return;
  }

  delete it->second;
  m_session_list.erase(it);
}

ccReg::Session_ptr ccReg_Admin_i::getSession(const char* _session_id)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getSession('%1%')") % _session_id);

  // garbageSession();

  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::const_iterator it = m_session_list.find(_session_id);
  if (it == m_session_list.end()) {
    LOGGER(PACKAGE).debug(boost::format("session '%1%' not found -- deleted due to timout")
        % _session_id);
    throw ccReg::Admin::ObjectNotFound();
  }

  it->second->updateActivity();
  return it->second->_this();
}


void ccReg_Admin_i::fillRegistrar(ccReg::AdminRegistrar& creg,
                                  Fred::Registrar::Registrar *reg) {

  creg.id = reg->getId();
  creg.name = DUPSTRFUN(reg->getName);
  creg.handle = DUPSTRFUN(reg->getHandle);
  creg.url = DUPSTRFUN(reg->getURL);
  creg.organization = DUPSTRFUN(reg->getOrganization);
  creg.street1 = DUPSTRFUN(reg->getStreet1);
  creg.street2 = DUPSTRFUN(reg->getStreet2);
  creg.street3 = DUPSTRFUN(reg->getStreet3);
  creg.city = DUPSTRFUN(reg->getCity);
  creg.postalcode = DUPSTRFUN(reg->getPostalCode);
  creg.stateorprovince = DUPSTRFUN(reg->getProvince);
  creg.country = DUPSTRFUN(reg->getCountry);
  creg.telephone = DUPSTRFUN(reg->getTelephone);
  creg.fax = DUPSTRFUN(reg->getFax);
  creg.email = DUPSTRFUN(reg->getEmail);
  creg.credit = DUPSTRC(formatMoney(reg->getCredit()*100));
  creg.access.length(reg->getACLSize());
  for (unsigned i=0; i<reg->getACLSize(); i++) {
    creg.access[i].md5Cert = DUPSTRFUN(reg->getACL(i)->getCertificateMD5);
    creg.access[i].password = DUPSTRFUN(reg->getACL(i)->getPassword);
  }
  creg.hidden = reg->getHandle() == "REG-CZNIC" ? true : false;
}

ccReg::RegistrarList* ccReg_Admin_i::getRegistrars()
    throw (ccReg::Admin::SQL_ERROR) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try { 
    std::auto_ptr<Fred::Manager> regm(
        Fred::Manager::create(&ldb, cfg.get<bool>("registry.restricted_handles"))
    );
    Fred::Registrar::Manager *rm = regm->getRegistrarManager();
    Fred::Registrar::RegistrarList::AutoPtr rl = rm->createList();

    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    unionFilter->addFilter( new Database::Filters::RegistrarImpl(true) );
    rl->reload(*unionFilter.get());

    LOG( NOTICE_LOG, "getRegistrars: num -> %d", rl->size() );
    ccReg::RegistrarList* reglist = new ccReg::RegistrarList;
    reglist->length(rl->size());
    for (unsigned i=0; i<rl->size(); i++)
    fillRegistrar((*reglist)[i],rl->get(i));
    ldb.Disconnect();
    return reglist;
  }
  catch (Fred::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

ccReg::RegistrarList* ccReg_Admin_i::getRegistrarsByZone(const char *zone)
    throw (ccReg::Admin::SQL_ERROR) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try {
    std::auto_ptr<Fred::Manager> regm(
        Fred::Manager::create(&ldb,cfg.get<bool>("registry.restricted_handles"))
    );
    Fred::Registrar::Manager *rm = regm->getRegistrarManager();
    Fred::Registrar::RegistrarList::AutoPtr rl = rm->createList();

    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    std::auto_ptr<Database::Filters::Registrar> r ( new Database::Filters::RegistrarImpl(true));
    r->addZoneFqdn().setValue(zone);
    unionFilter->addFilter( r.release() );
    rl->reload(*unionFilter.get());

    LOG( NOTICE_LOG, "getRegistrars: num -> %d", rl->size() );
    ccReg::RegistrarList* reglist = new ccReg::RegistrarList;
    reglist->length(rl->size());
    for (unsigned i=0; i<rl->size(); i++)
    fillRegistrar((*reglist)[i],rl->get(i));
    ldb.Disconnect();
    return reglist;
  }
  catch (Fred::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

ccReg::AdminRegistrar* ccReg_Admin_i::getRegistrarById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound, ccReg::Admin::SQL_ERROR) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  LOG( NOTICE_LOG, "getRegistarByHandle: id -> %lld", (unsigned long long)id );
  if (!id) throw ccReg::Admin::ObjectNotFound();
  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try {
    std::auto_ptr<Fred::Manager> regm(
        Fred::Manager::create(&ldb,cfg.get<bool>("registry.restricted_handles"))
    );
    Fred::Registrar::Manager *rm = regm->getRegistrarManager();
    Fred::Registrar::RegistrarList::AutoPtr rl = rm->createList();

    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    std::auto_ptr<Database::Filters::Registrar> r ( new Database::Filters::RegistrarImpl(true));
    r->addId().setValue(Database::ID(id));
    unionFilter->addFilter( r.release() );
    rl->reload(*unionFilter.get());

    if (rl->size() < 1) {
      ldb.Disconnect();
      throw ccReg::Admin::ObjectNotFound();
    }
    ccReg::AdminRegistrar* creg = new ccReg::AdminRegistrar;
    fillRegistrar(*creg,rl->get(0));
    ldb.Disconnect();
    return creg;
  }
  catch (Fred::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

ccReg::AdminRegistrar* ccReg_Admin_i::getRegistrarByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound, ccReg::Admin::SQL_ERROR) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  LOG( NOTICE_LOG, "getRegistarByHandle: handle -> %s", handle );
  if (!handle || !*handle) throw ccReg::Admin::ObjectNotFound();
  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try {
    std::auto_ptr<Fred::Manager> regm(
        Fred::Manager::create(&ldb,cfg.get<bool>("registry.restricted_handles"))
    );
    Fred::Registrar::Manager *rm = regm->getRegistrarManager();
    Fred::Registrar::RegistrarList::AutoPtr rl = rm->createList();
    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    std::auto_ptr<Database::Filters::Registrar> r ( new Database::Filters::RegistrarImpl(true));
    r->addHandle().setValue(handle);
    unionFilter->addFilter( r.release() );
    rl->reload(*unionFilter.get());

    if (rl->size() < 1) {
      ldb.Disconnect();
      throw ccReg::Admin::ObjectNotFound();
    }
    ccReg::AdminRegistrar* creg = new ccReg::AdminRegistrar;
    fillRegistrar(*creg,rl->get(0));
    ldb.Disconnect();
    return creg;
  }
  catch (Fred::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

void ccReg_Admin_i::fillContact(ccReg::ContactDetail* cc,
                                Fred::Contact::Contact* c) {

  cc->id = c->getId();
  cc->handle = DUPSTRFUN(c->getHandle);
  cc->roid = DUPSTRFUN(c->getROID);
  cc->registrarHandle = DUPSTRFUN(c->getRegistrarHandle);
  cc->transferDate = DUPSTRDATE(c->getTransferDate);
  cc->updateDate = DUPSTRDATE(c->getUpdateDate);
  cc->createDate = DUPSTRDATE(c->getCreateDate);
  cc->createRegistrarHandle = DUPSTRFUN(c->getCreateRegistrarHandle);
  cc->updateRegistrarHandle = DUPSTRFUN(c->getUpdateRegistrarHandle);
  cc->authInfo = DUPSTRFUN(c->getAuthPw);
  cc->name = DUPSTRFUN(c->getName);
  cc->organization = DUPSTRFUN(c->getOrganization);
  cc->street1 = DUPSTRFUN(c->getStreet1);
  cc->street2 = DUPSTRFUN(c->getStreet2);
  cc->street3 = DUPSTRFUN(c->getStreet3);
  cc->province = DUPSTRFUN(c->getProvince);
  cc->postalcode = DUPSTRFUN(c->getPostalCode);
  cc->city = DUPSTRFUN(c->getCity);
  cc->country = DUPSTRFUN(c->getCountry);
  cc->telephone = DUPSTRFUN(c->getTelephone);
  cc->fax = DUPSTRFUN(c->getFax);
  cc->email = DUPSTRFUN(c->getEmail);
  cc->notifyEmail = DUPSTRFUN(c->getNotifyEmail);
  cc->ssn = DUPSTRFUN(c->getSSN);
  cc->ssnType = DUPSTRFUN(c->getSSNType);
  cc->vat = DUPSTRFUN(c->getVAT);
  cc->discloseName = c->getDiscloseName();
  cc->discloseOrganization = c->getDiscloseOrganization();
  cc->discloseAddress = c->getDiscloseAddr();
  cc->discloseEmail = c->getDiscloseEmail();
  cc->discloseTelephone = c->getDiscloseTelephone();
  cc->discloseFax = c->getDiscloseFax();
  cc->discloseIdent = c->getDiscloseIdent();
  cc->discloseVat = c->getDiscloseVat();
  cc->discloseNotifyEmail = c->getDiscloseNotifyEmail();
  std::vector<unsigned> slist;
  for (unsigned i=0; i<c->getStatusCount(); i++) {
    if (registry_manager_->getStatusDesc(
        c->getStatusByIdx(i)->getStatusId()
    )->getExternal())
      slist.push_back(c->getStatusByIdx(i)->getStatusId());
  }
  cc->statusList.length(slist.size());
  for (unsigned i=0; i<slist.size(); i++)
    cc->statusList[i] = slist[i];
}

ccReg::ContactDetail* ccReg_Admin_i::getContactByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getContactByHandle('%1%')") % handle);

  DB db;
  if (!handle || !*handle)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Contact::Manager *cr = r->getContactManager();
  std::auto_ptr<Fred::Contact::List> cl(cr->createList());
  cl->setWildcardExpansion(false);
  cl->setHandleFilter(handle);
  cl->reload();
  if (cl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::ContactDetail* cc = new ccReg::ContactDetail;
  fillContact(cc, cl->getContact(0));
  db.Disconnect();
  return cc;
}

ccReg::ContactDetail* ccReg_Admin_i::getContactById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getContactById(%1%)") % id);
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Contact::Manager *cr = r->getContactManager();
  std::auto_ptr<Fred::Contact::List> cl(cr->createList());

  Database::Filters::Union uf;
  Database::Filters::Contact *cf = new Database::Filters::ContactHistoryImpl();
  cf->addId().setValue(Database::ID(id));
  uf.addFilter(cf);
  cl->reload(uf);

  //cl->setIdFilter(id);
  //cl->reload();

  if (cl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::ContactDetail* cc = new ccReg::ContactDetail;
  fillContact(cc, cl->getContact(0));
  db.Disconnect();
  TRACE(boost::format("[IN] ccReg_Admin_i::getContactById(%1%): found contact with handle '%2%'")
      % id % cc->handle);
  return cc;
}

void ccReg_Admin_i::fillNSSet(ccReg::NSSetDetail* cn, Fred::NSSet::NSSet* n) {
  cn->id = n->getId();
  cn->handle = DUPSTRFUN(n->getHandle);
  cn->roid = DUPSTRFUN(n->getROID);
  cn->registrarHandle = DUPSTRFUN(n->getRegistrarHandle);
  cn->transferDate = DUPSTRDATE(n->getTransferDate);
  cn->updateDate = DUPSTRDATE(n->getUpdateDate);
  cn->createDate = DUPSTRDATE(n->getCreateDate);
  cn->createRegistrarHandle = DUPSTRFUN(n->getCreateRegistrarHandle);
  cn->updateRegistrarHandle = DUPSTRFUN(n->getUpdateRegistrarHandle);
  cn->authInfo = DUPSTRFUN(n->getAuthPw);
  cn->admins.length(n->getAdminCount());
  try {
    for (unsigned i=0; i<n->getAdminCount(); i++)
    cn->admins[i] = DUPSTRC(n->getAdminByIdx(i));
  }
  catch (Fred::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
  cn->hosts.length(n->getHostCount());
  for (unsigned i=0; i<n->getHostCount(); i++) {
    cn->hosts[i].fqdn = DUPSTRFUN(n->getHostByIdx(i)->getNameIDN);
    cn->hosts[i].inet.length(n->getHostByIdx(i)->getAddrCount());
    for (unsigned j=0; j<n->getHostByIdx(i)->getAddrCount(); j++)
      cn->hosts[i].inet[j] = DUPSTRC(n->getHostByIdx(i)->getAddrByIdx(j));
  }
  std::vector<unsigned> slist;
  for (unsigned i=0; i<n->getStatusCount(); i++) {
    if (registry_manager_->getStatusDesc(
        n->getStatusByIdx(i)->getStatusId()
    )->getExternal())
      slist.push_back(n->getStatusByIdx(i)->getStatusId());
  }
  cn->statusList.length(slist.size());
  for (unsigned i=0; i<slist.size(); i++)
    cn->statusList[i] = slist[i];

  cn->reportLevel = n->getCheckLevel();
}

ccReg::NSSetDetail* ccReg_Admin_i::getNSSetByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getNSSetByHandle('%1%')") % handle);

  DB db;
  if (!handle || !*handle)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::NSSet::Manager *nr = r->getNSSetManager();
  std::auto_ptr<Fred::NSSet::List> nl(nr->createList());
  nl->setWildcardExpansion(false);
  nl->setHandleFilter(handle);
  nl->reload();
  if (nl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::NSSetDetail* cn = new ccReg::NSSetDetail;
  fillNSSet(cn, nl->getNSSet(0));
  db.Disconnect();
  return cn;
}

ccReg::NSSetDetail* ccReg_Admin_i::getNSSetById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getNSSetById('%1%')") % id);
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::NSSet::Manager *nr = r->getNSSetManager();
  std::auto_ptr<Fred::NSSet::List> nl(nr->createList());

  Database::Filters::Union uf;
  Database::Filters::NSSet *nf = new Database::Filters::NSSetHistoryImpl();
  nf->addId().setValue(Database::ID(id));
  uf.addFilter(nf);
  nl->reload(uf);

  // nl->setIdFilter(id);
  // nl->reload();

  if (nl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::NSSetDetail* cn = new ccReg::NSSetDetail;
  fillNSSet(cn, nl->getNSSet(0));
  db.Disconnect();
  return cn;
}

void
ccReg_Admin_i::fillKeySet(ccReg::KeySetDetail *ck, Fred::KeySet::KeySet *k)
{
    ck->id = k->getId();
    ck->handle          = DUPSTRFUN(k->getHandle);
    ck->roid            = DUPSTRFUN(k->getROID);
    ck->registrarHandle = DUPSTRFUN(k->getRegistrarHandle);
    ck->transferDate    = DUPSTRDATE(k->getTransferDate);
    ck->updateDate      = DUPSTRDATE(k->getUpdateDate);
    ck->createDate      = DUPSTRDATE(k->getCreateDate);
    ck->createRegistrarHandle = DUPSTRFUN(k->getCreateRegistrarHandle);
    ck->updateRegistrarHandle = DUPSTRFUN(k->getUpdateRegistrarHandle);
    ck->authInfo        = DUPSTRFUN(k->getAuthPw);
    ck->admins.length(k->getAdminCount());
    try {
        for (unsigned int i = 0; i < k->getAdminCount(); i++)
            ck->admins[i] = DUPSTRC(k->getAdminByIdx(i));
    }
    catch (Fred::NOT_FOUND) {
        // TODO implement error handling
    }

    ck->dsrecords.length(k->getDSRecordCount());
    for (unsigned int i = 0; i < k->getDSRecordCount(); i++) {
        ck->dsrecords[i].keyTag = k->getDSRecordByIdx(i)->getKeyTag();
        ck->dsrecords[i].alg = k->getDSRecordByIdx(i)->getAlg();
        ck->dsrecords[i].digestType = k->getDSRecordByIdx(i)->getDigestType();
        ck->dsrecords[i].digest = DUPSTRC(k->getDSRecordByIdx(i)->getDigest());
        ck->dsrecords[i].maxSigLife = k->getDSRecordByIdx(i)->getMaxSigLife();
    }

    ck->dnskeys.length(k->getDNSKeyCount());
    for (unsigned int i = 0; i < k->getDNSKeyCount(); i++) {
        ck->dnskeys[i].flags = k->getDNSKeyByIdx(i)->getFlags();
        ck->dnskeys[i].protocol = k->getDNSKeyByIdx(i)->getProtocol();
        ck->dnskeys[i].alg = k->getDNSKeyByIdx(i)->getAlg();
        ck->dnskeys[i].key = DUPSTRFUN(k->getDNSKeyByIdx(i)->getKey);
    }

    std::vector<unsigned int> slist;
    for (unsigned int i = 0; i < k->getStatusCount(); i++)
        if (registry_manager_->getStatusDesc(
                    k->getStatusByIdx(i)->getStatusId())->getExternal())
            slist.push_back(k->getStatusByIdx(i)->getStatusId());

    ck->statusList.length(slist.size());
    for (unsigned int i = 0; i < slist.size(); i++)
        ck->statusList[i] = slist[i];
}

ccReg::KeySetDetail *
ccReg_Admin_i::getKeySetByHandle(const char *handle)
    throw (ccReg::Admin::ObjectNotFound)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

    TRACE(boost::format(
                "[CALL] ccReg_Admin_i::getKeySetByHandle('%1%')") % handle);

    DB db;
    if (!handle || !*handle)
        throw ccReg::Admin::ObjectNotFound();

    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db,
                cfg.get<bool>("registry.restricted_handles")));
    Fred::KeySet::Manager *kr = r->getKeySetManager();
    std::auto_ptr<Fred::KeySet::List> kl(kr->createList());
    kl->setWildcardExpansion(false);
    kl->setHandleFilter(handle);
    kl->reload();

    if (kl->getCount() != 1) {
        db.Disconnect();
        throw ccReg::Admin::ObjectNotFound();
    }

    ccReg::KeySetDetail *ck = new ccReg::KeySetDetail;
    fillKeySet(ck, kl->getKeySet(0));
    db.Disconnect();
    return ck;
}

ccReg::KeySetDetail *
ccReg_Admin_i::getKeySetById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

    TRACE(boost::format(
                "[CALL] ccReg_Admin_i::getKeySetById('%1%')") % id);
    DB db;
    if (!id)
        throw ccReg::Admin::ObjectNotFound();
    db.OpenDatabase(m_connection_string.c_str());

    std::auto_ptr<Fred::Manager> r(
            Fred::Manager::create(&db,  cfg.get<bool>("registry.restricted_handles")));
    Fred::KeySet::Manager *kr = r->getKeySetManager();
    std::auto_ptr<Fred::KeySet::List> kl(kr->createList());

    Database::Filters::Union uf;
    Database::Filters::KeySet *kf = new Database::Filters::KeySetHistoryImpl();
    kf->addId().setValue(Database::ID(id));
    uf.addFilter(kf);
    kl->reload(uf);

    if (kl->getCount() != 1) {
        db.Disconnect();
        throw ccReg::Admin::ObjectNotFound();
    }

    ccReg::KeySetDetail *ck = new ccReg::KeySetDetail;
    fillKeySet(ck, kl->getKeySet(0));
    db.Disconnect();
    return ck;
}
// ccReg::KeySetDetail *
// ccReg_Admin_i::getKeySetByDomainFQDN(const char *fqdn)
    // throw (ccReg::Admin::ObjectNotFound)
// {
    // DB db;
    // if (!fqdn)
        // throw ccReg::Admin::ObjectNotFound();
    // db.OpenDatabase(m_connection_string.c_str());
// 
    // std::auto_ptr<Fred::Manager> regMan(
            // Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
    // Fred::KeySet::Manager *keyR = regMan->getKeySetManager();
    // std::auto_ptr<Fred::KeySet::List> klist(keyR->createList());
    // 
    // Database::Filters::Union uf;
    // Database::Filters::KeySet *keyF = new Database::Filters::KeySetHistoryImpl();
    // kf-
// }

void ccReg_Admin_i::fillEPPAction(ccReg::EPPAction* cea,
                                  const Fred::Registrar::EPPAction *rea) {
  cea->id = rea->getId();
  cea->xml = DUPSTRFUN(rea->getEPPMessageIn);
  cea->xml_out = DUPSTRFUN(rea->getEPPMessageOut);
  cea->time = DUPSTRDATE(rea->getStartTime);
  cea->type = DUPSTRFUN(rea->getTypeName);
  cea->objectHandle = DUPSTRFUN(rea->getHandle);
  cea->registrarHandle = DUPSTRFUN(rea->getRegistrarHandle);
  cea->result = rea->getResult();
  cea->clTRID = DUPSTRFUN(rea->getClientTransactionId);
  cea->svTRID = DUPSTRFUN(rea->getServerTransactionId);
}

ccReg::EPPAction* ccReg_Admin_i::getEPPActionBySvTRID(const char* svTRID)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB db;
  if (!svTRID || !*svTRID)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Registrar::Manager *rm = r->getRegistrarManager();
  Fred::Registrar::EPPActionList *eal = rm->getEPPActionList();
  eal->setSvTRIDFilter(svTRID);
  eal->setPartialLoad(false);
  eal->reload();
  if (eal->size() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::EPPAction* ea = new ccReg::EPPAction;
  fillEPPAction(ea, eal->get(0));
  db.Disconnect();
  return ea;
}

ccReg::EPPAction* ccReg_Admin_i::getEPPActionById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getEPPActionById(%1%)") % id);
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  
  DB ldb;  
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }  
  try {
    std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&ldb,cfg.get<bool>("registry.restricted_handles")));
    Fred::Registrar::Manager *rm = r->getRegistrarManager();
    Fred::Registrar::EPPActionList *eal = rm->getEPPActionList();
    eal->setIdFilter(id);
    eal->setPartialLoad(false);
    eal->reload();
    if (eal->size() != 1) {
      ldb.Disconnect();
      throw ccReg::Admin::ObjectNotFound();
    }
    ccReg::EPPAction* ea = new ccReg::EPPAction;
    fillEPPAction(ea, eal->get(0));
    ldb.Disconnect();
    TRACE(boost::format("[IN] ccReg_Admin_i::getEPPActionById(%1%): found action for object with handle '%2%'")
        % id % ea->objectHandle);
    return ea;
  }
  catch (Fred::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

void ccReg_Admin_i::fillDomain(ccReg::DomainDetail* cd,
                               Fred::Domain::Domain* d) {
  cd->id = d->getId();
  cd->fqdn = DUPSTRFUN(d->getFQDNIDN);
  cd->roid = DUPSTRFUN(d->getROID);
  cd->registrarHandle = DUPSTRFUN(d->getRegistrarHandle);
  cd->transferDate = DUPSTRDATE(d->getTransferDate);
  cd->updateDate = DUPSTRDATE(d->getUpdateDate);
  cd->createDate = DUPSTRDATE(d->getCreateDate);
  cd->createRegistrarHandle = DUPSTRFUN(d->getCreateRegistrarHandle);
  cd->updateRegistrarHandle = DUPSTRFUN(d->getUpdateRegistrarHandle);
  cd->authInfo = DUPSTRFUN(d->getAuthPw);
  cd->registrantHandle = DUPSTRFUN(d->getRegistrantHandle);
  cd->expirationDate = DUPSTRDATED(d->getExpirationDate);
  cd->valExDate = DUPSTRDATED(d->getValExDate);
  cd->nssetHandle = DUPSTRFUN(d->getNSSetHandle);
  cd->keysetHandle = DUPSTRFUN(d->getKeySetHandle);
  cd->admins.length(d->getAdminCount(1));
  cd->temps.length(d->getAdminCount(2));
  std::vector<unsigned> slist;
  for (unsigned i=0; i<d->getStatusCount(); i++) {
    if (registry_manager_->getStatusDesc(
        d->getStatusByIdx(i)->getStatusId()
    )->getExternal())
      slist.push_back(d->getStatusByIdx(i)->getStatusId());
  }
  cd->statusList.length(slist.size());
  for (unsigned i=0; i<slist.size(); i++)
    cd->statusList[i] = slist[i];
  try {
    for (unsigned i=0; i<d->getAdminCount(1); i++)
    cd->admins[i] = DUPSTRC(d->getAdminHandleByIdx(i,1));
    for (unsigned i=0; i<d->getAdminCount(2); i++)
    cd->temps[i] = DUPSTRC(d->getAdminHandleByIdx(i,2));
  }
  catch (Fred::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
}

ccReg::DomainDetail* ccReg_Admin_i::getDomainByFQDN(const char* fqdn)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getDomainByFQDN('%1%')") % fqdn);

  DB db;
  if (!fqdn || !*fqdn)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Domain::Manager *dm = r->getDomainManager();
  std::auto_ptr<Fred::Domain::List> dl(dm->createList());
  dl->setWildcardExpansion(false);
  dl->setFQDNFilter(r->getZoneManager()->encodeIDN(fqdn));
  dl->reload();
  if (dl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::DomainDetail* cd = new ccReg::DomainDetail;
  fillDomain(cd, dl->getDomain(0));
  db.Disconnect();
  return cd;
}

ccReg::DomainDetail* ccReg_Admin_i::getDomainById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getDomainById('%1%')") % id);
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());

  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Domain::Manager *dm = r->getDomainManager();
  std::auto_ptr<Fred::Domain::List> dl(dm->createList());

  Database::Filters::Union uf;
  Database::Filters::Domain *df = new Database::Filters::DomainHistoryImpl();
  df->addId().setValue(Database::ID(id));
  uf.addFilter(df);
  dl->reload(uf);

  //dl->setIdFilter(id);
  //dl->reload();


  if (dl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::DomainDetail* cd = new ccReg::DomainDetail;
  fillDomain(cd, dl->getDomain(0));
  db.Disconnect();
  return cd;
}
ccReg::DomainDetails *
ccReg_Admin_i::getDomainsByKeySetId(ccReg::TID id, CORBA::Long limit)
    throw (ccReg::Admin::ObjectNotFound)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

    DB db;
    if (!id)
        throw ccReg::Admin::ObjectNotFound();
    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Fred::Manager> r(
            Fred::Manager::create(&db,
                cfg.get<bool>("registry.restricted_handles"))
            );
    Fred::Domain::Manager *dm = r->getDomainManager();
    std::auto_ptr<Fred::Domain::List> dl(dm->createList());

    Database::Filters::Union uf;
    Database::Filters::Domain *df = new Database::Filters::DomainHistoryImpl();
    df->addKeySetId().setValue(Database::ID(id));
    uf.addFilter(df);
    dl->setLimit(limit);
    dl->reload(uf);

    ccReg::DomainDetails_var dlist = new ccReg::DomainDetails;
    dlist->length(dl->getCount());
    for (unsigned int i = 0; i < dl->getCount(); i++)
        fillDomain(&dlist[i], dl->getDomain(i));
    db.Disconnect();
    return dlist._retn();
}

ccReg::DomainDetails *
ccReg_Admin_i::getDomainsByKeySetHandle(const char *handle, CORBA::Long limit)
    throw (ccReg::Admin::ObjectNotFound)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Admin_i::getDomainsByKeySetHandle('%1%')")
                % handle);
    DB db;
    if (!handle || !*handle)
        throw ccReg::Admin::ObjectNotFound();
    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Fred::Manager> r(
            Fred::Manager::create(&db,
                cfg.get<bool>("registry.restricted_handles"))
            );
    Fred::Domain::Manager *dm = r->getDomainManager();
    std::auto_ptr<Fred::Domain::List> dl(dm->createList());

    Database::Filters::Union uf;
    Database::Filters::Domain *df = new Database::Filters::DomainHistoryImpl();

    df->addKeySet().addHandle().setValue(
            std::string(handle));
    uf.addFilter(df);
    dl->setLimit(limit);
    dl->reload(uf);
    
    ccReg::DomainDetails_var dlist = new ccReg::DomainDetails;
    dlist->length(dl->getCount());
    for (unsigned int i = 0; i < dl->getCount(); i++)
        fillDomain(&dlist[i], dl->getDomain(i));

    db.Disconnect();
    return dlist._retn();
}


ccReg::DomainDetails* ccReg_Admin_i::getDomainsByInverseKey(const char* key,
                                                            ccReg::DomainInvKeyType type,
                                                            CORBA::Long limit) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getDomainsByInverseKey('%1%', %2%, %3%)")
      % key % type % limit);

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Domain::Manager *dm = r->getDomainManager();
  std::auto_ptr<Fred::Domain::List> dl(dm->createList());
  switch (type) {
    case ccReg::DIKT_REGISTRANT:
      dl->setRegistrantHandleFilter(key);
      break;
    case ccReg::DIKT_ADMIN:
      dl->setAdminHandleFilter(key);
      break;
    case ccReg::DIKT_TEMP:
      dl->setTempHandleFilter(key);
      break;
    case ccReg::DIKT_NSSET:
      dl->setNSSetHandleFilter(key);
      break;
    case ccReg::DIKT_KEYSET:
      dl->setKeySetHandleFilter(key);
      break;
  }
  dl->setLimit(limit);
  dl->reload();
  ccReg::DomainDetails_var dlist = new ccReg::DomainDetails;
  dlist->length(dl->getCount());
  for (unsigned i=0; i<dl->getCount(); i++)
    fillDomain(&dlist[i], dl->getDomain(i));
  db.Disconnect();
  return dlist._retn();
}

ccReg::NSSetDetails* ccReg_Admin_i::getNSSetsByInverseKey(const char* key,
                                                          ccReg::NSSetInvKeyType type,
                                                          CORBA::Long limit) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Zone::Manager *zm = r->getZoneManager();
  Fred::NSSet::Manager *nm = r->getNSSetManager();
  std::auto_ptr<Fred::NSSet::List> nl(nm->createList());
  switch (type) {
    case ccReg::NIKT_NS : nl->setHostNameFilter(zm->encodeIDN(key)); break;
    case ccReg::NIKT_TECH : nl->setAdminFilter(key); break;
  }
  nl->setLimit(limit);
  nl->reload();
  ccReg::NSSetDetails_var nlist = new ccReg::NSSetDetails;
  nlist->length(nl->getCount());
  for (unsigned i=0; i<nl->getCount(); i++)
    fillNSSet(&nlist[i], nl->getNSSet(i));
  db.Disconnect();
  return nlist._retn();
}

ccReg::KeySetDetails *
ccReg_Admin_i::getKeySetsByInverseKey(
        const char *key,
        ccReg::KeySetInvKeyType type,
        CORBA::Long limit)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

    DB db;
    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Fred::Manager> r(
            Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
    Fred::KeySet::Manager *km = r->getKeySetManager();
    std::auto_ptr<Fred::KeySet::List> kl(km->createList());
    switch (type) {
        case ccReg::KIKT_TECH:
            kl->setAdminFilter(key);
            break;
    }
    kl->setLimit(limit);
    kl->reload();
    ccReg::KeySetDetails_var klist = new ccReg::KeySetDetails;
    klist->length(kl->getCount());
    for (unsigned int i = 0; i < kl->getCount(); i++)
        fillKeySet(&klist[i], kl->getKeySet(i));
    db.Disconnect();
    return klist._retn();
}

ccReg::KeySetDetails *
ccReg_Admin_i::getKeySetsByContactId(ccReg::TID id, CORBA::Long limit)
  throw (ccReg::Admin::ObjectNotFound)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

    DB db;
    if (!id)
        throw ccReg::Admin::ObjectNotFound();
    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Fred::Manager> r(
            Fred::Manager::create(&db,
                cfg.get<bool>("registry.restricted_handles"))
            );
    Fred::KeySet::Manager *km = r->getKeySetManager();
    std::auto_ptr<Fred::KeySet::List> kl(km->createList());

    Database::Filters::Union uf;
    Database::Filters::KeySet *kf = new Database::Filters::KeySetHistoryImpl();
    kf->addTechContact().addId().setValue(Database::ID(id));
    uf.addFilter(kf);
    kl->setLimit(limit);
    kl->reload(uf);

    ccReg::KeySetDetails_var klist = new ccReg::KeySetDetails;
    klist->length(kl->getCount());
    for (unsigned int i = 0; i < kl->getCount(); i++)
        fillKeySet(&klist[i], kl->getKeySet(i));
    db.Disconnect();
    return klist._retn();
}

ccReg::KeySetDetails *
ccReg_Admin_i::getKeySetsByContactHandle(const char *handle, CORBA::Long limit)
  throw (ccReg::Admin::ObjectNotFound)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

    DB db;
    if (!handle || !*handle)
        throw ccReg::Admin::ObjectNotFound();
    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Fred::Manager> r(
            Fred::Manager::create(&db,
                cfg.get<bool>("registry.restricted_handles"))
            );
    Fred::KeySet::Manager *km = r->getKeySetManager();
    std::auto_ptr<Fred::KeySet::List> kl(km->createList());

    Database::Filters::Union uf;
    Database::Filters::KeySet *kf = new Database::Filters::KeySetHistoryImpl();
    kf->addTechContact().addHandle().setValue(std::string(handle));
    uf.addFilter(kf);
    kl->setLimit(limit);
    kl->reload(uf);

    ccReg::KeySetDetails_var klist = new ccReg::KeySetDetails;
    klist->length(kl->getCount());
    for (unsigned int i = 0; i < kl->getCount(); i++)
        fillKeySet(&klist[i], kl->getKeySet(i));
    db.Disconnect();
    return klist._retn();
}


CORBA::Long ccReg_Admin_i::getDomainCount(const char *zone) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getDomainCount(zone);
  db.Disconnect();
  return ret;
}

CORBA::Long ccReg_Admin_i::getSignedDomainCount(const char *_fqdn)
{
  Logging::Context ctx(server_name_);

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db,cfg.get<bool>("registry.restricted_handles")));
  Fred::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getSignedDomainCount(_fqdn);
  db.Disconnect();
  return ret;
}

CORBA::Long ccReg_Admin_i::getEnumNumberCount() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getEnumNumberCount();
  db.Disconnect();
  return ret;
}

Registry::EPPActionTypeSeq* ccReg_Admin_i::getEPPActionTypeList() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  Fred::Registrar::Manager *rm = r->getRegistrarManager();
  Registry::EPPActionTypeSeq *et = new Registry::EPPActionTypeSeq;
  
  et->length(rm->getEPPActionTypeCount());
  for (unsigned i=0; i<rm->getEPPActionTypeCount(); i++) {
    (*et)[i].id = rm->getEPPActionTypeByIdx(i).id;
    (*et)[i].name = DUPSTRC(rm->getEPPActionTypeByIdx(i).name);
  }
  
  db.Disconnect();
  return et;
}

Registry::CountryDescSeq* ccReg_Admin_i::getCountryDescList() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Fred::Manager> r(Fred::Manager::create(&db, cfg.get<bool>("registry.restricted_handles")));
  /* 
   * TEMP: this is for loading country codes from database - until new database 
   * library is not fully integrated into registrar library 
   */ 
  r->dbManagerInit();
  
  Registry::CountryDescSeq *cd = new Registry::CountryDescSeq;
  cd->length(r->getCountryDescSize());
  for (unsigned i=0; i<r->getCountryDescSize(); i++) {
    (*cd)[i].cc = DUPSTRC(r->getCountryDescByIdx(i).cc);
    (*cd)[i].name = DUPSTRC(r->getCountryDescByIdx(i).name);
  }
  
  db.Disconnect();
  return cd;
}

char* ccReg_Admin_i::getDefaultCountry() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  return CORBA::string_dup("CZ");
}

Registry::ObjectStatusDescSeq* ccReg_Admin_i::getDomainStatusDescList(const char *lang) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  Registry::ObjectStatusDescSeq* o = new Registry::ObjectStatusDescSeq;
  for (unsigned i=0; i<registry_manager_->getStatusDescCount(); i++) {
    const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
    if (sd->getExternal() && sd->isForType(3)) {
      o->length(o->length()+1);
      try {
        (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
      } catch (...) {
        // unknown language
        (*o)[o->length()-1].name = CORBA::string_dup("");
      }
      (*o)[o->length()-1].id    = sd->getId();
      (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
    }
  }
  return o;
}

Registry::ObjectStatusDescSeq* ccReg_Admin_i::getContactStatusDescList(const char *lang) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  Registry::ObjectStatusDescSeq* o = new Registry::ObjectStatusDescSeq;
  for (unsigned i=0; i<registry_manager_->getStatusDescCount(); i++) {
    const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
    if (sd->getExternal() && sd->isForType(1)) {
      o->length(o->length()+1);
      try {
        (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
      } catch (...) {
        // unknown language
        (*o)[o->length()-1].name = CORBA::string_dup("");
      }
      (*o)[o->length()-1].id    = sd->getId();
      (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
    }
  }
  return o;
}

Registry::ObjectStatusDescSeq* ccReg_Admin_i::getNSSetStatusDescList(const char *lang) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  Registry::ObjectStatusDescSeq* o = new Registry::ObjectStatusDescSeq;
  for (unsigned i=0; i<registry_manager_->getStatusDescCount(); i++) {
    const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
    if (sd->getExternal() && sd->isForType(2)) {
      o->length(o->length()+1);
      try {
        (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
      } catch (...) {
        // unknown language
        (*o)[o->length()-1].name = CORBA::string_dup("");
      }
      (*o)[o->length()-1].id    = sd->getId();
      (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
    }
  }
  return o;
}

Registry::ObjectStatusDescSeq *
ccReg_Admin_i::getKeySetStatusDescList(const char *lang)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  Registry::ObjectStatusDescSeq *o = new Registry::ObjectStatusDescSeq;
    for (unsigned int i = 0; i < registry_manager_->getStatusDescCount(); i++) {
        const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
        if (sd->getExternal() && sd->isForType(4)) {
            o->length(o->length() + 1);
            try {
                (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
            }
            catch (...) {
                //unknown lang
                (*o)[o->length()-1].name = CORBA::string_dup("");
            }
            (*o)[o->length()-1].id    = sd->getId();
            (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
        }
    }
    return o;
}

Registry::ObjectStatusDescSeq *ccReg_Admin_i::getObjectStatusDescList(const char *lang) {

    try
    {
        Logging::Context ctx(server_name_);
        ConnectionReleaser releaser;

        Registry::ObjectStatusDescSeq_var o = new Registry::ObjectStatusDescSeq;
        const Registry::ObjectStatusDescSeq* optr = &(o.in());
        if(optr == 0)
        {
            LOGGER(PACKAGE).error("ccReg_Admin_i::getObjectStatusDescList error new Registry::ObjectStatusDescSeq failed ");
            throw ccReg::Admin::InternalServerError();
        }

        unsigned states_count = registry_manager_->getStatusDescCount();
        o->length(states_count);
        for (unsigned i = 0; i < states_count; ++i) {
        const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
        if(sd == 0)
        {
            LOGGER(PACKAGE).error((std::string(
                    "ccReg_Admin_i::getObjectStatusDescList error registry_manager_->getStatusDescByIdx(i) i: "
                    +boost::lexical_cast<std::string>(i))).c_str());
            throw ccReg::Admin::InternalServerError();
        }
        o[i].id    = sd->getId();
        o[i].shortName = DUPSTRFUN(sd->getName);
        o[i].name  = DUPSTRC(sd->getDesc(lang));
      }
      return o._retn();
    }//try
    catch(const std::exception& ex)
    {
        std::string msg = std::string("ccReg_Admin_i::getObjectStatusDescList ex: ")+ex.what();
        LOGGER(PACKAGE).error(msg.c_str());
        throw ccReg::Admin::InternalServerError();
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Admin_i::getObjectStatusDescList error ");
        throw ccReg::Admin::InternalServerError();
    }
}

char* ccReg_Admin_i::getCreditByZone(const char*registrarHandle, ccReg::TID zone) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  std::auto_ptr<Fred::Invoicing::Manager>
      invman(Fred::Invoicing::Manager::create());
  char *ret = DUPSTRC(formatMoney(invman->getCreditByZone(registrarHandle, zone)));
  return ret;
}

void ccReg_Admin_i::generateLetters() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DB ldb;
  try {
    ldb.OpenDatabase(m_connection_string.c_str());
    
    MailerManager mm(ns);
    std::auto_ptr<Fred::Document::Manager> docman(
        Fred::Document::Manager::create(cfg.get<std::string>("registry.docgen_path"),
                                            cfg.get<std::string>("registry.docgen_template_path"),
                                            cfg.get<std::string>("registry.fileclient_path"), 
                                            ns->getHostName()));
    std::auto_ptr<Fred::Zone::Manager> zoneMan(
        Fred::Zone::Manager::create());

    Fred::Messages::ManagerPtr msgMan
        = Fred::Messages::create_manager();


    std::auto_ptr<Fred::Domain::Manager> domMan(
        Fred::Domain::Manager::create(&ldb,
                                          zoneMan.get()));
    std::auto_ptr<Fred::Contact::Manager> conMan(
        Fred::Contact::Manager::create(&ldb,
                                           cfg.get<bool>("registry.restricted_handles")));
    std::auto_ptr<Fred::NSSet::Manager> nssMan(
        Fred::NSSet::Manager::create(&ldb,
                                         zoneMan.get(),
                                         cfg.get<bool>("registry.restricted_handles")));
    std::auto_ptr<Fred::KeySet::Manager> keyMan(
            Fred::KeySet::Manager::create(
                &ldb,
                cfg.get<bool>("registry.restricted_handles")));
    std::auto_ptr<Fred::Registrar::Manager> rMan(
        Fred::Registrar::Manager::create(&ldb));
    std::auto_ptr<Fred::Notify::Manager> notifyMan(
        Fred::Notify::Manager::create(&ldb,
                                          &mm,
                                          conMan.get(),
                                          nssMan.get(),
                                          keyMan.get(),
                                          domMan.get(), 
                                          docman.get(),
                                          rMan.get(),
                                          msgMan
                                          ));
    notifyMan->generateLetters(cfg.get<unsigned>("registry.docgen_domain_count_limit"));
    ldb.Disconnect();
  }
  catch (...) {
    ldb.Disconnect();
  }
}

bool
ccReg_Admin_i::setInZoneStatus(ccReg::TID domainId)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Admin_i::setInZoneStatus(%1%)")
            % domainId);
    Database::Query query;
    query.buffer()
        << "SELECT id FROM object_state_request WHERE object_id="
        << Database::Value(domainId) << " AND state_id=6 "
        << "AND (canceled ISNULL OR canceled > CURRENT_TIMESTAMP) "
        << "AND (valid_to ISNULL OR valid_to > CURRENT_TIMESTAMP)";
    // Database::Connection *conn = m_db_manager.acquire();
    Database::Connection conn = Database::Manager::acquire();
    try {
        Database::Result res = conn.exec(query);
        if (res.size() != 0) {
            LOGGER(PACKAGE).error("Already in ``object_state_request''");
            return false;
        }
    } catch (...) {
        LOGGER(PACKAGE).error("setInZoneStatus: an error has occured");
        return false;
    }
    Database::InsertQuery insert("object_state_request");
    Database::DateTime now = Database::NOW_UTC;
    insert.add("object_id", Database::Value(domainId));
    insert.add("state_id", 6);
    insert.add("valid_from", Database::Value(now));
    insert.add("valid_to", Database::Value(now + Database::Days(7)));
    insert.add("crdate", Database::Value(now));
    try {
        conn.exec(insert);
    } catch (...) {
        LOGGER(PACKAGE).error("setInZoneStatus: failed to insert");
        return false;
    }
    query.clear();
    query.buffer()
        << "SELECT update_object_states(" << domainId << ");";
    try {
        conn.exec(query);
    } catch (...) {
        LOGGER(PACKAGE).error("setInZoneStatus: failed to update object states");
        return false;
    }
    return true;
}

ccReg::TID ccReg_Admin_i::createPublicRequest(Registry::PublicRequest::Type _type,
                                              const char *_reason,
                                              const char *_email_to_answer,
                                              const ccReg::Admin::ObjectIdList& _object_ids,
                                              const ccReg::TID requestId) 
  throw (
    ccReg::Admin::BAD_EMAIL, ccReg::Admin::OBJECT_NOT_FOUND,
    ccReg::Admin::ACTION_NOT_FOUND, ccReg::Admin::SQL_ERROR,
    ccReg::Admin::INVALID_INPUT, ccReg::Admin::REQUEST_BLOCKED
  ) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::createPublicRequest(%1%, '%2%', '%3%', %4%, %5%)") %
        _type %  _reason % _email_to_answer % &_object_ids % requestId);
  
  MailerManager mailer_manager(ns);
  
  std::auto_ptr<Fred::Document::Manager> doc_manager(
          Fred::Document::Manager::create(
              cfg.get<std::string>("registry.docgen_path"),
              cfg.get<std::string>("registry.docgen_template_path"),
              cfg.get<std::string>("registry.fileclient_path"),
              ns->getHostName())
          );
  std::auto_ptr<Fred::PublicRequest::Manager> request_manager(
          Fred::PublicRequest::Manager::create(
              registry_manager_->getDomainManager(),
              registry_manager_->getContactManager(),
              registry_manager_->getNSSetManager(),
              registry_manager_->getKeySetManager(),
              &mailer_manager,
              doc_manager.get(),
              registry_manager_->getMessageManager())
          );
  
#define REQUEST_TYPE_CORBA2DB_CASE(type)            \
  case Registry::PublicRequest::type:                  \
    request_type = Fred::PublicRequest::type; break;
  
  Fred::PublicRequest::Type request_type;
  switch (_type) {
    REQUEST_TYPE_CORBA2DB_CASE(PRT_AUTHINFO_AUTO_RIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_AUTHINFO_AUTO_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_AUTHINFO_EMAIL_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_AUTHINFO_POST_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_BLOCK_CHANGES_EMAIL_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_BLOCK_CHANGES_POST_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_BLOCK_TRANSFER_EMAIL_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_BLOCK_TRANSFER_POST_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_UNBLOCK_CHANGES_EMAIL_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_UNBLOCK_CHANGES_POST_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_UNBLOCK_TRANSFER_EMAIL_PIF)
    REQUEST_TYPE_CORBA2DB_CASE(PRT_UNBLOCK_TRANSFER_POST_PIF)
    default:
      LOGGER(PACKAGE).error(boost::format("can't create new public request - unknown request type (%1%)")
                                          % _type);
      throw ccReg::Admin::INVALID_INPUT();
  }
  
  std::auto_ptr<Fred::PublicRequest::PublicRequest> new_request(request_manager->createRequest(request_type));
  new_request->setType(request_type);
  new_request->setRegistrarId(0);
  new_request->setReason(_reason);
  new_request->setEmailToAnswer(_email_to_answer);
  new_request->setRequestId(requestId);
  for (unsigned i=0; i<_object_ids.length(); i++)
    new_request->addObject(Fred::PublicRequest::OID(_object_ids[i]));
  try {
    if (!new_request->check()) throw ccReg::Admin::REQUEST_BLOCKED();
    new_request->save();
    return new_request->getId();
  }
  catch (ccReg::Admin::REQUEST_BLOCKED) {
    throw;
  }
  catch (...) {
    throw ccReg::Admin::SQL_ERROR();
  }
}

void ccReg_Admin_i::processPublicRequest(ccReg::TID id, CORBA::Boolean invalid) 
    throw (
    ccReg::Admin::SQL_ERROR, ccReg::Admin::OBJECT_NOT_FOUND, 
    ccReg::Admin::MAILER_ERROR, ccReg::Admin::REQUEST_BLOCKED
  ) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::processPublicRequest(%1%, %2%)") %
  	id % invalid);

  MailerManager mailer_manager(ns);  
  std::auto_ptr<Fred::Document::Manager> doc_manager(
          Fred::Document::Manager::create(
              cfg.get<std::string>("registry.docgen_path"),
              cfg.get<std::string>("registry.docgen_template_path"),
              cfg.get<std::string>("registry.fileclient_path"),
              ns->getHostName())
          );
  std::auto_ptr<Fred::PublicRequest::Manager> request_manager(
          Fred::PublicRequest::Manager::create(
              registry_manager_->getDomainManager(),
              registry_manager_->getContactManager(),
              registry_manager_->getNSSetManager(),
              registry_manager_->getKeySetManager(),
              &mailer_manager,
              doc_manager.get(),
              registry_manager_->getMessageManager())
          );
  try {
    request_manager->processRequest(id,invalid,true);
  }
  catch (Fred::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
  catch (Fred::NOT_FOUND) {
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }
  catch (Fred::Mailer::NOT_SEND) {
    throw ccReg::Admin::MAILER_ERROR();
  }
  catch (Fred::PublicRequest::REQUEST_BLOCKED) {
    throw ccReg::Admin::REQUEST_BLOCKED();
  }
  catch (...) {
    /* this also catches when try to process request which
     * need to be authenticated */
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }
}

ccReg::Admin::Buffer* ccReg_Admin_i::getPublicRequestPDF(ccReg::TID id,
                                                         const char *lang) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getPublicRequestPDF(%1%, '%2%')") %
        id % lang);

  MailerManager mailer_manager(ns);
   
  std::auto_ptr<Fred::Document::Manager> doc_manager(
          Fred::Document::Manager::create(
              cfg.get<std::string>("registry.docgen_path"),
              cfg.get<std::string>("registry.docgen_template_path"),
              cfg.get<std::string>("registry.fileclient_path"),
              ns->getHostName())
          );
  std::auto_ptr<Fred::PublicRequest::Manager> request_manager(
          Fred::PublicRequest::Manager::create(
              registry_manager_->getDomainManager(),
              registry_manager_->getContactManager(),
              registry_manager_->getNSSetManager(),
              registry_manager_->getKeySetManager(),
              &mailer_manager,
              doc_manager.get(),
              registry_manager_->getMessageManager())
          );
  DB db;
  try {
    std::stringstream outstr;
    request_manager-> getPdf(id,lang,outstr);
    unsigned long size = outstr.str().size();
    CORBA::Octet *b = ccReg::Admin::Buffer::allocbuf(size);
    memcpy(b,outstr.str().c_str(),size);
    ccReg::Admin::Buffer* output = new ccReg::Admin::Buffer(size, size, b, 1);
    LOGGER(PACKAGE).info(boost::format("Retrieved pdf file for request id=%1% lang='%2%'") %
                         id % lang);
    return output;
  }
  catch (Fred::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
  catch (Fred::NOT_FOUND) {
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }
}

/* enum dictionary method implementation */

/* helper method - query construct */
std::string ccReg_Admin_i::_createQueryForEnumDomainsByRegistrant(const std::string &select_part,
        const std::string &name, bool by_person, bool by_org)
{
  std::string query = "";

  std::string from_part = "domain d JOIN zone z ON (z.id = d.zone) " \
              "JOIN enumval ev ON (d.id = ev.domainid) "             \
              "JOIN object_registry oreg ON (oreg.id = d.id) "       \
              "JOIN contact c ON (c.id = d.registrant)";

  std::string where_part = "";
  /* escape default wildcard chars % and _ in given name (we want to handle them as 
   * normal characters), then * and ? are * translated to this wildcards (% and _)
   * (XXX maybe some easier way? */
  std::string namecopy = name;
  std::string esc = "\\";
  std::string::size_type i = 0;
  while ((i = namecopy.find_first_of("%_", i)) != std::string::npos) {
    namecopy.replace(i, 1, esc + namecopy[i]);
    i += esc.size() + 1;
  }
  std::string translate = "TRANSLATE(E'%1%', '*?', '%%_')";
  /* escape handle */
  Database::Connection conn = Database::Manager::acquire();
  std::string ename = conn.escape(namecopy);

  where_part = "ev.publish='t' AND z.enum_zone='t'";
  if (by_person && by_org) {
    where_part += str(boost::format(" AND COALESCE(c.organization, c.name) ILIKE " + translate)
                    % ename);
  }
  else if (by_person) {
    where_part += str(boost::format(" AND c.name ILIKE " + translate + " AND c.organization IS NULL")
                    % ename);
  }
  else if (by_org) {
    where_part += str(boost::format(" AND c.organization ILIKE " + translate)
                    % ename);
  }
  return str(boost::format("SELECT %1% FROM %2% WHERE %3%")
                           % select_part % from_part % where_part);
}


/* IDL method */
::CORBA::ULongLong ccReg_Admin_i::countEnumDomainsByRegistrant(const char* name,
        ::CORBA::Boolean by_person, ::CORBA::Boolean by_org)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  try {
    if (!by_person && !by_org) {
        return 0;
    }

    std::string select_part = "count(*)";

    std::string count_query = _createQueryForEnumDomainsByRegistrant(select_part,
            name, by_person, by_org);

    Database::Connection conn = Database::Manager::acquire();
    Result count_result  = conn.exec(count_query);

    return static_cast<unsigned long long>(count_result[0][0]);
  }
  catch (Database::Exception &ex) {
      LOGGER(PACKAGE).error(boost::format("Database problem: %1%") % ex.what());
      throw ccReg::Admin::InternalServerError();
  }
  catch (std::exception &ex) {
    LOGGER(PACKAGE).error(boost::format("Internal error: %1%") % ex.what());
    throw ccReg::Admin::InternalServerError();

  }
  catch (...) {
    throw ccReg::Admin::InternalServerError();
  }
}


/* IDL method */
ccReg::EnumDictList* ccReg_Admin_i::getEnumDomainsByRegistrant(const char* name,
        ::CORBA::Boolean by_person, ::CORBA::Boolean by_org,
        ::CORBA::Long offset, ::CORBA::Long limit)
{
  Logging::Context(server_name_);
  ConnectionReleaser releaser;

  try {
    ccReg::EnumDictList_var data = new ccReg::EnumDictList();
    if (!by_person && !by_org) {
        return data._retn();
    }

    std::string select_part = "COALESCE(c.organization, c.name) AS holder, "    \
              "TRIM(COALESCE(c.street1, '')), TRIM(COALESCE(c.street2, '')), "  \
              "TRIM(COALESCE(c.street3, '')), TRIM(COALESCE(c.city, '')), "     \
              "TRIM(COALESCE(c.postalcode, '')), "                              \
              "TRIM(COALESCE(c.stateorprovince, '')), "                         \
              "TRIM(COALESCE(c.country, '')), oreg.name AS domain";

    std::string data_query = _createQueryForEnumDomainsByRegistrant(select_part,
            name, by_person, by_org);
    data_query += str(boost::format(" ORDER BY holder OFFSET %1% LIMIT %2%") % offset % limit);
    /* exec query */
    Database::Connection conn = Database::Manager::acquire();
    Result data_result  = conn.exec(data_query);

    /* fill result data */
    unsigned int size = data_result.size();
    data->length(size);
    for (unsigned int i = 0; i < size; i++) {
      ccReg::TAddress addr;
      addr.street1    = CORBA::string_dup(static_cast<std::string>(data_result[i][1]).c_str());
      addr.street2    = CORBA::string_dup(static_cast<std::string>(data_result[i][2]).c_str());
      addr.street3    = CORBA::string_dup(static_cast<std::string>(data_result[i][3]).c_str());
      addr.city       = CORBA::string_dup(static_cast<std::string>(data_result[i][4]).c_str());
      addr.postalcode = CORBA::string_dup(static_cast<std::string>(data_result[i][5]).c_str());
      addr.province   = CORBA::string_dup(static_cast<std::string>(data_result[i][6]).c_str());
      addr.country    = CORBA::string_dup(static_cast<std::string>(data_result[i][7]).c_str());

      data[i].name    = CORBA::string_dup(((std::string)data_result[i][0]).c_str());
      data[i].address = addr;
      data[i].domain  = CORBA::string_dup(((std::string)data_result[i][8]).c_str());
    }
    return data._retn();
  }
  catch (Database::Exception &ex) {
      LOGGER(PACKAGE).error(boost::format("Database problem: %1%") % ex.what());
      throw ccReg::Admin::InternalServerError();
  }
  catch (std::exception &ex) {
    LOGGER(PACKAGE).error(boost::format("Internal error: %1%") % ex.what());
    throw ccReg::Admin::InternalServerError();

  }
  catch (...) {
    throw ccReg::Admin::InternalServerError();
  }
}


/* IDL method */
ccReg::EnumDictList* ccReg_Admin_i::getEnumDomainsRecentEntries(::CORBA::Long count)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  try {
    boost::format data_query;
    data_query = boost::format("SELECT COALESCE(c.organization, c.name) AS holder, " \
                "TRIM(COALESCE(c.street1, '')), TRIM(COALESCE(c.street2, '')), "     \
                "TRIM(COALESCE(c.street3, '')), TRIM(COALESCE(c.city, '')), "        \
                "TRIM(COALESCE(c.postalcode, '')), "                                 \
                "TRIM(COALESCE(c.stateorprovince, '')), "                            \
                "TRIM(COALESCE(c.country, '')), oreg.name AS domain "                \
                "FROM domain d JOIN zone z ON (z.id = d.zone) "                      \
                "JOIN enumval ev ON (d.id = ev.domainid) "                           \
                "JOIN object_registry oreg ON (oreg.id = d.id) "                     \
                "JOIN contact c ON (c.id = d.registrant) "                           \
                "WHERE z.enum_zone = 't' AND ev.publish = 't' "                      \
                "ORDER BY oreg.crdate DESC LIMIT %1%")
                 % count;

    /* execute query */
    Database::Connection conn = Database::Manager::acquire();
    Result data_result = conn.exec(data_query.str());

    /* fill result data */
    ccReg::EnumDictList_var data = new ccReg::EnumDictList();

	unsigned int size = data_result.size();
	data->length(size);
	for(unsigned i=0; i<size; i++) {
      ccReg::TAddress addr;
      addr.street1    = CORBA::string_dup(static_cast<std::string>(data_result[i][1]).c_str());
      addr.street2    = CORBA::string_dup(static_cast<std::string>(data_result[i][2]).c_str());
      addr.street3    = CORBA::string_dup(static_cast<std::string>(data_result[i][3]).c_str());
      addr.city       = CORBA::string_dup(static_cast<std::string>(data_result[i][4]).c_str());
      addr.postalcode = CORBA::string_dup(static_cast<std::string>(data_result[i][5]).c_str());
      addr.province   = CORBA::string_dup(static_cast<std::string>(data_result[i][6]).c_str());
      addr.country    = CORBA::string_dup(static_cast<std::string>(data_result[i][7]).c_str());

      data[i].name    = CORBA::string_dup(((std::string)data_result[i][0]).c_str());
      data[i].address = addr;
      data[i].domain  = CORBA::string_dup(((std::string)data_result[i][8]).c_str());
    }
    return data._retn();
  }
  catch (Database::Exception &ex) {
      LOGGER(PACKAGE).error(boost::format("Database problem: %1%") % ex.what());
      throw ccReg::Admin::InternalServerError();
  }
  catch (std::exception &ex) {
    LOGGER(PACKAGE).error(boost::format("Internal error: %1%") % ex.what());
    throw ccReg::Admin::InternalServerError();

  }
  catch (...) {
    throw ccReg::Admin::InternalServerError();
  }
}

Registry::Registrar::Certification::Manager_ptr ccReg_Admin_i::getCertificationManager()
{
    Logging::Context ctx(server_name_);
    return Registry::Registrar::Certification::Manager::_duplicate(reg_cert_mgr_ref_);
}
Registry::Registrar::Group::Manager_ptr ccReg_Admin_i::getGroupManager()
{
    Logging::Context ctx(server_name_);
    return  Registry::Registrar::Group::Manager::_duplicate(reg_grp_mgr_ref_);
}

Registry_Registrar_Certification_Manager_i::Registry_Registrar_Certification_Manager_i()
{}
Registry_Registrar_Certification_Manager_i::~Registry_Registrar_Certification_Manager_i()
{}

//   Methods corresponding to IDL attributes and operations
ccReg::TID Registry_Registrar_Certification_Manager_i::createCertification(
        ccReg::TID reg_id
        , const ccReg::DateType& from
        , const ccReg::DateType& to
        , ::CORBA::Short score
        , ccReg::TID evaluation_file_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        if((score < 0) || (score > 5))
            throw std::runtime_error("Invalid value of score");
        if(evaluation_file_id < 1)
            throw std::runtime_error("Invalid value of evaluation_file_id");
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///create registrar certification
        return regman->createRegistrarCertification(
                reg_id
                , Database::Date(makeBoostDate(from))
                , Database::Date(makeBoostDate(to))
                , static_cast<Fred::Registrar::RegCertClass>(score)
                , evaluation_file_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//createCertification

void Registry_Registrar_Certification_Manager_i::shortenCertification(
        ccReg::TID cert_id
        , const ccReg::DateType& to)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///shorten registrar certification
        return regman->shortenRegistrarCertification(
                cert_id
                , Database::Date(makeBoostDate(to)));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//shortenCertification

void Registry_Registrar_Certification_Manager_i::updateCertification(
        ccReg::TID cert_id
        , ::CORBA::Short score
        , ccReg::TID evaluation_file_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        if((score < 0) || (score > 5))
            throw std::runtime_error("Invalid value of score");
        if(evaluation_file_id < 1)
            throw std::runtime_error("Invalid value of evaluation_file_id");

        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///update registrar certification
        return regman->updateRegistrarCertification(
                cert_id
                , static_cast<Fred::Registrar::RegCertClass>(score)
                , evaluation_file_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//updateCertification

Registry::Registrar::Certification::CertificationList*
Registry_Registrar_Certification_Manager_i::getCertificationsByRegistrar(
        ccReg::TID registrar_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///get registrar certification
        Fred::Registrar::CertificationSeq cs
        = regman->getRegistrarCertifications(registrar_id);

        std::auto_ptr<Registry::Registrar::Certification::CertificationList> cl
         (new Registry::Registrar::Certification::CertificationList);
        cl->length(cs.size());

        for(unsigned i = 0;i < cs.size(); ++i)
        {
            (*cl)[i].id = cs[i].id;
            (*cl)[i].fromDate = makeCorbaDate(cs[i].valid_from);
            (*cl)[i].toDate = makeCorbaDate(cs[i].valid_until);
            (*cl)[i].score = cs[i].classification;
            (*cl)[i].evaluation_file_id = cs[i].eval_file_id;
        }

        return cl.release();
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//getCertificationsByRegistrar

Registry_Registrar_Group_Manager_i::Registry_Registrar_Group_Manager_i()
{}
Registry_Registrar_Group_Manager_i::~Registry_Registrar_Group_Manager_i()
{}
//   Methods corresponding to IDL attributes and operations
ccReg::TID Registry_Registrar_Group_Manager_i::createGroup(const char* name)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///create group
        return regman->createRegistrarGroup(std::string(name));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//createGroup

void Registry_Registrar_Group_Manager_i::deleteGroup(ccReg::TID group_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///delete group
        regman->cancelRegistrarGroup(group_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//deleteGroup

void Registry_Registrar_Group_Manager_i::updateGroup(
        ccReg::TID group_id
        , const char* name)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///update group
        regman->updateRegistrarGroup(group_id, std::string(name));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//updateGroup

ccReg::TID Registry_Registrar_Group_Manager_i::addRegistrarToGroup(
        ccReg::TID reg_id
        , ccReg::TID group_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///create membership of registrar in group
        return regman->createRegistrarGroupMembership(
                reg_id
                , group_id
                , Database::Date(NOW)
                , Database::Date(POS_INF));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//addRegistrarToGroup

void Registry_Registrar_Group_Manager_i::removeRegistrarFromGroup(
        ccReg::TID reg_id
        , ccReg::TID group_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///end membership of registrar in group
        regman->endRegistrarGroupMembership(
                reg_id
                , group_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}

Registry::Registrar::Group::GroupList*
Registry_Registrar_Group_Manager_i::getGroups()
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///get registrar certification
        Fred::Registrar::GroupSeq gs
        = regman->getRegistrarGroups();

        std::auto_ptr<Registry::Registrar::Group::GroupList> gl
         (new Registry::Registrar::Group::GroupList);
        gl->length(gs.size());

        for(unsigned i = 0;i < gs.size(); ++i)
        {
            (*gl)[i].id = gs[i].id;
            (*gl)[i].name = CORBA::string_dup(gs[i].name.c_str());
            (*gl)[i].cancelled = makeCorbaTime(gs[i].cancelled);
        }

        return gl.release();
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw;// Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//getGroups


Registry::Registrar::Group::MembershipByRegistrarList*
Registry_Registrar_Group_Manager_i::getMembershipsByRegistar(
        ccReg::TID registrar_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///get registrar certification
        Fred::Registrar::MembershipByRegistrarSeq mbrs
        = regman->getMembershipByRegistrar(registrar_id);

        std::auto_ptr<Registry::Registrar::Group::MembershipByRegistrarList> mbrl
         (new Registry::Registrar::Group::MembershipByRegistrarList);
        mbrl->length(mbrs.size());

        for(unsigned i = 0;i < mbrs.size(); ++i)
        {
            (*mbrl)[i].id = mbrs[i].id;
            (*mbrl)[i].group_id = mbrs[i].group_id;
            (*mbrl)[i].fromDate = makeCorbaDate(mbrs[i].member_from);
            (*mbrl)[i].toDate = makeCorbaDate(mbrs[i].member_until);
        }

        return mbrl.release();
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//getMembershipsByRegistar

Registry::Registrar::Group::MembershipByGroupList*
Registry_Registrar_Group_Manager_i::getMembershipsByGroup(ccReg::TID group_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        Fred::Registrar::Manager::AutoPtr regman(
                Fred::Registrar::Manager::create(0));
        ///get registrar certification
        Fred::Registrar::MembershipByGroupSeq mbgs
        = regman->getMembershipByGroup(group_id);

        std::auto_ptr<Registry::Registrar::Group::MembershipByGroupList> mbgl
         (new Registry::Registrar::Group::MembershipByGroupList);
        mbgl->length(mbgs.size());

        for(unsigned i = 0;i < mbgs.size(); ++i)
        {
            (*mbgl)[i].id = mbgs[i].id;
            (*mbgl)[i].registrar_id = mbgs[i].registrar_id;
            (*mbgl)[i].fromDate = makeCorbaDate(mbgs[i].member_from);
            (*mbgl)[i].toDate = makeCorbaDate(mbgs[i].member_until);
        }

        return mbgl.release();
    }//try
    catch(const std::exception & ex)
    {
        LOGGER(PACKAGE).debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER(PACKAGE).debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//getMembershipsByGroup


ccReg::EnumList* ccReg_Admin_i::getBankAccounts()
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  LOGGER(PACKAGE).debug("ccReg_Admin_i::getBankAccounts");
  try
  {
      Fred::Banking::EnumList el = Fred::Banking::getBankAccounts();
      ccReg::EnumList_var ret = new ccReg::EnumList;
      ret->length(el.size());
      for(std::size_t i = 0; i < el.size(); ++i)
      {
          ret[i].id = el[i].id;
          ret[i].name = CORBA::string_dup(el[i].name.c_str());
      }
      return ret._retn();
  }//try
  catch(std::exception& ex)
  {
      throw ccReg::ErrorReport(ex.what());
  }
  catch(...)
  {
      throw ccReg::ErrorReport("unknown exception");
  }
}//ccReg_Admin_i::getBankAccounts

