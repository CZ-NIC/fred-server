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
#include "register/register.h"
#include "register/notify.h"
#include "corba/mailer_manager.h"

#include "log/logger.h"

ccReg_Admin_i::ccReg_Admin_i(const std::string _database,
                             NameService *_ns,
                             Conf& _cfg,
                             bool _session_garbage) throw (DB_CONNECT_FAILED) :
  m_connection_string(_database), ns(_ns), cfg(_cfg) {
  // these object are shared between threads (CAUTION)
  if (!db.OpenDatabase(m_connection_string.c_str())) {
    LOG(ALERT_LOG,
        "can not connect to DATABASE %s",
        m_connection_string.c_str());
    throw DB_CONNECT_FAILED();
  }
  m_db_manager.reset(new DBase::PSQLManager(m_connection_string));
  register_manager_.reset(Register::Manager::create(&db, cfg.GetRestrictedHandles()));
  register_manager_->initStates();

  m_user_list.push_back("superuser");
  m_user_list.push_back("martin");
  m_user_list.push_back("pavel");
  m_user_list.push_back("jara");
  m_user_list.push_back("zuzka");
  m_user_list.push_back("david");
  m_user_list.push_back("feela");
  m_user_list.push_back("ondrej");
  m_user_list.push_back("tdivis");
  m_user_list.push_back("jsadek");
  m_user_list.push_back("helpdesk");
  
  if (_session_garbage) {
    session_garbage_active_ = true;
    session_garbage_thread_ = new boost::thread(boost::bind(&ccReg_Admin_i::garbageSession, this));
  }
}

ccReg_Admin_i::~ccReg_Admin_i() {
  TRACE("[CALL] ccReg_Admin_i::~ccReg_Admin_i()");
  db.Disconnect();

  session_garbage_active_ = false;
  // session_garbage_thread_->join();
  delete session_garbage_thread_;
  
  /// sessions cleanup
  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::iterator it = m_session_list.begin();
  while (it != m_session_list.end()) {
    std::string session_id = it->first;
    delete it->second;
    m_session_list.erase(it++);
    LOGGER("corba").debug(boost::format("session '%1%' destroyed") % session_id);
  }
}

#define SWITCH_CONVERT(x) case Register::x : ch->handleClass = ccReg::x; break
#define SWITCH_CONVERT_T(x) case Register::x : ch->hType = ccReg::x; break
void ccReg_Admin_i::checkHandle(const char* handle,
                                ccReg::CheckHandleTypeSeq_out chso) {
  DB ldb;
  ldb.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&ldb,cfg.GetRestrictedHandles()));
  ccReg::CheckHandleTypeSeq* chs = new ccReg::CheckHandleTypeSeq;
  Register::CheckHandleList chl;
  r->checkHandle(handle, chl, true); // allow IDN in whois queries
  chs->length(chl.size());
  for (unsigned i=0; i< chl.size(); i++) {
    ccReg::CheckHandleType *ch = &(*chs)[i];
    Register::CheckHandle& chd = chl[i];
    ch->newHandle = CORBA::string_dup(chd.newHandle.c_str());
    ch->conflictHandle = CORBA::string_dup(chd.conflictHandle.c_str());
    switch (chd.type) {
      SWITCH_CONVERT_T(HT_ENUM_NUMBER);
      SWITCH_CONVERT_T(HT_ENUM_DOMAIN);
      SWITCH_CONVERT_T(HT_DOMAIN);
      SWITCH_CONVERT_T(HT_CONTACT);
      SWITCH_CONVERT_T(HT_NSSET);
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
  LOGGER("corba").debug("session garbage thread started...");
  while (session_garbage_active_) {
    LOGGER("corba").debug("session garbage procedure sleeped");
    
    /// TODO: thread sleep interval should be in configuration
    boost::xtime sleep_time;
    boost::xtime_get(&sleep_time, boost::TIME_UTC);
    sleep_time.sec += 300;
    boost::thread::sleep(sleep_time);
    
    TRACE("[CALL] ccReg_Admin_i::garbageSession()");
    LOGGER("corba").debug("session garbage procedure invoked");
    boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
    bool found;
    do {
      found = false;
      SessionListType::iterator it = m_session_list.begin();
      for (; it != m_session_list.end() && !it->second->isTimeouted(); ++it);
      if (it != m_session_list.end()) {
        LOGGER("corba").debug(boost::format("admin session '%1%' deleted -- remains '%2%' session(s)")
            % it->first % (m_session_list.size() - 1));
        delete it->second;
        m_session_list.erase(it);
        found = true;
      }
    } while (found);
  }
  LOGGER("corba").debug("session garbage thread stopped");
}

void ccReg_Admin_i::authenticateUser(const char* _username,
                                     const char* _password)
    throw (ccReg::Admin::AuthFailed) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::authenticateUser('%1%', '%2%')")
      % _username % _password);

  if (std::string(_password) != "superuser123")
    throw ccReg::Admin::AuthFailed();

  std::vector<std::string>::const_iterator i = find(m_user_list.begin(),
                                                    m_user_list.end(),
                                                    _username);
  if (i == m_user_list.end())
    throw ccReg::Admin::AuthFailed();
}

char* ccReg_Admin_i::createSession(const char* username)
    throw (ccReg::Admin::AuthFailed) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::createSession('%1%')") % username);

  std::vector<std::string>::const_iterator i = find(m_user_list.begin(),
                                                    m_user_list.end(),
                                                    username);
  if (i == m_user_list.end())
    throw ccReg::Admin::AuthFailed();

  // garbageSession();

  ccReg_User_i *user_info = new ccReg_User_i(i - m_user_list.begin(), username, username, username);

  std::string session_id = to_iso_string(microsec_clock::local_time()) + "/"
      + username;

  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  
  ccReg_Session_i *session = new ccReg_Session_i(m_connection_string, ns, cfg, user_info);
  m_session_list[session_id] = session; 
  LOGGER("corba").notice(boost::format("admin session '%1%' created -- total number of sessions is '%2%'")
      % session_id % m_session_list.size());

  return CORBA::string_dup(session_id.c_str());
}

void ccReg_Admin_i::destroySession(const char* _session_id) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::destroySession('%1%')") % _session_id);
  
  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::iterator it = m_session_list.find(_session_id);
  if (it == m_session_list.end()) {
    LOGGER("corba").debug(boost::format("session '%1%' not found -- already destroyed")
        % _session_id);
    return;
  }

  delete it->second;
  m_session_list.erase(it);
}

ccReg::Session_ptr ccReg_Admin_i::getSession(const char* _session_id)
    throw (ccReg::Admin::ObjectNotFound) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::getSession('%1%')") % _session_id);

  // garbageSession();

  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::const_iterator it = m_session_list.find(_session_id);
  if (it == m_session_list.end()) {
    LOGGER("corba").debug(boost::format("session '%1%' not found -- deleted due to timout")
        % _session_id);
    throw ccReg::Admin::ObjectNotFound();
  }

  it->second->updateActivity();
  return it->second->_this();
}

void ccReg_Admin_i::fillRegistrar(ccReg::Registrar& creg,
                                  Register::Registrar::Registrar *reg) {
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
  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try { 
    std::auto_ptr<Register::Manager> regm(
        Register::Manager::create(&ldb,cfg.GetRestrictedHandles())
    );
    Register::Registrar::Manager *rm = regm->getRegistrarManager();
    Register::Registrar::RegistrarList *rl = rm->getList();
    rl->reload();
    LOG( NOTICE_LOG, "getRegistrars: num -> %d", rl->size() );
    ccReg::RegistrarList* reglist = new ccReg::RegistrarList;
    reglist->length(rl->size());
    for (unsigned i=0; i<rl->size(); i++)
    fillRegistrar((*reglist)[i],rl->get(i));
    ldb.Disconnect();
    return reglist;
  }
  catch (Register::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

ccReg::RegistrarList* ccReg_Admin_i::getRegistrarsByZone(const char *zone)
    throw (ccReg::Admin::SQL_ERROR) {
  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try {
    std::auto_ptr<Register::Manager> regm(
        Register::Manager::create(&ldb,cfg.GetRestrictedHandles())
    );
    Register::Registrar::Manager *rm = regm->getRegistrarManager();
    Register::Registrar::RegistrarList *rl = rm->getList();
    rl->setZoneFilter(zone);
    rl->reload();
    LOG( NOTICE_LOG, "getRegistrars: num -> %d", rl->size() );
    ccReg::RegistrarList* reglist = new ccReg::RegistrarList;
    reglist->length(rl->size());
    for (unsigned i=0; i<rl->size(); i++)
    fillRegistrar((*reglist)[i],rl->get(i));
    ldb.Disconnect();
    return reglist;
  }
  catch (Register::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

ccReg::Registrar* ccReg_Admin_i::getRegistrarById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound, ccReg::Admin::SQL_ERROR) {
  LOG( NOTICE_LOG, "getRegistarByHandle: id -> %lld", (unsigned long long)id );
  if (!id) throw ccReg::Admin::ObjectNotFound();
  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try {
    std::auto_ptr<Register::Manager> regm(
        Register::Manager::create(&ldb,cfg.GetRestrictedHandles())
    );
    Register::Registrar::Manager *rm = regm->getRegistrarManager();
    Register::Registrar::RegistrarList *rl = rm->getList();
    rl->setIdFilter(id);
    rl->reload();
    if (rl->size() < 1) {
      ldb.Disconnect();
      throw ccReg::Admin::ObjectNotFound();
    }
    ccReg::Registrar* creg = new ccReg::Registrar;
    fillRegistrar(*creg,rl->get(0));
    ldb.Disconnect();
    return creg;
  }
  catch (Register::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

ccReg::Registrar* ccReg_Admin_i::getRegistrarByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound, ccReg::Admin::SQL_ERROR) {
  LOG( NOTICE_LOG, "getRegistarByHandle: handle -> %s", handle );
  if (!handle || !*handle) throw ccReg::Admin::ObjectNotFound();
  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try {
    std::auto_ptr<Register::Manager> regm(
        Register::Manager::create(&ldb,cfg.GetRestrictedHandles())
    );
    Register::Registrar::Manager *rm = regm->getRegistrarManager();
    Register::Registrar::RegistrarList *rl = rm->getList();
    rl->setHandleFilter(handle);
    rl->reload();
    if (rl->size() < 1) {
      ldb.Disconnect();
      throw ccReg::Admin::ObjectNotFound();
    }
    ccReg::Registrar* creg = new ccReg::Registrar;
    fillRegistrar(*creg,rl->get(0));
    ldb.Disconnect();
    return creg;
  }
  catch (Register::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

void ccReg_Admin_i::putRegistrar(const ccReg::Registrar& regData) {
  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  Register::Registrar::RegistrarList *rl = rm->getList();
  Register::Registrar::Registrar *reg; // registrar to be created or updated
  if (!regData.id)
    reg = rl->create();
  else {
    rl->setIdFilter(regData.id);
    rl->reload();
    if (rl->size() != 1) {
      db.Disconnect();
      throw ccReg::Admin::UpdateFailed();
    }
    reg = rl->get(0);
  }
  reg->setHandle((const char *)regData.handle);
  reg->setURL((const char *)regData.url);
  reg->setName((const char *)regData.name);
  reg->setOrganization((const char *)regData.organization);
  reg->setStreet1((const char *)regData.street1);
  reg->setStreet2((const char *)regData.street2);
  reg->setStreet3((const char *)regData.street3);
  reg->setCity((const char *)regData.city);
  reg->setProvince((const char *)regData.stateorprovince);
  reg->setPostalCode((const char *)regData.postalcode);
  reg->setCountry((const char *)regData.country);
  reg->setTelephone((const char *)regData.telephone);
  reg->setFax((const char *)regData.fax);
  reg->setEmail((const char *)regData.email);
  reg->clearACLList();
  for (unsigned i=0; i<regData.access.length(); i++) {
    Register::Registrar::ACL *acl = reg->newACL();
    acl->setCertificateMD5((const char *)regData.access[i].md5Cert);
    acl->setPassword((const char *)regData.access[i].password);
  }
  try {
    reg->save();
    db.Disconnect();
  } catch (...) {
    db.Disconnect();
    throw ccReg::Admin::UpdateFailed();
  }
}

void ccReg_Admin_i::fillContact(ccReg::ContactDetail* cc,
                                Register::Contact::Contact* c) {
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
    if (register_manager_->getStatusDesc(
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
  TRACE(boost::format("[CALL] ccReg_Admin_i::getContactByHandle('%1%')") % handle);

  DB db;
  if (!handle || !*handle)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Contact::Manager *cr = r->getContactManager();
  std::auto_ptr<Register::Contact::List> cl(cr->createList());
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
  TRACE(boost::format("[CALL] ccReg_Admin_i::getContactById(%1%)") % id);
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Contact::Manager *cr = r->getContactManager();
  std::auto_ptr<Register::Contact::List> cl(cr->createList());

  DBase::Filters::Union uf;
  DBase::Filters::Contact *cf = new DBase::Filters::ContactImpl();
  cf->addId().setValue(DBase::ID(id));
  uf.addFilter(cf);
  cl->reload2(uf, m_db_manager.get());

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

void ccReg_Admin_i::fillNSSet(ccReg::NSSetDetail* cn, Register::NSSet::NSSet* n) {
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
  catch (Register::NOT_FOUND) {
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
    if (register_manager_->getStatusDesc(
        n->getStatusByIdx(i)->getStatusId()
    )->getExternal())
      slist.push_back(n->getStatusByIdx(i)->getStatusId());
  }
  cn->statusList.length(slist.size());
  for (unsigned i=0; i<slist.size(); i++)
    cn->statusList[i] = slist[i];
}

ccReg::NSSetDetail* ccReg_Admin_i::getNSSetByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::getNSSetByHandle('%1%')") % handle);

  DB db;
  if (!handle || !*handle)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::NSSet::Manager *nr = r->getNSSetManager();
  std::auto_ptr<Register::NSSet::List> nl(nr->createList());
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
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::NSSet::Manager *nr = r->getNSSetManager();
  std::auto_ptr<Register::NSSet::List> nl(nr->createList());

  DBase::Filters::Union uf;
  DBase::Filters::NSSet *nf = new DBase::Filters::NSSetImpl();
  nf->addId().setValue(DBase::ID(id));
  uf.addFilter(nf);
  nl->reload2(uf, m_db_manager.get());

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

void ccReg_Admin_i::fillEPPAction(ccReg::EPPAction* cea,
                                  const Register::Registrar::EPPAction *rea) {
  cea->id = rea->getId();
  cea->xml = DUPSTRFUN(rea->getEPPMessage);
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
  DB db;
  if (!svTRID || !*svTRID)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  Register::Registrar::EPPActionList *eal = rm->getEPPActionList();
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
  TRACE(boost::format("[CALL] ccReg_Admin_i::getEPPActionById(%1%)") % id);
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  
  DB ldb;  
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }  
  try {
    std::auto_ptr<Register::Manager> r(Register::Manager::create(&ldb,cfg.GetRestrictedHandles()));
    Register::Registrar::Manager *rm = r->getRegistrarManager();
    Register::Registrar::EPPActionList *eal = rm->getEPPActionList();
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
  catch (Register::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

void ccReg_Admin_i::fillDomain(ccReg::DomainDetail* cd,
                               Register::Domain::Domain* d) {
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
  cd->admins.length(d->getAdminCount(1));
  cd->temps.length(d->getAdminCount(2));
  std::vector<unsigned> slist;
  for (unsigned i=0; i<d->getStatusCount(); i++) {
    if (register_manager_->getStatusDesc(
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
  catch (Register::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
}

ccReg::DomainDetail* ccReg_Admin_i::getDomainByFQDN(const char* fqdn)
    throw (ccReg::Admin::ObjectNotFound) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::getDomainByFQDN('%1%')") % fqdn);

  DB db;
  if (!fqdn || !*fqdn)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Domain::Manager *dm = r->getDomainManager();
  std::auto_ptr<Register::Domain::List> dl(dm->createList());
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
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Domain::Manager *dm = r->getDomainManager();
  std::auto_ptr<Register::Domain::List> dl(dm->createList());

  DBase::Filters::Union uf;
  DBase::Filters::Domain *df = new DBase::Filters::DomainImpl();
  df->addId().setValue(DBase::ID(id));
  uf.addFilter(df);
  dl->reload2(uf, m_db_manager.get());

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

ccReg::DomainDetails* ccReg_Admin_i::getDomainsByInverseKey(const char* key,
                                                            ccReg::DomainInvKeyType type,
                                                            CORBA::Long limit) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::getDomainsByInverseKey('%1%', %2%, %3%)")
      % key % type % limit);

  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Domain::Manager *dm = r->getDomainManager();
  std::auto_ptr<Register::Domain::List> dl(dm->createList());
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
  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Zone::Manager *zm = r->getZoneManager();
  Register::NSSet::Manager *nm = r->getNSSetManager();
  std::auto_ptr<Register::NSSet::List> nl(nm->createList());
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

void ccReg_Admin_i::fillAuthInfoRequest(ccReg::AuthInfoRequest::Detail *carid,
                                        Register::AuthInfoRequest::Detail *rarid) {
  carid->id = rarid->getId();
  carid->handle = DUPSTRFUN(rarid->getObjectHandle);
  switch (rarid->getRequestStatus()) {
    case Register::AuthInfoRequest::RS_NEW:
      carid->status = ccReg::AuthInfoRequest::RS_NEW;
      break;
    case Register::AuthInfoRequest::RS_ANSWERED:
      carid->status = ccReg::AuthInfoRequest::RS_ANSWERED;
      break;
    case Register::AuthInfoRequest::RS_INVALID:
      carid->status = ccReg::AuthInfoRequest::RS_INVALID;
      break;
  }
  switch (rarid->getRequestType()) {
    case Register::AuthInfoRequest::RT_EPP:
      carid->type = ccReg::AuthInfoRequest::RT_EPP;
      break;
    case Register::AuthInfoRequest::RT_AUTO_PIF:
      carid->type = ccReg::AuthInfoRequest::RT_AUTO_PIF;
      break;
    case Register::AuthInfoRequest::RT_EMAIL_PIF:
      carid->type = ccReg::AuthInfoRequest::RT_EMAIL_PIF;
      break;
    case Register::AuthInfoRequest::RT_POST_PIF:
      carid->type = ccReg::AuthInfoRequest::RT_POST_PIF;
      break;
  }
  carid->crTime = DUPSTRDATE(rarid->getCreationTime);
  carid->closeTime = DUPSTRDATE(rarid->getClosingTime);
  carid->reason = DUPSTRFUN(rarid->getReason);
  carid->svTRID = DUPSTRFUN(rarid->getSvTRID);
  carid->email = DUPSTRFUN(rarid->getEmailToAnswer);
  carid->answerEmailId = rarid->getAnswerEmailId();
  switch (rarid->getObjectType()) {
    case Register::AuthInfoRequest::OT_DOMAIN:
      carid->oType = ccReg::AuthInfoRequest::OT_DOMAIN;
      break;
    case Register::AuthInfoRequest::OT_CONTACT:
      carid->oType = ccReg::AuthInfoRequest::OT_CONTACT;
      break;
    case Register::AuthInfoRequest::OT_NSSET:
      carid->oType = ccReg::AuthInfoRequest::OT_NSSET;
      break;
  }
  carid->objectId = rarid->getObjectId();
  carid->registrar = DUPSTRFUN(rarid->getRegistrarName);
}

ccReg::AuthInfoRequest::Detail* ccReg_Admin_i::getAuthInfoRequestById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound) {
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  MailerManager mm(ns);
  std::auto_ptr<Register::Document::Manager>
      docman(Register::Document::Manager::create(cfg.GetDocGenPath(),
                                                 cfg.GetDocGenTemplatePath(),
                                                 cfg.GetFileClientPath(),
                                                 ns->getHostName() ) );
  std::auto_ptr<Register::AuthInfoRequest::Manager>
      r(Register::AuthInfoRequest::Manager::create(&db,&mm,docman.get()));
  Register::AuthInfoRequest::List *airl = r->getList();
  airl->setIdFilter(id);
  airl->reload();
  if (airl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::AuthInfoRequest::Detail* aird = new ccReg::AuthInfoRequest::Detail;
  fillAuthInfoRequest(aird, airl->get(0));
  db.Disconnect();
  return aird;
}

ccReg::Mailing::Detail* ccReg_Admin_i::getEmailById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound) {
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  MailerManager mm(ns);
  MailerManager::Filter mf;
  mf.id = id;
  try {mm.reload(mf);}
  catch (...) {throw ccReg::Admin::ObjectNotFound();}
  if (mm.getMailList().size() != 1)
    throw ccReg::Admin::ObjectNotFound();
  MailerManager::Detail& mld = mm.getMailList()[0];
  ccReg::Mailing::Detail* md = new ccReg::Mailing::Detail;
  md->id = mld.id;
  md->type = mld.type;
  md->status = mld.status;
  md->createTime = DUPSTRC(mld.createTime);
  md->modTime = DUPSTRC(mld.modTime);
  md->content = DUPSTRC(mld.content);
  md->handles.length(mld.handles.size());
  for (unsigned i=0; i<mld.handles.size(); i++)
    md->handles[i] = DUPSTRC(mld.handles[i]);
  md->attachments.length(mld.attachments.size());
  for (unsigned i=0; i<mld.attachments.size(); i++)
    md->attachments[i] = mld.attachments[i];
  return md;
}

void ccReg_Admin_i::fillInvoice(ccReg::Invoicing::Invoice *ci,
                                Register::Invoicing::Invoice *i) {
  std::stringstream buf;
  ci->id = i->getId();
  ci->zone = i->getZone();
  ci->crTime = DUPSTRDATE(i->getCrTime);
  ci->taxDate = DUPSTRDATED(i->getTaxDate);
  ci->fromDate = DUPSTRDATED(i->getAccountPeriod().begin);
  ci->toDate = DUPSTRDATED(i->getAccountPeriod().end);
  ci->type
      = (i->getType() == Register::Invoicing::IT_DEPOSIT ? ccReg::Invoicing::IT_ADVANCE
                                                         : ccReg::Invoicing::IT_ACCOUNT);
  buf << i->getNumber();
  ci->number = DUPSTRC(buf.str());
  ci->registrarId = i->getRegistrar();
  ci->registrarHandle = DUPSTRC(i->getClient()->getHandle());
  ci->credit = DUPSTRC(formatMoney(i->getCredit()));
  ci->price = DUPSTRC(formatMoney(i->getPrice()));
  ci->vatRate = i->getVatRate();
  ci->total = DUPSTRC(formatMoney(i->getTotal()));
  ci->totalVAT = DUPSTRC(formatMoney(i->getTotalVAT()));
  ci->varSymbol = DUPSTRC(i->getVarSymbol());
  ci->filePDF = i->getFilePDF();
  ci->fileXML = i->getFileXML();
  ci->payments.length(i->getSourceCount());
  for (unsigned k=0; k<i->getSourceCount(); k++) {
    const Register::Invoicing::PaymentSource *ps = i->getSource(k);
    ci->payments[k].id = ps->getId();
    ci->payments[k].price = DUPSTRC(formatMoney(ps->getPrice()));
    ci->payments[k].balance = DUPSTRC(formatMoney(ps->getCredit()));
    buf.str("");
    buf << ps->getNumber();
    ci->payments[k].number = DUPSTRC(buf.str());
  }
  ci->actions.length(i->getActionCount());
  for (unsigned k=0; k<i->getActionCount(); k++) {
    const Register::Invoicing::PaymentAction *pa = i->getAction(k);
    ci->actions[k].objectId = pa->getObjectId();
    ci->actions[k].objectName = DUPSTRFUN(pa->getObjectName);
    ci->actions[k].actionTime = DUPSTRDATE(pa->getActionTime);
    ci->actions[k].exDate = DUPSTRDATED(pa->getExDate);
    ci->actions[k].actionType = pa->getAction();
    ci->actions[k].unitsCount = pa->getUnitsCount();
    ci->actions[k].pricePerUnit = DUPSTRC(formatMoney(pa->getPricePerUnit()));
    ci->actions[k].price = DUPSTRC(formatMoney(pa->getPrice()));
  }
}

ccReg::Invoicing::Invoice* ccReg_Admin_i::getInvoiceById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound) {
  DB db;
  if (!id)
    throw ccReg::Admin::ObjectNotFound();
  db.OpenDatabase(m_connection_string.c_str());
  MailerManager mm(ns);
  std::auto_ptr<Register::Document::Manager>
      docman(Register::Document::Manager::create(cfg.GetDocGenPath(),
                                                 cfg.GetDocGenTemplatePath(),
                                                 cfg.GetFileClientPath(),
                                                 ns->getHostName() ) );
  std::auto_ptr<Register::Invoicing::Manager>
      invman(Register::Invoicing::Manager::create(&db,docman.get(),&mm));
  Register::Invoicing::List *invl = invman->createList();
  invl->setIdFilter(id);
  invl->reload();
  if (invl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::Invoicing::Invoice* inv = new ccReg::Invoicing::Invoice;
  fillInvoice(inv, invl->get(0));
  db.Disconnect();
  return inv;
}

CORBA::Long ccReg_Admin_i::getDomainCount(const char *zone) {
  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getDomainCount(zone);
  db.Disconnect();
  return ret;
}

CORBA::Long ccReg_Admin_i::getEnumNumberCount() {
  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getEnumNumberCount();
  db.Disconnect();
  return ret;
}

ccReg::EPPActionTypeSeq* ccReg_Admin_i::getEPPActionTypeList() {
  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  ccReg::EPPActionTypeSeq *et = new ccReg::EPPActionTypeSeq;
  
  et->length(rm->getEPPActionTypeCount());
  for (unsigned i=0; i<rm->getEPPActionTypeCount(); i++)
    (*et)[i] = DUPSTRC(rm->getEPPActionTypeByIdx(i));
  
  db.Disconnect();
  return et;
}

ccReg::CountryDescSeq* ccReg_Admin_i::getCountryDescList() {
  DB db;
  db.OpenDatabase(m_connection_string.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db,cfg.GetRestrictedHandles()));
  /* 
   * TEMP: this is for loading country codes from database - until new database 
   * library is not fully integrated into registrar library 
   */ 
  r->dbManagerInit(m_db_manager.get());
  
  ccReg::CountryDescSeq *cd = new ccReg::CountryDescSeq;
  cd->length(r->getCountryDescSize());
  for (unsigned i=0; i<r->getCountryDescSize(); i++) {
    (*cd)[i].cc = DUPSTRC(r->getCountryDescByIdx(i).cc);
    (*cd)[i].name = DUPSTRC(r->getCountryDescByIdx(i).name);
  }
  
  db.Disconnect();
  return cd;
}

char* ccReg_Admin_i::getDefaultCountry() {
  return CORBA::string_dup("CZ");
}

ccReg::ObjectStatusDescSeq* ccReg_Admin_i::getDomainStatusDescList(const char *lang) {
  ccReg::ObjectStatusDescSeq* o = new ccReg::ObjectStatusDescSeq;
  for (unsigned i=0; i<register_manager_->getStatusDescCount(); i++) {
    const Register::StatusDesc *sd = register_manager_->getStatusDescByIdx(i);
    if (sd->getExternal() && sd->isForType(3)) {
      o->length(o->length()+1);
      try {
        (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
      } catch (...) {
        // unknown language
        (*o)[o->length()-1].name = CORBA::string_dup("");
      }
      (*o)[o->length()-1].id = sd->getId();
    }
  }
  return o;
}

ccReg::ObjectStatusDescSeq* ccReg_Admin_i::getContactStatusDescList(const char *lang) {
  ccReg::ObjectStatusDescSeq* o = new ccReg::ObjectStatusDescSeq;
  for (unsigned i=0; i<register_manager_->getStatusDescCount(); i++) {
    const Register::StatusDesc *sd = register_manager_->getStatusDescByIdx(i);
    if (sd->getExternal() && sd->isForType(1)) {
      o->length(o->length()+1);
      try {
        (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
      } catch (...) {
        // unknown language
        (*o)[o->length()-1].name = CORBA::string_dup("");
      }
      (*o)[o->length()-1].id = sd->getId();
    }
  }
  return o;
}

ccReg::ObjectStatusDescSeq* ccReg_Admin_i::getNSSetStatusDescList(const char *lang) {
  ccReg::ObjectStatusDescSeq* o = new ccReg::ObjectStatusDescSeq;
  for (unsigned i=0; i<register_manager_->getStatusDescCount(); i++) {
    const Register::StatusDesc *sd = register_manager_->getStatusDescByIdx(i);
    if (sd->getExternal() && sd->isForType(2)) {
      o->length(o->length()+1);
      try {
        (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
      } catch (...) {
        // unknown language
        (*o)[o->length()-1].name = CORBA::string_dup("");
      }
      (*o)[o->length()-1].id = sd->getId();
    }
  }
  return o;
}

char* ccReg_Admin_i::getCreditByZone(const char*registrarHandle, ccReg::TID zone) {
  DB ldb;
  try {
    ldb.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Register::Invoicing::Manager> invman(Register::Invoicing::Manager::create(&ldb,NULL,NULL));
    char *ret = DUPSTRC(formatMoney(invman->getCreditByZone(registrarHandle,zone)));
    ldb.Disconnect();
    return ret;
  }
  catch (...) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}

void ccReg_Admin_i::generateLetters() {
  DB ldb;
  try {
    ldb.OpenDatabase(m_connection_string.c_str());
    
    MailerManager mm(ns);
    std::auto_ptr<Register::Document::Manager> docman(
        Register::Document::Manager::create(cfg.GetDocGenPath(), 
                                            cfg.GetDocGenTemplatePath(),
                                            cfg.GetFileClientPath(), 
                                            ns->getHostName()));
    std::auto_ptr<Register::Zone::Manager> zoneMan(
        Register::Zone::Manager::create(&ldb));
    std::auto_ptr<Register::Domain::Manager> domMan(
        Register::Domain::Manager::create(&ldb,
                                          zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
        Register::Contact::Manager::create(&ldb,
                                           cfg.GetRestrictedHandles()));
    std::auto_ptr<Register::NSSet::Manager> nssMan(
        Register::NSSet::Manager::create(&ldb,
                                         zoneMan.get(),
                                         cfg.GetRestrictedHandles()));
    std::auto_ptr<Register::Registrar::Manager> rMan(
        Register::Registrar::Manager::create(&ldb));
    std::auto_ptr<Register::Notify::Manager> notifyMan(
        Register::Notify::Manager::create(&ldb,
                                          &mm,
                                          conMan.get(),
                                          nssMan.get(),
                                          domMan.get(), 
                                          docman.get(),
                                          rMan.get()));
    notifyMan->generateLetters();
    ldb.Disconnect();
  }
  catch (...) {
    ldb.Disconnect();
  }
}

ccReg::TID ccReg_Admin_i::createPublicRequest(ccReg::PublicRequest::Type _type,
                                              ccReg::TID _epp_action_id,
                                              const char *_reason,
                                              const char *_email_to_answer,
                                              const ccReg::Admin::ObjectIdList& _object_ids) 
  throw (
    ccReg::Admin::BAD_EMAIL, ccReg::Admin::OBJECT_NOT_FOUND,
    ccReg::Admin::ACTION_NOT_FOUND, ccReg::Admin::SQL_ERROR,
    ccReg::Admin::INVALID_INPUT, ccReg::Admin::REQUEST_BLOCKED
  ) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::createPublicRequest(%1%, %2%, '%3%', '%4%, %5%") % 
        _type % _epp_action_id % _reason % _email_to_answer % &_object_ids);
  
  MailerManager mailer_manager(ns);
  
  std::auto_ptr<DBase::Connection> conn(m_db_manager->getConnection());
  std::auto_ptr<Register::Document::Manager> doc_manager(
                     Register::Document::Manager::create(cfg.GetDocGenPath(),
                                                         cfg.GetDocGenTemplatePath(),
                                                         cfg.GetFileClientPath(),
                                                         ns->getHostName()));
  std::auto_ptr<Register::PublicRequest::Manager> request_manager(
                         Register::PublicRequest::Manager::create(m_db_manager.get(),
                                                            register_manager_->getDomainManager(),
                                                            register_manager_->getContactManager(),
                                                            register_manager_->getNSSetManager(),
                                                            &mailer_manager,
                                                            doc_manager.get()));
  
#define REQUEST_TYPE_CORBA2DB_CASE(type)            \
  case ccReg::PublicRequest::type:                  \
    request_type = Register::PublicRequest::type; break;  
  
  Register::PublicRequest::Type request_type;
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
  }
  
  Register::PublicRequest::PublicRequest *new_request = 
    request_manager->createRequest(request_type,conn.get());
  new_request->setType(request_type);
  new_request->setEppActionId(_epp_action_id);
  new_request->setReason(_reason);
  new_request->setEmailToAnswer(_email_to_answer);
  for (unsigned i=0; i<_object_ids.length(); i++)
    new_request->addObject(Register::PublicRequest::OID(_object_ids[i]));
  try {
    if (!new_request->check()) throw ccReg::Admin::REQUEST_BLOCKED();
    new_request->save(conn.get());
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
  TRACE(boost::format("[CALL] ccReg_Admin_i::processPublicRequest(%1%, %2%)") %
  	id % invalid);

  MailerManager mailer_manager(ns);  
  std::auto_ptr<Register::Document::Manager> doc_manager(
                     Register::Document::Manager::create(cfg.GetDocGenPath(),
                                                         cfg.GetDocGenTemplatePath(),
                                                         cfg.GetFileClientPath(),
                                                         ns->getHostName()));
  std::auto_ptr<Register::PublicRequest::Manager> request_manager(
                         Register::PublicRequest::Manager::create(m_db_manager.get(),
                                                            register_manager_->getDomainManager(),
                                                            register_manager_->getContactManager(),
                                                            register_manager_->getNSSetManager(),
                                                            &mailer_manager,
                                                            doc_manager.get()));
  try {
    request_manager->processRequest(id,invalid,true);
  }
  catch (Register::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
  catch (Register::NOT_FOUND) {
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }
  catch (Register::Mailer::NOT_SEND) {
    throw ccReg::Admin::MAILER_ERROR();
  }
  catch (Register::PublicRequest::REQUEST_BLOCKED) {
    throw ccReg::Admin::REQUEST_BLOCKED();
  }
}

ccReg::Admin::Buffer* ccReg_Admin_i::getPublicRequestPDF(ccReg::TID id,
                                                         const char *lang) {
  TRACE(boost::format("[CALL] ccReg_Admin_i::getPublicRequestPDF(%1%, '%2%')") %
        id % lang);

  MailerManager mailer_manager(ns);
   
  std::auto_ptr<DBase::Connection> conn(m_db_manager->getConnection());
  std::auto_ptr<Register::Document::Manager> doc_manager(
                       Register::Document::Manager::create(cfg.GetDocGenPath(),
                                                           cfg.GetDocGenTemplatePath(),
                                                           cfg.GetFileClientPath(),
                                                           ns->getHostName()));
  std::auto_ptr<Register::PublicRequest::Manager> request_manager(
                           Register::PublicRequest::Manager::create(m_db_manager.get(),
                                                              register_manager_->getDomainManager(),
                                                              register_manager_->getContactManager(),
                                                              register_manager_->getNSSetManager(),
                                                              &mailer_manager,
                                                              doc_manager.get()));  DB db;
  try {
    std::stringstream outstr;
    request_manager-> getPdf(id,lang,outstr);
    unsigned long size = outstr.str().size();
    CORBA::Octet *b = ccReg::Admin::Buffer::allocbuf(size);
    memcpy(b,outstr.str().c_str(),size);
    ccReg::Admin::Buffer* output = new ccReg::Admin::Buffer(size, size, b, 1);
    LOGGER("corba").info(boost::format("Retrieved pdf file for request id=%1% lang='%2%'") %
                         id % lang);
    return output;
  }
  catch (Register::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
  catch (Register::NOT_FOUND) {
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }
}

