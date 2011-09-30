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

#include <memory> ///< auto_ptr<>
#include <algorithm>
#include <boost/shared_ptr.hpp>

#include "old_utils/dbsql.h"
#include "registry.h"
#include "zone.h"
#include "domain.h"
#include "filter.h"
#include "db_settings.h"
#include "log/logger.h"

namespace Fred {

class StatusDescImpl : public virtual StatusDesc {
  TID id;
  std::string name;
  bool external;
  std::map<std::string,std::string> desc;
  bool contact;
  bool nsset;
  bool domain;
  bool keyset;

public:
  StatusDescImpl(TID _id,
                 const std::string& _name,
                 bool _external,
                 const std::string& types) :
    id(_id), name(_name), external(_external) {
    contact = types.find("1") != std::string::npos;
    nsset = types.find("2") != std::string::npos;
    domain = types.find("3") != std::string::npos;
    keyset = types.find("4") != std::string::npos;
  }

  void addDesc(const std::string& lang, const std::string text) {
    desc[lang] = text;
  }

  bool operator==(TID _id) const {
    return id == _id;
  }

  virtual TID getId() const {
    return id;
  }

  virtual const std::string& getName() const {
    return name;
  }

  virtual bool getExternal() const {
    return external;
  }

  virtual const std::string& getDesc(const std::string& lang) const
      throw (BAD_LANG) {
    std::string lang_upper = lang;
    boost::algorithm::to_upper(lang_upper);
    std::map<std::string,std::string>::const_iterator i = desc.find(lang_upper);
    if (i == desc.end())
      throw BAD_LANG();
    return i->second;
  }

  virtual bool isForType(short type) const {
    //return type == 1 ? contact : type == 2 ? nsset : domain;
    return type == 1 ? contact : type == 2 ? nsset : type == 3 ? domain : keyset;
  }
};

class ManagerImpl : virtual public Manager {
  DBSharedPtr db;
  bool m_restricted_handles;


  Messages::ManagerPtr m_message_manager;
  Zone::Manager::ZoneManagerPtr m_zone_manager;
  std::auto_ptr<Domain::Manager> m_domain_manager;
  Registrar::Manager::AutoPtr m_registrar_manager;
  std::auto_ptr<Contact::Manager> m_contact_manager;
  std::auto_ptr<NSSet::Manager> m_nsset_manager;
  std::auto_ptr<KeySet::Manager> m_keyset_manager;
  std::auto_ptr<Filter::Manager> m_filter_manager;

  std::vector<CountryDesc> m_countries;
  std::vector<StatusDescImpl> statusList;

public:
  ManagerImpl(DBSharedPtr _db, bool _restrictedHandles)
    : db(_db)
    , m_restricted_handles(_restrictedHandles)
    , m_message_manager(Messages::create_manager())
    , m_zone_manager(Zone::Manager::create())
    , m_registrar_manager(Registrar::Manager::create(db))
  {
    m_domain_manager.reset(Domain::Manager::create(db, m_zone_manager.get()));
    m_contact_manager.reset(Contact::Manager::create(db, m_restricted_handles));
    m_nsset_manager.reset(NSSet::Manager::create(db,
                                                 m_zone_manager.get(),
                                                 m_restricted_handles));
    m_keyset_manager.reset(KeySet::Manager::create(db, m_restricted_handles));
	
    // TEMP: this will be ok when DBase::Manager ptr will be initilized
    // here in constructor (not in dbManagerInit method)
    // m_filter_manager.reset(Filter::Manager::create(m_db_manager));

    CountryDesc cd;
    cd.cc = "CZ";
    cd.name = "Czech Republic";
    m_countries.push_back(cd);
    cd.cc = "SK";
    cd.name = "Slovak Republic";
    m_countries.push_back(cd);
  }//ManagerImpl


  Messages::ManagerPtr getMessageManager() const {
    return m_message_manager;
  }

  Zone::Manager *getZoneManager() const {
    return m_zone_manager.get();
  }

  Domain::Manager *getDomainManager() const {
    return m_domain_manager.get();
  }

  Registrar::Manager *getRegistrarManager() const {
    return m_registrar_manager.get();
  }

  Contact::Manager *getContactManager() const {
    return m_contact_manager.get();
  }

  NSSet::Manager *getNSSetManager() const {
    return m_nsset_manager.get();
  }

  KeySet::Manager *getKeySetManager() const
  {
      return m_keyset_manager.get();
  }

  Filter::Manager* getFilterManager() const {
    return m_filter_manager.get();
  }

  /// interface method implementation
  void checkHandle(const std::string& handle, 
									 CheckHandleList& chl, 
									 bool allowIDN) const {
    CheckHandle ch;
    bool isEnum = false;
    try {
      // trying convert string to enum domain
      ch.newHandle = m_zone_manager->makeEnumDomain(handle);
      ch.type = HT_ENUM_NUMBER;
      isEnum = true;
    } catch (...) {}
    bool isDomain = true;
    std::string fqdn = isEnum ? ch.newHandle : handle;
     if (allowIDN)
       fqdn = m_zone_manager->encodeIDN(fqdn);
    NameIdPair conflictFQDN;
    switch (m_domain_manager->checkAvail(fqdn, conflictFQDN, false, allowIDN)) {
      case Domain::CA_INVALID_HANDLE:
        isDomain = false;
        break;
      case Domain::CA_BAD_LENGHT:
        ch.handleClass = CH_UNREGISTRABLE_LONG;
        break;
      case Domain::CA_BAD_ZONE:
        ch.handleClass = CH_UNREGISTRABLE;
        break;
      case Domain::CA_BLACKLIST:
        ch.handleClass = CH_PROTECTED;
        break;
      case Domain::CA_REGISTRED:
        ch.handleClass = CH_REGISTRED;
        break;
      case Domain::CA_PARENT_REGISTRED:
        ch.handleClass = CH_REGISTRED_PARENT;
        break;
      case Domain::CA_CHILD_REGISTRED:
        ch.handleClass = CH_REGISTRED_CHILD;
        break;
      case Domain::CA_AVAILABLE:
        ch.handleClass = CH_FREE;
        break;
    }
    ch.conflictHandle = conflictFQDN.name;
    if (allowIDN)
       ch.conflictHandle = m_zone_manager->decodeIDN(ch.conflictHandle);
    if (isDomain) {
      if (!isEnum)
        ch.type = m_zone_manager->checkEnumDomainSuffix(fqdn) ? HT_ENUM_DOMAIN
                                                              : HT_DOMAIN;
      chl.push_back(ch);
    }
    // check if handle is registred contact
    NameIdPair conflictContact;
    if (getContactManager()->checkAvail(handle, conflictContact)
        == Contact::Manager::CA_REGISTRED) {
      CheckHandle chCon;
      chCon.type = HT_CONTACT;
      chCon.handleClass= CH_REGISTRED;
      chl.push_back(chCon);
    }
    // check if handle is registred nsset   
    NameIdPair conflictNSSet;
    if (getNSSetManager()->checkAvail(handle, conflictNSSet) == NSSet::Manager::CA_REGISTRED) {
      CheckHandle chNss;
      chNss.type = HT_NSSET;
      chNss.handleClass= CH_REGISTRED;
      chl.push_back(chNss);
    }
    // check if handle is registered keyset
    NameIdPair conflictKeySet;
    if (getKeySetManager()->checkAvail(handle, conflictKeySet) ==
            KeySet::Manager::CA_REGISTRED) {
        CheckHandle chKey;
        chKey.type = HT_KEYSET;
        chKey.handleClass = CH_REGISTRED;
        chl.push_back(chKey);
    }
    // check if handle is registrant   
    if (getRegistrarManager()->checkHandle(handle)) {
      CheckHandle chReg;
      chReg.type = HT_REGISTRAR;
      chReg.handleClass= CH_REGISTRED;
      chl.push_back(chReg);
    }
    // if empty return OTHER   
    if (!chl.size()) {
      CheckHandle chOth;
      chOth.type = HT_OTHER;
      chOth.handleClass= CH_FREE;
      chl.push_back(chOth);
    }
  }

  virtual void loadCountryDesc() {
    TRACE("[CALL] Fred::Manager::loadCountryDesc()");
    Database::SelectQuery country_query;
    country_query.select() << "id, country_cs, country";
    country_query.from() << "enum_country";

    try {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result r_country = conn.exec(country_query);
      
      m_countries.clear();
      for (Database::Result::Iterator it = r_country.begin(); it != r_country.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();
        CountryDesc desc;
        
        std::string cc      = *col;
        std::string name_cs = *(++col);
        std::string name    = *(++col);
        
        desc.cc = cc;
        desc.name = (!name_cs.empty() ? name_cs : name);
        m_countries.push_back(desc);
      }
      LOGGER(PACKAGE).debug(boost::format("loaded '%1%' country codes "
              "description from database") % r_country.size());
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }

  virtual unsigned getCountryDescSize() const {
    return m_countries.size();
  }

  virtual const CountryDesc& getCountryDescByIdx(unsigned idx) const
      throw (NOT_FOUND) {
    if (idx >= m_countries.size())
      throw NOT_FOUND();
    return m_countries[idx];
  }

  virtual void initStates() throw (SQL_ERROR) {
    TRACE("[CALL] Fred::Manager::initStates()");

    statusList.clear();

    /// HACK: OK state
    statusList.push_back(StatusDescImpl(0, "ok", true, "1,2,3,4"));
    statusList.back().addDesc("CS", "Objekt je bez omezení");
    statusList.back().addDesc("EN", "Objekt is without restrictions");

    if (!db->ExecSelect("SELECT id, name, external, ARRAY_TO_STRING(types,',') "
      "FROM enum_object_states ORDER BY id") )
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      statusList.push_back(StatusDescImpl(
      STR_TO_ID(db->GetFieldValue(i,0)),
      db->GetFieldValue(i,1),
      *db->GetFieldValue(i,2) == 't',
      db->GetFieldValue(i,3)) );
    }
    unsigned states_loaded = db->GetSelectRows();
    db->FreeSelect();
    
    if (!db->ExecSelect("SELECT state_id, lang, description FROM enum_object_states_desc"))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      std::vector<StatusDescImpl>::iterator it = find(statusList.begin(),
                                                     statusList.end(),
                                                      STR_TO_ID(db->GetFieldValue(i,0)));
      if (it != statusList.end())
        it->addDesc(db->GetFieldValue(i, 1), db->GetFieldValue(i, 2));

    }
    db->FreeSelect();
    
    LOGGER(PACKAGE).debug(boost::format("loaded '%1%' object states description from database")
        % states_loaded);
  }

  virtual const StatusDesc* getStatusDesc(TID status) const {
    std::vector<StatusDescImpl>::const_iterator it = find(statusList.begin(),
                                                         statusList.end(),
                                                         status);
    if (it == statusList.end())
      return NULL;
    return &(*it);
  }

  virtual const StatusDesc* getStatusDesc(const std::string &_name) const
  {
      for (std::vector<StatusDescImpl>::const_iterator it = statusList.begin();
              it != statusList.end();
              ++it)
      {
          if (it->getName() == _name) {
              return &(*it);
          }
      }
      return 0;
  }

  virtual unsigned getStatusDescCount() const {
    return statusList.size();
  }

  virtual const StatusDesc* getStatusDescByIdx(unsigned idx) const {
    if (idx >= statusList.size())
      return NULL;
    return &statusList[idx];
  }

  virtual void updateObjectStates(unsigned long long _id) //const throw (SQL_ERROR)
  {
      Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("ManagerImpl::updateObjectStates _id: ")
       + boost::lexical_cast<std::string>(_id));

    TRACE("[CALL] Fred::Manager::updateObjectStates()");
    std::stringstream sql;
    sql << "SELECT update_object_states(" << _id << ")";
    Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("ManagerImpl::updateObjectStates sql: ")
     + sql.str());

    if (!db->ExecSelect(sql.str().c_str())) {
      LOGGER(PACKAGE).error("updateObjectStates(): throwing SQL_ERROR");
      throw SQL_ERROR();
    }
    db->FreeSelect();
  }

  /// TEMP: method for initialization new Database manager
  virtual void dbManagerInit() {
    m_filter_manager.reset(Filter::Manager::create());
    
    /// load country codes descrition from database
    loadCountryDesc();
  }
};

Manager *Manager::create(DBSharedPtr db, bool _restrictedHandles) {
  TRACE("[CALL] Fred::Manager::create()");
  return new ManagerImpl(db, _restrictedHandles);
}

}//namespace Fred
