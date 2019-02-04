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
#include "src/bin/corba/Admin.hh"

#include "src/bin/corba/admin/common.hh"
#include "src/bin/corba/admin/admin_impl.hh"
#include "src/deprecated/util/log.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/registry.hh"
#include "src/deprecated/libfred/notify.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "libfred/poll/get_request_fee_message.hh"
#include "src/deprecated/libfred/banking/get_bank_accounts.hh"
#include "src/deprecated/libfred/banking/payment_data.hh"
#include "src/deprecated/libfred/public_request/public_request_authinfo_impl.hh"
#include "src/deprecated/libfred/public_request/public_request_block_impl.hh"
#include "src/deprecated/libfred/public_request/public_request_personal_info_impl.hh"
#include "libfred/public_request/create_public_request.hh"
#include "libfred/registrable_object/contact/undisclose_address.hh"
#include "libfred/registrar/info_registrar.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "util/factory_check.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "util/random.hh"
#include "src/bin/corba/connection_releaser.hh"
#include "src/bin/corba/epp_corba_client_impl.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/backend/contact_verification/public_request_contact_verification_impl.hh"

class Registry_RegistrarCertification_i;
class Registry_RegistrarGroup_i;

ccReg_Admin_i::ccReg_Admin_i(const std::string _database, NameService *_ns
                             , bool restricted_handles
                             , const std::string& docgen_path
                             , const std::string& docgen_template_path
                             , unsigned int docgen_domain_count_limit
                             , const std::string& fileclient_path
                             , unsigned adifd_session_max
                             , unsigned adifd_session_timeout
                             , unsigned adifd_session_garbage
                             , bool _session_garbage)
: m_connection_string(_database), ns(_ns)
, restricted_handles_(restricted_handles)
, docgen_path_(docgen_path)
, docgen_template_path_(docgen_template_path)
, docgen_domain_count_limit_(docgen_domain_count_limit)
, fileclient_path_(fileclient_path)
, adifd_session_max_(adifd_session_max)
, adifd_session_timeout_(adifd_session_timeout)
, adifd_session_garbage_(adifd_session_garbage)
, bankingInvoicing(_ns)
, session_garbage_active_ (false)
, session_garbage_thread_ (0)
{
  /* HACK: to recognize ADIFD and PIFD until separation of objects */
  if (_session_garbage) {
    server_name_ = ("adifd");
  }
  else {
    server_name_ = ("pifd");
  }

  //factory_check - required keys are in factory
  FactoryHaveSupersetOfKeysChecker<LibFred::PublicRequest::Factory>
  ::KeyVector required_keys = boost::assign::list_of
   (LibFred::PublicRequest::PRT_AUTHINFO_AUTO_PIF)
   (LibFred::PublicRequest::PRT_AUTHINFO_EMAIL_PIF)
   (LibFred::PublicRequest::PRT_AUTHINFO_POST_PIF)
   (LibFred::PublicRequest::PRT_BLOCK_TRANSFER_EMAIL_PIF)
   (LibFred::PublicRequest::PRT_BLOCK_CHANGES_EMAIL_PIF)
   (LibFred::PublicRequest::PRT_UNBLOCK_TRANSFER_EMAIL_PIF)
   (LibFred::PublicRequest::PRT_UNBLOCK_CHANGES_EMAIL_PIF)
   (LibFred::PublicRequest::PRT_BLOCK_TRANSFER_POST_PIF)
   (LibFred::PublicRequest::PRT_BLOCK_CHANGES_POST_PIF)
   (LibFred::PublicRequest::PRT_UNBLOCK_TRANSFER_POST_PIF)
   (LibFred::PublicRequest::PRT_UNBLOCK_CHANGES_POST_PIF);

  FactoryHaveSupersetOfKeysChecker<LibFred::PublicRequest::Factory>
      (required_keys).check();

  //factory_check - factory keys are in database
  FactoryHaveSubsetOfKeysChecker<LibFred::PublicRequest::Factory>
      (LibFred::PublicRequest::get_enum_public_request_type()).check();


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

  Database::Connection conn = Database::Manager::acquire();
  db_disconnect_guard_.reset(new DB(conn));


  registry_manager_.reset(LibFred::Manager::create(db_disconnect_guard_
          , restricted_handles_));
  registry_manager_->initStates();

  if (_session_garbage) {
    session_garbage_active_ = true;
    session_garbage_thread_ = new boost::thread(boost::bind(&ccReg_Admin_i::garbageSession, this));
  }
}

ccReg_Admin_i::~ccReg_Admin_i() {
  TRACE("[CALL] ccReg_Admin_i::~ccReg_Admin_i()");

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
    LOGGER.debug(boost::format("session '%1%' destroyed") % session_id);
  }

  TRACE("Admin object completely destroyed.");
}

#define SWITCH_CONVERT(x) case LibFred::x : ch->handleClass = ccReg::x; break
#define SWITCH_CONVERT_T(x) case LibFred::x : ch->hType = ccReg::x; break
void ccReg_Admin_i::checkHandle(const char* handle,
                                ccReg::CheckHandleTypeSeq_out chso) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DBSharedPtr  db;
  Database::Connection conn = Database::Manager::acquire();
  db.reset(new DB(conn));

  std::unique_ptr<LibFred::Manager> r(LibFred::Manager::create(db
          , restricted_handles_));
  ccReg::CheckHandleTypeSeq* chs = new ccReg::CheckHandleTypeSeq;
  LibFred::CheckHandleList chl;
  r->checkHandle(handle, chl, true); // allow IDN in whois queries
  chs->length(chl.size());
  for (unsigned i=0; i< chl.size(); i++) {
    ccReg::CheckHandleType *ch = &(*chs)[i];
    LibFred::CheckHandle& chd = chl[i];
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
}

void ccReg_Admin_i::garbageSession() {
  Logging::Context ctx(server_name_);
  Logging::Context ctx2("session-garbage");

  LOGGER.debug("thread started...");

  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  while (session_garbage_active_) {
    LOGGER.debug("procedure sleeped");

    boost::xtime sleep_time;
#if BOOST_VERSION >= 105000
    boost::xtime_get(&sleep_time, boost::TIME_UTC_);
#else
    boost::xtime_get(&sleep_time, boost::TIME_UTC);
#endif
    sleep_time.sec += adifd_session_garbage_;
    cond_.timed_wait(scoped_lock, sleep_time);

    LOGGER.debug("procedure invoked");

    SessionListType::iterator it = m_session_list.begin();
    while (it != m_session_list.end()) {
      if (it->second->isTimeouted()) {
        std::string session_id = it->first;
        delete it->second;
        m_session_list.erase(it++);
        LOGGER.debug(boost::format("session '%1%' deleted -- remains '%2%'")
            % session_id % m_session_list.size());
      }
      else {
        ++it;
      }
    }
  }
  LOGGER.debug("thread stopped");
}

void ccReg_Admin_i::authenticateUser(const char* _username,
                                     const char* _password)
{
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
  unsigned sess_max = adifd_session_max_;
  if (sess_max && m_session_list.size() == sess_max) {
    LOGGER.info(boost::format("session limit (max=%1%) exceeded") % sess_max);

    SessionListType::iterator it        = m_session_list.begin();
    SessionListType::iterator it_oldest = m_session_list.begin();
    for (; it != m_session_list.end(); ++it) {
      LOGGER.debug(boost::format("session %1% - last activity at %2%")
                                          % it->second->getId()
                                          % it->second->getLastActivity());
      if (it_oldest->second->getLastActivity() > it->second->getLastActivity()) {
        it_oldest = it;
      }
    }

    LOGGER.info(boost::format("destroying oldest session -- %1% (last activity at %2%)")
                                       % it_oldest->second->getId()
                                       % it_oldest->second->getLastActivity());
    delete it_oldest->second;
    m_session_list.erase(it_oldest);
  }

  ccReg_User_i *user_info = new ccReg_User_i(1 /* dummy id until user management */, username, username, username);

  std::string session_id = "sessid#" + Random::string_alphanum(5) + "-" + username;


  ccReg_Session_i *session = new ccReg_Session_i(session_id, m_connection_string, ns,
          restricted_handles_,
          docgen_path_,
          docgen_template_path_,
          fileclient_path_,
          adifd_session_timeout_,
          bankingInvoicing._this(), user_info);

  m_session_list[session_id] = session;


  LOGGER.notice(boost::format("admin session '%1%' created -- total number of sessions is '%2%'")
      % session_id % m_session_list.size());

  return CORBA::string_dup(session_id.c_str());
}

void ccReg_Admin_i::destroySession(const char* _session_id) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::destroySession('%1%')") % _session_id);

  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::iterator it = m_session_list.find(_session_id);
  if (it == m_session_list.end()) {
    LOGGER.debug(boost::format("session '%1%' not found -- already destroyed")
        % _session_id);
    return;
  }

  delete it->second;
  m_session_list.erase(it);
}

ccReg::Session_ptr ccReg_Admin_i::getSession(const char* _session_id)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getSession('%1%')") % _session_id);

  // garbageSession();

  boost::mutex::scoped_lock scoped_lock(m_session_list_mutex);
  SessionListType::const_iterator it = m_session_list.find(_session_id);
  if (it == m_session_list.end()) {
    LOGGER.debug(boost::format("session '%1%' not found -- deleted due to timout")
        % _session_id);
    throw ccReg::Admin::ObjectNotFound();
  }

  it->second->updateActivity();
  return it->second->_this();
}


void ccReg_Admin_i::fillRegistrar(ccReg::AdminRegistrar& creg,
                                  LibFred::Registrar::Registrar *reg)
{
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
  creg.credit = DUPSTRC(formatMoney(reg->getCredit()));
  creg.access.length(reg->getACLSize());
  for (unsigned i=0; i<reg->getACLSize(); i++) {
    creg.access[i].md5Cert = DUPSTRFUN(reg->getACL(i)->getCertificateMD5);
    creg.access[i].password = "";
  }
  creg.hidden = reg->getHandle() == "REG-CZNIC" ? true : false;
}

ccReg::RegistrarList* ccReg_Admin_i::getRegistrars()
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;
  try {

    DBSharedPtr  db;
    Database::Connection conn = Database::Manager::acquire();
    db.reset(new DB(conn));

    std::unique_ptr<LibFred::Manager> regm(
        LibFred::Manager::create(db
                , restricted_handles_)
    );
    LibFred::Registrar::Manager *rm = regm->getRegistrarManager();
    LibFred::Registrar::RegistrarList::AutoPtr rl = rm->createList();

    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    unionFilter->addFilter( new Database::Filters::RegistrarImpl(true) );
    rl->reload(*unionFilter.get());

    LOG<Logging::Log::EventImportance::notice>( "getRegistrars: num -> %d", rl->size() );
    ccReg::RegistrarList* reglist = new ccReg::RegistrarList;
    reglist->length(rl->size());
    for (unsigned i=0; i<rl->size(); i++)
    fillRegistrar((*reglist)[i],rl->get(i));
    return reglist;
  }
  catch (LibFred::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
}

ccReg::AdminRegistrar* ccReg_Admin_i::getRegistrarById(ccReg::TID id)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  LOG<Logging::Log::EventImportance::notice>( "getRegistarByHandle: id -> %lld", (unsigned long long)id );
  if (!id) throw ccReg::Admin::ObjectNotFound();

  try {

    DBSharedPtr  db;
    Database::Connection conn = Database::Manager::acquire();
    db.reset(new DB(conn));

    std::unique_ptr<LibFred::Manager> regm(
        LibFred::Manager::create(db,restricted_handles_)
    );
    LibFred::Registrar::Manager *rm = regm->getRegistrarManager();
    LibFred::Registrar::RegistrarList::AutoPtr rl = rm->createList();

    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    std::unique_ptr<Database::Filters::Registrar> r ( new Database::Filters::RegistrarImpl(true));
    r->addId().setValue(Database::ID(id));
    unionFilter->addFilter( r.release() );
    rl->reload(*unionFilter.get());

    if (rl->size() < 1) {
      throw ccReg::Admin::ObjectNotFound();
    }
    ccReg::AdminRegistrar* creg = new ccReg::AdminRegistrar;
    fillRegistrar(*creg,rl->get(0));
    return creg;
  }
  catch (LibFred::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
}

CORBA::Long ccReg_Admin_i::getDomainCount(const char *zone) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DBSharedPtr  db;
  Database::Connection conn = Database::Manager::acquire();
  db.reset(new DB(conn));


  std::unique_ptr<LibFred::Manager> r(LibFred::Manager::create(db
          , restricted_handles_));
  LibFred::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getDomainCount(zone);
  return ret;
}

CORBA::Long ccReg_Admin_i::getSignedDomainCount(const char *_fqdn)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DBSharedPtr  db;
  Database::Connection conn = Database::Manager::acquire();
  db.reset(new DB(conn));

  std::unique_ptr<LibFred::Manager> r(LibFred::Manager::create(db
          ,restricted_handles_));
  LibFred::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getSignedDomainCount(_fqdn);
  return ret;
}

CORBA::Long ccReg_Admin_i::getEnumNumberCount() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DBSharedPtr  db;
  Database::Connection conn = Database::Manager::acquire();
  db.reset(new DB(conn));

  std::unique_ptr<LibFred::Manager> r(LibFred::Manager::create(db
          , restricted_handles_));
  LibFred::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getEnumNumberCount();
  return ret;
}


Registry::CountryDescSeq* ccReg_Admin_i::getCountryDescList() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  DBSharedPtr  db;
  Database::Connection conn = Database::Manager::acquire();
  db.reset(new DB(conn));


  std::unique_ptr<LibFred::Manager> r(LibFred::Manager::create(db
          , restricted_handles_));
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

  return cd;
}

char* ccReg_Admin_i::getDefaultCountry() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  return CORBA::string_dup("CZ");
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
            LOGGER.error("ccReg_Admin_i::getObjectStatusDescList error new Registry::ObjectStatusDescSeq failed ");
            throw ccReg::Admin::InternalServerError();
        }

        unsigned states_count = registry_manager_->getStatusDescCount();
        o->length(states_count);
        for (unsigned i = 0; i < states_count; ++i) {
        const LibFred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
        if(sd == 0)
        {
            LOGGER.error((std::string(
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
        LOGGER.error(msg.c_str());
        throw ccReg::Admin::InternalServerError();
    }
    catch(...)
    {
        LOGGER.error("ccReg_Admin_i::getObjectStatusDescList error ");
        throw ccReg::Admin::InternalServerError();
    }
}

char* ccReg_Admin_i::getCreditByZone(const char*registrarHandle, ccReg::TID zone) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  std::unique_ptr<LibFred::Invoicing::Manager>
      invman(LibFred::Invoicing::Manager::create());
  char *ret = DUPSTRC(invman->getCreditByZone(registrarHandle, zone));
  return ret;
}

void ccReg_Admin_i::generateLetters() {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;


  try {
    DBSharedPtr db;
    Database::Connection conn = Database::Manager::acquire();
    db.reset(new DB(conn));


    MailerManager mm(ns);
    std::unique_ptr<LibFred::Document::Manager> docman(
        LibFred::Document::Manager::create(docgen_path_,
                                            docgen_template_path_,
                                            fileclient_path_,
                                            ns->getHostName()));
    std::unique_ptr<LibFred::Zone::Manager> zoneMan(
        LibFred::Zone::Manager::create());

    LibFred::Messages::ManagerPtr msgMan
        = LibFred::Messages::create_manager();


    std::unique_ptr<LibFred::Domain::Manager> domMan(
        LibFred::Domain::Manager::create(db,
                                          zoneMan.get()));
    std::unique_ptr<LibFred::Contact::Manager> conMan(
        LibFred::Contact::Manager::create(db,
                                           restricted_handles_));
    std::unique_ptr<LibFred::Nsset::Manager> nssMan(
        LibFred::Nsset::Manager::create(db,
                                         zoneMan.get(),
                                         restricted_handles_));
    std::unique_ptr<LibFred::Keyset::Manager> keyMan(
            LibFred::Keyset::Manager::create(
                    db,
                restricted_handles_));
    std::unique_ptr<LibFred::Registrar::Manager> rMan(
        LibFred::Registrar::Manager::create(db));
    std::unique_ptr<LibFred::Notify::Manager> notifyMan(
        LibFred::Notify::Manager::create(db,
                                          &mm,
                                          conMan.get(),
                                          nssMan.get(),
                                          keyMan.get(),
                                          domMan.get(),
                                          docman.get(),
                                          rMan.get(),
                                          msgMan
                                          ));
    notifyMan->generateLetters(docgen_domain_count_limit_);
  }
  catch (...) {
  }
}

bool ccReg_Admin_i::setInZoneStatus(ccReg::TID domainId)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;
    try {

        TRACE(boost::format("[CALL] ccReg_Admin_i::setInZoneStatus(%1%)")
                % domainId);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        /* Ticket #11166 - quick fix, we should rewrite interface and impl. for that feature */
        boost::posix_time::ptime whatisthetime = time_from_string(static_cast<std::string>(conn.exec("SELECT now()")[0][0]));
        boost::gregorian::date_duration dd7(7);

        Database::Result dname_res = conn.exec_params("SELECT obr.name "
            " FROM object_registry obr JOIN domain d ON d.id = obr.id "
            " WHERE d.id = $1::bigint", Database::query_param_list(domainId));
        if(dname_res.size() != 1)
        {
            throw std::runtime_error("domain not found");
        }

        std::string  object_name (dname_res[0][0]);

        LibFred::createObjectStateRequestName(
            object_name, 3 //object_type domain
            , Util::vector_of<std::string>(LibFred::ObjectState::SERVER_INZONE_MANUAL)
            , boost::posix_time::to_iso_extended_string(whatisthetime) //valid_from now
            , boost::posix_time::to_iso_extended_string(whatisthetime + dd7) //const optional_string& valid_to
            , true //bool update_object_state
            );
        tx.commit();
    } catch (std::exception& ex) {
        LOGGER.error("setInZoneStatus: an error has occured");
        LOGGER.error(ex.what());
        return false;
    } catch (...) {
        LOGGER.error("setInZoneStatus: an error has occured");
        return false;
    }
    return true;
}

ccReg::TID ccReg_Admin_i::createPublicRequest(Registry::PublicRequest::Type _type,
                                              const char *_reason,
                                              const char *_email_to_answer,
                                              const ccReg::Admin::ObjectIdList& _object_ids,
                                              const ccReg::TID requestId)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::createPublicRequest(%1%, '%2%', '%3%', %4%, %5%)") %
        _type %  _reason % _email_to_answer % &_object_ids % requestId);

  MailerManager mailer_manager(ns);

  std::unique_ptr<LibFred::Document::Manager> doc_manager(
          LibFred::Document::Manager::create(
              docgen_path_,
              docgen_template_path_,
              fileclient_path_,
              ns->getHostName())
          );
  std::unique_ptr<LibFred::PublicRequest::Manager> request_manager(
          LibFred::PublicRequest::Manager::create(
              registry_manager_->getDomainManager(),
              registry_manager_->getContactManager(),
              registry_manager_->getNssetManager(),
              registry_manager_->getKeysetManager(),
              &mailer_manager,
              doc_manager.get(),
              registry_manager_->getMessageManager())
          );

#define REQUEST_TYPE_CORBA2DB_CASE(type)            \
  case Registry::PublicRequest::type:                  \
    request_type = LibFred::PublicRequest::type; break;

  LibFred::PublicRequest::Type request_type;
  switch (_type)  {
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
      LOGGER.error(boost::format("can't create new public request - unknown request type (%1%)")
                                          % _type);
      throw ccReg::Admin::INVALID_INPUT();
  }

  std::unique_ptr<LibFred::PublicRequest::PublicRequest> new_request(request_manager->createRequest(request_type));
  new_request->setType(request_type);
  new_request->setRegistrarId(0);
  new_request->setReason(_reason);
  new_request->setEmailToAnswer(_email_to_answer);
  new_request->setRequestId(requestId);
  for (unsigned i=0; i<_object_ids.length(); i++) {
    if (_object_ids[i] == 0) {
        throw ccReg::Admin::OBJECT_NOT_FOUND();
    }
    new_request->addObject(LibFred::PublicRequest::OID(_object_ids[i]));
  }
  try {
    if (!new_request->check()) throw ccReg::Admin::REQUEST_BLOCKED();
    new_request->save();
    return new_request->getId();
  }
  catch (ccReg::Admin::REQUEST_BLOCKED) {
    throw;
  }
  catch (ccReg::Admin::OBJECT_NOT_FOUND) {
    throw;
  }
  catch (...) {
    throw ccReg::Admin::SQL_ERROR();
  }
}

namespace {

std::string get_system_registrar_handle()
{
    const std::string handle =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->system_registrar;
    if (!handle.empty())
    {
        return handle;
    }
    throw std::runtime_error("missing configuration for system registrar");
}

} // namespace {anonymous}

void ccReg_Admin_i::processPublicRequest(ccReg::TID id, CORBA::Boolean invalid)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::processPublicRequest(%1%, %2%)") %
  	id % invalid);

  MailerManager mailer_manager(ns);
  std::unique_ptr<LibFred::Document::Manager> doc_manager(
          LibFred::Document::Manager::create(
              docgen_path_,
              docgen_template_path_,
              fileclient_path_,
              ns->getHostName())
          );
  std::unique_ptr<LibFred::PublicRequest::Manager> request_manager(
          LibFred::PublicRequest::Manager::create(
              registry_manager_->getDomainManager(),
              registry_manager_->getContactManager(),
              registry_manager_->getNssetManager(),
              registry_manager_->getKeysetManager(),
              &mailer_manager,
              doc_manager.get(),
              registry_manager_->getMessageManager())
          );
  try {
    request_manager->processRequest(id,invalid,true);
  }
  catch (LibFred::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
  catch (LibFred::NOT_FOUND) {
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }
  catch (LibFred::Mailer::NOT_SEND) {
    throw ccReg::Admin::MAILER_ERROR();
  }
  catch (LibFred::PublicRequest::REQUEST_BLOCKED) {
    throw ccReg::Admin::REQUEST_BLOCKED();
  }
  catch (...) {
    /* this also catches when try to process request which
     * need to be authenticated */
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }

  std::unique_ptr<LibFred::PublicRequest::List> request_list(request_manager->loadRequest(id));
  unsigned long long object_id = request_list->get(0)->getObject(0).id;
  try {
      LibFred::Contact::undisclose_address_async(object_id, get_system_registrar_handle()); // #21767
  }
  catch (const std::exception& e)
  {
      LOGGER.info(
              boost::format("processing validation request id=%1%: async disclose address of contact with id %2%: %3%")
              % id % object_id % e.what());
  }
  catch (...)
  {
      LOGGER.info(
              boost::format("processing validation request id=%1%: async disclose address of contact with id %2%: unknown error")
              % id % object_id);
  }
}

ccReg::Admin::Buffer* ccReg_Admin_i::getPublicRequestPDF(ccReg::TID id,
                                                         const char *lang) {
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Admin_i::getPublicRequestPDF(%1%, '%2%')") %
        id % lang);

  MailerManager mailer_manager(ns);

  std::unique_ptr<LibFred::Document::Manager> doc_manager(
          LibFred::Document::Manager::create(
              docgen_path_,
              docgen_template_path_,
              fileclient_path_,
              ns->getHostName())
          );
  std::unique_ptr<LibFred::PublicRequest::Manager> request_manager(
          LibFred::PublicRequest::Manager::create(
              registry_manager_->getDomainManager(),
              registry_manager_->getContactManager(),
              registry_manager_->getNssetManager(),
              registry_manager_->getKeysetManager(),
              &mailer_manager,
              doc_manager.get(),
              registry_manager_->getMessageManager())
          );

  try {
    std::stringstream outstr;
    request_manager-> getPdf(id,lang,outstr);
    unsigned long size = outstr.str().size();
    CORBA::Octet *b = ccReg::Admin::Buffer::allocbuf(size);
    memcpy(b,outstr.str().c_str(),size);
    ccReg::Admin::Buffer* output = new ccReg::Admin::Buffer(size, size, b, 1);
    LOGGER.info(boost::format("Retrieved pdf file for request id=%1% lang='%2%'") %
                         id % lang);
    return output;
  }
  catch (LibFred::SQL_ERROR) {
    throw ccReg::Admin::SQL_ERROR();
  }
  catch (LibFred::NOT_FOUND) {
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  }
}

ccReg::TID ccReg_Admin_i::resendPin3Letter(ccReg::TID publicRequestId)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Admin_i::resendPin3Letter(%1%)") % publicRequestId);

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Database::Result res = conn.exec_params(
            "SELECT (SELECT name "            // [0] - type of request
                    "FROM enum_public_request_type WHERE id=pr.request_type),"
                   "(SELECT name "            // [1] - request status opened/resolved/invalidated
                    "FROM enum_public_request_status WHERE id=pr.status),"
                   "prmm.id IS NULL,"         // [2] - have any message
                   "prmm.message_archive_id," // [3] - sms/letter id
                   "ma.id, "                  // [4] - id of pin3 letter
                   "NOT (array_agg(status_id) over ()) "
                       "&& (SELECT array_agg(id) "
                                "FROM enum_send_status "
                              "WHERE status_name NOT IN ('sent', 'no_processing')) AS copy_allowed "
            "FROM public_request pr "
            "LEFT JOIN public_request_messages_map prmm ON prmm.public_request_id=pr.id "
            "LEFT JOIN message_archive ma ON ("
                "ma.id=prmm.message_archive_id AND "
                "ma.message_type_id IN (SELECT id FROM message_type "
                                       "WHERE type IN ('mojeid_pin3','contact_verification_pin3')) AND "
                "ma.comm_type_id=(SELECT id FROM comm_type WHERE type='letter')) "
            "WHERE pr.id=$1::bigint AND "
                  "(prmm.id IS NULL OR prmm.message_archive_id IS NOT NULL) "
                  "ORDER BY prmm.id LIMIT 1",
            Database::query_param_list(publicRequestId)
        );
        if (res.size() <= 0) {
            LOGGER.error(boost::format("publicRequestId: %1% not found") % publicRequestId);
            throw ccReg::Admin::ObjectNotFound();
        }
        const std::string typeOfRequest = static_cast< std::string >(res[0][0]);
        if ((typeOfRequest != "mojeid_contact_identification") &&
            (typeOfRequest != "contact_identification")) {
            LOGGER.error(boost::format("publicRequestId: %1% of %2% type is not PIN3 request")
                % publicRequestId
                % typeOfRequest);
            throw ccReg::Admin::ObjectNotFound();
        }
        const std::string requestStatus = static_cast< std::string >(res[0][1]);
        if (requestStatus != Conversion::Enums::to_db_handle(LibFred::PublicRequest::Status::opened)) {
            LOGGER.error(
                boost::format("publicRequestId: %1% in %2% state is not new PIN3 request")
                % publicRequestId
                % requestStatus);
            throw ccReg::Admin::ObjectNotFound();
        }
        if (static_cast< bool >(res[0][2])) {
            LOGGER.error(boost::format("publicRequestId: %1% doesn't have message")
                % publicRequestId);
            throw ccReg::Admin::ObjectNotFound();
        }
        if (res[0][3].isnull()) {
            LOGGER.error(
                boost::format("publicRequestId: %1% doesn't have message_archive_id")
                % publicRequestId);
            throw ccReg::Admin::ObjectNotFound();
        }
        if (res[0][4].isnull()) {
            LOGGER.error(
                boost::format("message_archive_id: %1% doesn't exists")
                % static_cast< ccReg::TID >(res[0][3]));
            throw ccReg::Admin::ObjectNotFound();
        }
        if (!static_cast<bool>(res[0][5])) {
            LOGGER.warning(boost::format("letter id=%1% has copy prohibited status")
                    % static_cast<unsigned long long>(res[0][3]));
            throw LibFred::Messages::MessageCopyProhibited();
        }

        const ccReg::TID letterId = static_cast< ccReg::TID >(res[0][4]);
        LibFred::Messages::ManagerPtr msgMan = LibFred::Messages::create_manager();
        const ccReg::TID newLetterId = msgMan->copy_letter_to_send(letterId);
        conn.exec_params(
            "INSERT INTO public_request_messages_map "
              "(public_request_id,message_archive_id,mail_archive_id) "
              "VALUES($1::bigint,$2::bigint,NULL)",
            Database::query_param_list(publicRequestId)(newLetterId)
        );
        tx.commit();
        return newLetterId;
    }
    catch (ccReg::Admin::ObjectNotFound&) {
        throw;
    }
    catch (ccReg::Admin::InternalServerError&) {
        throw;
    }
    catch (ccReg::Admin::OBJECT_NOT_FOUND&) {
        throw ccReg::Admin::ObjectNotFound();
    }
    catch (ccReg::Admin::SQL_ERROR&) {
        throw ccReg::Admin::InternalServerError();
    }
    catch (Database::NoDataFound &ex) {
        LOGGER.error(boost::format("Database::NoDataFound: %1%") % ex.what());
        throw ccReg::Admin::ObjectNotFound();
    }
    catch (LibFred::Messages::MessageCopyProhibited &ex)
    {
        LOGGER.error(boost::format("%1%") % ex.what());
        throw ccReg::Admin::MessageCopyProhibited();
    }
    catch (Database::Exception &ex) {
        LOGGER.error(boost::format("Database problem: %1%") % ex.what());
        throw ccReg::Admin::InternalServerError();
    }
    catch (std::exception &ex) {
        LOGGER.error(boost::format("Internal error: %1%") % ex.what());
        throw ccReg::Admin::InternalServerError();
    }
    catch (...) {
        throw ccReg::Admin::InternalServerError();
    }
}

ccReg::TID ccReg_Admin_i::resendPin2SMS(ccReg::TID publicRequestId)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Admin_i::resendPin2SMS(%1%)") % publicRequestId);

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Database::Result res = conn.exec_params(
            "SELECT (SELECT name "            // [0] - type of request
                    "FROM enum_public_request_type WHERE id=pr.request_type),"
                   "(SELECT name "            // [1] - request status opened/resolved/invalidated
                    "FROM enum_public_request_status WHERE id=pr.status),"
                   "prmm.id IS NULL,"         // [2] - have any message
                   "prmm.message_archive_id," // [3] - sms/letter id
                   "ma.id "                   // [4] - id of pin2 sms
            "FROM public_request pr "
            "LEFT JOIN public_request_messages_map prmm ON prmm.public_request_id=pr.id "
            "LEFT JOIN message_archive ma ON ("
                "ma.id=prmm.message_archive_id AND "
                "ma.message_type_id IN (SELECT id FROM message_type "
                                       "WHERE type IN ('mojeid_pin2','contact_verification_pin2')) AND "
                "ma.comm_type_id=(SELECT id FROM comm_type WHERE type='sms')) "
            "WHERE pr.id=$1::bigint AND "
                  "(prmm.id IS NULL OR prmm.message_archive_id IS NOT NULL) "
                  "ORDER BY prmm.id LIMIT 1",
            Database::query_param_list(publicRequestId)
        );
        if (res.size() <= 0) {
            LOGGER.error(boost::format("publicRequestId: %1% not found") % publicRequestId);
            throw ccReg::Admin::ObjectNotFound();
        }
        const std::string typeOfRequest = static_cast< std::string >(res[0][0]);
        if ((typeOfRequest != "mojeid_contact_conditional_identification") &&
            (typeOfRequest != Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION)) {
            LOGGER.error(boost::format("publicRequestId: %1% of %2% type is not PIN2 request")
                % publicRequestId
                % typeOfRequest);
            throw ccReg::Admin::ObjectNotFound();
        }
        const std::string requestStatus = static_cast< std::string >(res[0][1]);
        if (requestStatus != Conversion::Enums::to_db_handle(LibFred::PublicRequest::Status::opened)) {
            LOGGER.error(
                boost::format("publicRequestId: %1% in %2% state is not new PIN2 request")
                % publicRequestId
                % requestStatus);
            throw ccReg::Admin::ObjectNotFound();
        }
        if (static_cast< bool >(res[0][2])) {
            LOGGER.error(boost::format("publicRequestId: %1% doesn't have message")
                % publicRequestId);
            throw ccReg::Admin::ObjectNotFound();
        }
        if (res[0][3].isnull()) {
            LOGGER.error(
                boost::format("publicRequestId: %1% doesn't have message_archive_id")
                % publicRequestId);
            throw ccReg::Admin::ObjectNotFound();
        }
        if (res[0][4].isnull()) {
            LOGGER.error(
                boost::format("message_archive_id: %1% doesn't exists")
                % static_cast< ccReg::TID >(res[0][3]));
            throw ccReg::Admin::ObjectNotFound();
        }
        const ccReg::TID smsId = static_cast< ccReg::TID >(res[0][4]);
        LibFred::Messages::ManagerPtr msgMan = LibFred::Messages::create_manager();
        const ccReg::TID newSmsId = msgMan->copy_sms_to_send(smsId);
        conn.exec_params(
            "INSERT INTO public_request_messages_map "
              "(public_request_id,message_archive_id,mail_archive_id) "
              "VALUES($1::bigint,$2::bigint,NULL)",
            Database::query_param_list(publicRequestId)(newSmsId)
        );
        tx.commit();
        return newSmsId;
    }
    catch (ccReg::Admin::ObjectNotFound&) {
        throw;
    }
    catch (ccReg::Admin::InternalServerError&) {
        throw;
    }
    catch (ccReg::Admin::OBJECT_NOT_FOUND&) {
        throw ccReg::Admin::ObjectNotFound();
    }
    catch (ccReg::Admin::SQL_ERROR&) {
        throw ccReg::Admin::InternalServerError();
    }
    catch (Database::NoDataFound &ex) {
        LOGGER.error(boost::format("Database::NoDataFound: %1%") % ex.what());
        throw ccReg::Admin::ObjectNotFound();
    }
    catch (Database::Exception &ex) {
        LOGGER.error(boost::format("Database problem: %1%") % ex.what());
        throw ccReg::Admin::InternalServerError();
    }
    catch (std::exception &ex) {
        LOGGER.error(boost::format("Internal error: %1%") % ex.what());
        throw ccReg::Admin::InternalServerError();
    }
    catch (...) {
        throw ccReg::Admin::InternalServerError();
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
      LOGGER.error(boost::format("Database problem: %1%") % ex.what());
      throw ccReg::Admin::InternalServerError();
  }
  catch (std::exception &ex) {
    LOGGER.error(boost::format("Internal error: %1%") % ex.what());
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
      LOGGER.error(boost::format("Database problem: %1%") % ex.what());
      throw ccReg::Admin::InternalServerError();
  }
  catch (std::exception &ex) {
    LOGGER.error(boost::format("Internal error: %1%") % ex.what());
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
      LOGGER.error(boost::format("Database problem: %1%") % ex.what());
      throw ccReg::Admin::InternalServerError();
  }
  catch (std::exception &ex) {
    LOGGER.error(boost::format("Internal error: %1%") % ex.what());
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///create registrar certification
        return regman->createRegistrarCertification(
                reg_id
                , Database::Date(makeBoostDate_throw(from))
                , Database::Date(makeBoostDate(to))
                , static_cast<LibFred::Registrar::RegCertClass>(score)
                , evaluation_file_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///shorten registrar certification
        return regman->shortenRegistrarCertification(
                cert_id
                , Database::Date(makeBoostDate(to)));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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

        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///update registrar certification
        return regman->updateRegistrarCertification(
                cert_id
                , static_cast<LibFred::Registrar::RegCertClass>(score)
                , evaluation_file_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///get registrar certification
        LibFred::Registrar::CertificationSeq cs
        = regman->getRegistrarCertifications(registrar_id);

        std::unique_ptr<Registry::Registrar::Certification::CertificationList> cl
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
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///create group
        return regman->createRegistrarGroup(std::string(name));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//createGroup

void Registry_Registrar_Group_Manager_i::deleteGroup(ccReg::TID group_id)
{
    Logging::Context ctx("adifd");
    ConnectionReleaser releaser;

    try
    {
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///delete group
        regman->cancelRegistrarGroup(group_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///update group
        regman->updateRegistrarGroup(group_id, std::string(name));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///create membership of registrar in group
        return regman->createRegistrarGroupMembership(
                reg_id
                , group_id
                , Database::Date(NOW)
                , Database::Date(POS_INF));
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///end membership of registrar in group
        regman->endRegistrarGroupMembership(
                reg_id
                , group_id);
    }//try
    catch(const std::exception & ex)
    {
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///get registrar certification
        LibFred::Registrar::GroupSeq gs
        = regman->getRegistrarGroups();

        std::unique_ptr<Registry::Registrar::Group::GroupList> gl
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
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw;// Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///get registrar certification
        LibFred::Registrar::MembershipByRegistrarSeq mbrs
        = regman->getMembershipByRegistrar(registrar_id);

        std::unique_ptr<Registry::Registrar::Group::MembershipByRegistrarList> mbrl
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
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
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
        DBSharedPtr nodb;
        LibFred::Registrar::Manager::AutoPtr regman(
                LibFred::Registrar::Manager::create(nodb));
        ///get registrar certification
        LibFred::Registrar::MembershipByGroupSeq mbgs
        = regman->getMembershipByGroup(group_id);

        std::unique_ptr<Registry::Registrar::Group::MembershipByGroupList> mbgl
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
        LOGGER.debug(boost::format("exception: %1%") % ex.what());
        throw Registry::Registrar::InvalidValue(CORBA::string_dup(ex.what()));
    }//catch std ex
    catch(...)
    {
        LOGGER.debug("exception");
        throw Registry::Registrar::InternalServerError();
    }//catch all
}//getMembershipsByGroup


ccReg::EnumList* ccReg_Admin_i::getBankAccounts()
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  LOGGER.debug("ccReg_Admin_i::getBankAccounts");
  try
  {
      LibFred::Banking::EnumList el = LibFred::Banking::getBankAccounts();
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


ccReg::RegistrarRequestCountInfo* ccReg_Admin_i::getRegistrarRequestCount(const char* _registrar)
{
    try {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(server_name_);
        const unsigned long long registrar_id =
            ::LibFred::InfoRegistrarByHandle(_registrar).exec(ctx).info_registrar_data.id;
        const LibFred::Poll::RequestFeeInfoEvent request_fee_info =
            LibFred::Poll::get_last_request_fee_info_message(ctx, registrar_id);

        ccReg::RegistrarRequestCountInfo_var ret = new ccReg::RegistrarRequestCountInfo;
        ret->periodFrom = LibFred::Corba::wrap_string_to_corba_string(
                formatTime(request_fee_info.from, true,true).c_str());
        ret->periodTo = LibFred::Corba::wrap_string_to_corba_string(
                formatTime(request_fee_info.to - boost::posix_time::seconds(1), true, true).c_str());
        LibFred::Corba::wrap_int(request_fee_info.free_count, ret->totalFreeCount);
        LibFred::Corba::wrap_int(request_fee_info.used_count, ret->usedCount);
        ret->price = LibFred::Corba::wrap_string_to_corba_string(request_fee_info.price.get_string().c_str());

        return ret._retn();
    }
    catch (LibFred::NOT_FOUND&) {
        throw ccReg::Admin::ObjectNotFound();
    }
    catch (std::exception &ex) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().error(ex.what());
        throw ccReg::Admin::InternalServerError();
    }
    catch (...) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().error("unknown error");
        throw ccReg::Admin::InternalServerError();
    }
}

bool ccReg_Admin_i::isRegistrarBlocked(ccReg::TID reg_id)
{
    try {
        Logging::Context(server_name_);
        ConnectionReleaser release;
        TRACE(boost::format("[CALL] ccReg_Admin_i::isRegistrarBlocked(%1%)") % reg_id);

        DBSharedPtr nodb;
        std::unique_ptr<LibFred::Registrar::Manager> regman(
                LibFred::Registrar::Manager::create(nodb));

        regman->checkRegistrarExists(reg_id);
        return regman->isRegistrarBlocked(reg_id);
    } catch (LibFred::NOT_FOUND &) {
        throw ccReg::Admin::ObjectNotFound();
    } catch (std::exception &ex) {
        LOGGER.error(ex.what());
        throw ccReg::Admin::InternalServerError();
    } catch (...) {
        LOGGER.error("unknown error in isRegistrarBlocked");
        throw ccReg::Admin::InternalServerError();
    }

}

bool ccReg_Admin_i::blockRegistrar(ccReg::TID reg_id)
{
    try {
        Logging::Context(server_name_);
        ConnectionReleaser release;
        TRACE(boost::format("[CALL] ccReg_Admin_i::blockRegistrar(%1%)") % reg_id);

        DBSharedPtr nodb;
        std::unique_ptr<LibFred::Registrar::Manager> regman(
                LibFred::Registrar::Manager::create(nodb));

        regman->checkRegistrarExists(reg_id);

        std::unique_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());
        return regman->blockRegistrar(reg_id, epp_cli.get());
    } catch (LibFred::NOT_FOUND &) {
        throw ccReg::Admin::ObjectNotFound();
    } catch (std::exception &ex) {
        LOGGER.error(ex.what());
        throw ccReg::Admin::InternalServerError();
    } catch (...) {
        LOGGER.error("unknown error in blockRegistrar");
        throw ccReg::Admin::InternalServerError();
    }

}

void ccReg_Admin_i::unblockRegistrar(ccReg::TID reg_id, ccReg::TID request_id)
{
    try {
        Logging::Context(server_name_);
        ConnectionReleaser release;
        TRACE(boost::format("[CALL] ccReg_Admin_i::unblockRegistrar(%1%, %2%)") % reg_id % request_id);

        DBSharedPtr nodb;
        std::unique_ptr<LibFred::Registrar::Manager> regman(
                LibFred::Registrar::Manager::create(nodb));

        regman->checkRegistrarExists(reg_id);
        regman->unblockRegistrar(reg_id, request_id);
    } catch (LibFred::NOT_FOUND &) {
        throw ccReg::Admin::ObjectNotFound();
    } catch (LibFred::NOT_BLOCKED &) {
        throw ccReg::Admin::ObjectNotBlocked();
    } catch (std::exception &ex) {
        LOGGER.error(ex.what());
        throw ccReg::Admin::InternalServerError();
    } catch (...) {
        LOGGER.error("unknown error in unblockRegistrar");
        throw ccReg::Admin::InternalServerError();
    }
}

ccReg::ULLSeq* ccReg_Admin_i::getSummaryOfExpiredDomains(const char *registrar_handle, const ccReg::DatePeriodList &date_intervals)
{
    try {
        Logging::Context(server_name_);
        ConnectionReleaser release;
        TRACE(boost::format("[CALL] ccReg_Admin_i::getSummaryOfExpiredDomains(%1%, date_intervals") % registrar_handle );

        // convert 2nd parametre - the sequence
        std::vector<boost::gregorian::date_period> intervals;
        intervals.reserve(date_intervals.length());

        for(unsigned i=0; i<date_intervals.length(); i++) {
            intervals.push_back(date_period(
                        makeBoostDate_throw(date_intervals[i].from),
                        makeBoostDate_throw(date_intervals[i].to))
            );
        }

        // call implementation
        std::vector<unsigned long long> counts = LibFred::Domain::getExpiredDomainSummary(registrar_handle, intervals);

        // convert return value
        ccReg::ULLSeq_var ret = new ccReg::ULLSeq();

        ret->length(counts.size());
        for (unsigned i=0; i<counts.size(); i++) {
            ret[i] = counts[i];
        }

        return ret._retn();

    } catch (LibFred::INVALID_VALUE &ex) {
        throw ccReg::Admin::InvalidValue(ex.what());
    } catch (LibFred::NOT_FOUND &) {
        throw ccReg::Admin::ObjectNotFound();
    } catch (std::exception &ex) {
        LOGGER.error(ex.what());
        throw ccReg::Admin::InternalServerError();
    } catch (...) {
        LOGGER.error("unknown exception in getSummaryOfExpiredDomains");
        throw ccReg::Admin::InternalServerError();
    }
}

