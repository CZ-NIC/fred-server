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

#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>
#include <vector>

#include "nsset.h"
#include "object_impl.h"
#include "sql.h"
#include "old_utils/dbsql.h"
#include "old_utils/util.h"
#include "db/dbs.h"
#include "model/model_filters.h"
#include "log/logger.h"

#define NSSET_REGEX_RESTRICTED "[nN][sS][sS][iI][dD]:[a-zA-Z0-9_:.-]{1,57}"
#define NSSET_REGEX "[a-zA-Z0-9_:.-]{1,63}"

namespace Register {
namespace NSSet {
static boost::regex format(NSSET_REGEX);
static boost::regex formatRestricted(NSSET_REGEX_RESTRICTED);

class HostImpl : public virtual Host {
  typedef std::vector<std::string> AddrListType;
  AddrListType addrList;
  std::string name;
  std::string nameIDN;
public:
  HostImpl(const std::string& _name, Zone::Manager *zm) : 
  		name(_name), nameIDN(zm->decodeIDN(name)) {
  }
  virtual const std::string getName() const {
		return name;
  }      
  virtual const std::string getNameIDN() const {
		return nameIDN;
	}

  /// must not be reference because of find_if algo (ref. to ref. problem)
  bool hasName(std::string _name) const {
    return name == _name;
  }
  virtual unsigned getAddrCount() const {
    return addrList.size();
  }
  virtual std::string getAddrByIdx(unsigned idx) const {
    if (idx >= getAddrCount())
      return "";
    else
      return addrList[idx];
  }
  void addAddr(const std::string& addr) {
    if (!addr.empty())
      addrList.push_back(addr);
  }
};
class NSSetImpl : public ObjectImpl, public virtual NSSet {
  std::string handle;
  typedef std::vector<std::string> ContactListType;
  typedef std::vector<HostImpl> HostListType;
  ContactListType admins;
  HostListType hosts;
  unsigned checkLevel;
  Zone::Manager *zm;
public:
  NSSetImpl(TID _id, const std::string& _handle, TID _registrar,
      const std::string& _registrarHandle, ptime _crDate, ptime _trDate,
      ptime _upDate, date _erDate, TID _createRegistrar,
      const std::string& _createRegistrarHandle, TID _updateRegistrar,
      const std::string& _updateRegistrarHandle, const std::string& _authPw,
      const std::string& _roid, unsigned _checkLevel, Zone::Manager *_zm) :
    ObjectImpl(_id, _crDate, _trDate, _upDate, _erDate, _registrar, 
               _registrarHandle, _createRegistrar, _createRegistrarHandle, 
               _updateRegistrar, _updateRegistrarHandle, 
               _authPw, _roid), handle(_handle), checkLevel(_checkLevel),
               zm(_zm) {
  }
  const std::string& getHandle() const {
    return handle;
  }
  virtual const unsigned getCheckLevel() const {
    return checkLevel;
  }
  unsigned getAdminCount() const {
    return admins.size();
  }
  std::string getAdminByIdx(unsigned idx) const {
    if (idx>=admins.size())
      return "";
    else
      return admins[idx];
  }
  unsigned getHostCount() const {
    return hosts.size();
  }
  const Host *getHostByIdx(unsigned idx) const {
    if (idx>=hosts.size())
      return NULL;
    else
      return &hosts[idx];
  }
  void addAdminHandle(const std::string& admin) {
    admins.push_back(admin);
  }
  HostImpl *addHost(const std::string& name) {
    HostListType::iterator i = find_if(hosts.begin(), hosts.end(),
        std::bind2nd(std::mem_fun_ref(&HostImpl::hasName), name) );
    if (i == hosts.end())
      hosts.push_back(HostImpl(name, zm));
    else
      return &(*i);
    return &hosts.back();
  }
  bool hasId(TID _id) {
    return id_ == _id;
  }
};


COMPARE_CLASS_IMPL(NSSetImpl, Handle)
COMPARE_CLASS_IMPL(NSSetImpl, CreateDate)
COMPARE_CLASS_IMPL(NSSetImpl, DeleteDate)
COMPARE_CLASS_IMPL(NSSetImpl, RegistrarHandle)


class ListImpl : public virtual List, public ObjectListImpl {
  std::string handle;
  std::string hostname;
  std::string ip;
  std::string admin;
  Zone::Manager *zm;
public:
  ListImpl(DB *_db, Zone::Manager *_zm) :
    ObjectListImpl(_db), zm(_zm) {
  }
  NSSet *getNSSet(unsigned idx) const {
    return dynamic_cast<NSSet *>(get(idx));
  }
  void setHandleFilter(const std::string& _handle) {
    handle = _handle;
  }
  void setHostNameFilter(const std::string& name) {
    hostname = name;
    nonHandleFilterSet = true;
  }
  void setHostIPFilter(const std::string& _ip) {
    ip = _ip;
    nonHandleFilterSet = true;
  }
  void setAdminFilter(const std::string& handle) {
    admin = handle;
    nonHandleFilterSet = true;
  }
  void makeQuery(bool count, bool limit, std::stringstream& sql) const {
    std::stringstream from, where;
    sql.str("");
    if (!count)
      sql << "INSERT INTO " << getTempTableName() << " ";
    sql << "SELECT " << (count ? "COUNT(" : "") << "DISTINCT n.id"
        << (count ? ") " : " ");
    from << "FROM nsset n ";
    where << "WHERE 1=1 ";
    SQL_ID_FILTER(where, "n.id", idFilter);
    if (registrarFilter || !registrarHandleFilter.empty()
        || updateRegistrarFilter || !updateRegistrarHandleFilter.empty()
        || TIME_FILTER_SET(updateIntervalFilter)
        || TIME_FILTER_SET(trDateIntervalFilter) ) {
      from << ",object o ";
      where << "AND n.id=o.id ";
      SQL_ID_FILTER(where, "o.clid", registrarFilter);
      SQL_ID_FILTER(where, "o.upid", updateRegistrarFilter);
      SQL_DATE_FILTER(where, "o.upDate", updateIntervalFilter);
      SQL_DATE_FILTER(where, "o.trDate", trDateIntervalFilter);
      if (!registrarHandleFilter.empty()) {
        from << ",registrar reg ";
        where << "AND o.clid=reg.id ";
        SQL_HANDLE_WILDCHECK_FILTER(where, "reg.handle", registrarHandleFilter,
            wcheck, false);
      }
      if (!updateRegistrarHandleFilter.empty()) {
        from << ",registrar ureg ";
        where << "AND o.upid=ureg.id ";
        SQL_HANDLE_WILDCHECK_FILTER(where, "ureg.handle",
            updateRegistrarHandleFilter, wcheck, false);
      }
    }
    if (createRegistrarFilter || !createRegistrarHandleFilter.empty()
        || TIME_FILTER_SET(crDateIntervalFilter) || !handle.empty()) {
      from << ",object_registry obr ";
      where << "AND obr.id=n.id AND obr.type=2 ";
      SQL_ID_FILTER(where, "obr.crid", createRegistrarFilter);
      SQL_DATE_FILTER(where, "obr.crdate", crDateIntervalFilter);
      SQL_HANDLE_WILDCHECK_FILTER(where, "obr.name", handle, wcheck, true);
      if (!createRegistrarHandleFilter.empty()) {
        from << ",registrar creg ";
        where << "AND obr.crid=creg.id ";
        SQL_HANDLE_WILDCHECK_FILTER(where, "creg.handle",
            createRegistrarHandleFilter, wcheck, false);
      }
    }
    if (!admin.empty()) {
      from << ",nsset_contact_map ncm ";
      where << "AND n.id=ncm.nssetid ";
      // preprared for addition of admin ID filter
      if (!admin.empty()) {
        from << ",object_registry ncor ";
        where << "AND ncm.contactid=ncor.id AND ncor.type=1 ";
        SQL_HANDLE_WILDCHECK_FILTER(where, "ncor.name", admin, wcheck, true);
      }
    }
    if (!hostname.empty()) {
      from << ",host h ";
      where << "AND n.id=h.nssetid ";
      SQL_HANDLE_FILTER(where, "h.fqdn", hostname);
    }
    if (!ip.empty()) {
      from << ",host_ipaddr_map him ";
      where << "AND n.id=him.nssetid ";
      SQL_HANDLE_FILTER(where, "host(him.ipaddr)", ip);
    }
    if (!count)
      where << "ORDER BY n.id ASC ";
    if (limit)
      where << "LIMIT " << load_limit_ << " ";
    sql << from.rdbuf();
    sql << where.rdbuf();
  }
  void reload() throw (SQL_ERROR) {
    std::map<TID,std::string> registrars;
    std::ostringstream sql;
    sql << "SELECT id, handle FROM registrar";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      registrars[STR_TO_ID(db->GetFieldValue(i,0))] = db->GetFieldValue(i, 1);
    }
    db->FreeSelect();
    sql.str("");
    clear();
    bool useTempTable = nonHandleFilterSet || handle.empty(); 
    if (useTempTable)
      fillTempTable(true);
    sql << "SELECT " << "obr.id,obr.name," << "o.clid,"
        << "obr.crdate,o.trdate,o.update,"
        << "obr.crid,o.upid,o.authinfopw,obr.roid,n.checklevel " << "FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "nsset n, object_registry obr, object o "
        << "WHERE tmp.id=n.id AND n.id=o.id AND obr.id=o.id ";
    if (!useTempTable) {
      sql << "AND tmp.name=UPPER('" << db->Escape2(handle) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=2 "; 
    }
    sql << "ORDER BY tmp.id ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      data_.push_back(new NSSetImpl(
          STR_TO_ID(db->GetFieldValue(i,0)), // nsset id
          db->GetFieldValue(i,1), // nsset handle
          STR_TO_ID(db->GetFieldValue(i,2)), // registrar id
          registrars[STR_TO_ID(db->GetFieldValue(i,2))], // reg. handle
          MAKE_TIME(i,3), // registrar crdate
          MAKE_TIME(i,4), // registrar trdate
          MAKE_TIME(i,5), // registrar update
          date(not_a_date_time),
          STR_TO_ID(db->GetFieldValue(i,6)), // crid 
          registrars[STR_TO_ID(db->GetFieldValue(i,6))], // crid handle
          STR_TO_ID(db->GetFieldValue(i,7)), // upid
          registrars[STR_TO_ID(db->GetFieldValue(i,7))], // upid handle
          db->GetFieldValue(i,8), // authinfo
          db->GetFieldValue(i,9), // roid
          atoi(db->GetFieldValue(i,10)), // checklevel
          zm
      ));
    }
    db->FreeSelect();
    // no need to proceed when nothing was loaded
    if (!getCount())
      return;
    resetIDSequence();
    sql.str("");
    sql << "SELECT n.nssetid, cor.name " << "FROM " 
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "nsset_contact_map n, object_registry cor "
        << "WHERE tmp.id=n.nssetid AND n.contactid = cor.id ";
    if (!useTempTable) {
      sql << "AND tmp.name=UPPER('" << db->Escape2(handle) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=2 "; 
    }
    sql << "ORDER BY tmp.id, cor.id ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      NSSetImpl *ns =
          dynamic_cast<NSSetImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
              i, 0)) ));
      if (!ns)
        throw SQL_ERROR();
      ns->addAdminHandle(db->GetFieldValue(i, 1));
    }
    db->FreeSelect();
    resetIDSequence();
    sql.str("");
    sql << "SELECT h.nssetid, h.fqdn, him.ipaddr " << "FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "host h LEFT JOIN host_ipaddr_map him ON (h.id=him.hostid) "
        << "WHERE tmp.id=h.nssetid ";
    if (!useTempTable) {
      sql << "AND tmp.name=UPPER('" << db->Escape2(handle) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=2 "; 
    }
    sql << "ORDER BY tmp.id, h.id, him.id ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      NSSetImpl *ns =
          dynamic_cast<NSSetImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
              i, 0)) ));
      if (!ns)
        throw SQL_ERROR();
      HostImpl* h = ns->addHost(db->GetFieldValue(i, 1));
      h->addAddr(db->GetFieldValue(i, 2));
    }
    db->FreeSelect();
    ObjectListImpl::reload(useTempTable ? NULL : handle.c_str(),2);
  }
  virtual void reload2(DBase::Filters::Union &uf, DBase::Manager* dbm) {
    TRACE("[CALL] NSSet::ListImpl::reload2()");
    clear();
    uf.clearQueries();

    // TEMP: should be cached for quicker
    std::map<DBase::ID, std::string> registrars_table;

    DBase::SelectQuery id_query;
    std::auto_ptr<DBase::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      DBase::Filters::NSSet *df = dynamic_cast<DBase::Filters::NSSet* >(fit->get());
      if (!df)
        continue;
      DBase::SelectQuery *tmp = new DBase::SelectQuery();
      tmp->addSelect(new DBase::Column("historyid", df->joinNSSetTable(), "DISTINCT"));
      uf.addQuery(tmp);
    }
    id_query.limit(5000);
    uf.serialize(id_query);

    DBase::InsertQuery tmp_table_query = DBase::InsertQuery(getTempTableName(),
        id_query);
    LOGGER("db").debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    DBase::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, t_1.name, t_2.clid, t_1.crdate, "
        << "t_2.trdate, t_2.update, t_1.erdate, t_1.crid, t_2.upid, "
        << "t_2.authinfopw, t_1.roid, t_3.checklevel";
//    object_info_query.from() << getTempTableName()
//        << " tmp JOIN nsset t_3 ON (tmp.id = t_3.id) "
//        << "JOIN object t_2 ON (t_3.id = t_2.id) "
//        << "JOIN object_registry t_1 ON (t_1.id = t_2.id)";
    object_info_query.from() << getTempTableName()
        << " tmp JOIN nsset_history t_3 ON (tmp.id = t_3.historyid) "
        << "JOIN object_history t_2 ON (t_3.historyid = t_2.historyid) "
        << "JOIN object_registry t_1 ON (t_1.historyid = t_2.historyid)";
    object_info_query.order_by() << "t_1.id";

    try {
      std::auto_ptr<DBase::Connection> conn(dbm->getConnection());

      DBase::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
      std::auto_ptr<DBase::Result> r_create_tmp_table(conn->exec(create_tmp_table));
      conn->exec(tmp_table_query);

      // TEMP: should be cached somewhere
      DBase::Query registrars_query("SELECT id, handle FROM registrar");
      std::auto_ptr<DBase::Result> r_registrars(conn->exec(registrars_query));
      std::auto_ptr<DBase::ResultIterator> rit(r_registrars->getIterator());
      for (rit->first(); !rit->isDone(); rit->next()) {
        DBase::ID id = rit->getNextValue();
        std::string handle = rit->getNextValue();
        registrars_table[id] = handle;
      }

      std::auto_ptr<DBase::Result> r_info(conn->exec(object_info_query));
      std::auto_ptr<DBase::ResultIterator> it(r_info->getIterator());
      for (it->first(); !it->isDone(); it->next()) {
        DBase::ID nid = it->getNextValue();
        std::string handle = it->getNextValue();
        DBase::ID registrar_id = it->getNextValue();
        std::string registrar_handle = registrars_table[registrar_id];
        DBase::DateTime cr_date = it->getNextValue();
        DBase::DateTime tr_date = it->getNextValue();
        DBase::DateTime up_date = it->getNextValue();
        DBase::Date er_date = it->getNextValue();
        DBase::ID crid = it->getNextValue();
        std::string crid_handle = registrars_table[crid];
        DBase::ID upid = it->getNextValue();
        std::string upid_handle = registrars_table[upid];
        std::string authinfo = it->getNextValue();
        std::string roid = it->getNextValue();
        unsigned check_level = it->getNextValue();
        data_.push_back(
            new NSSetImpl(
                nid,
                handle,
                registrar_id,
                registrar_handle,
                cr_date,
                tr_date,
                up_date,
                er_date,
                crid,
                crid_handle,
                upid,
                upid_handle,
                authinfo,
                roid,
                check_level,
                zm
            ));
      }
      
      if (data_.empty())
        return;
      
      resetIDSequence();
      DBase::SelectQuery contacts_query;
      contacts_query.select() << "tmp.id, t_1.nssetid, t_2.name";
      //  contacts_query.from() << getTempTableName() << " tmp "
      //                        << "JOIN nsset_contact_map t_1 ON (tmp.id = t_1.nssetid) "
      //                        << "JOIN object_registry t_2 ON (t_1.contactid = _t_2.id)";
      contacts_query.from() << getTempTableName() << " tmp "
                            << "JOIN nsset_contact_map_history t_1 ON (tmp.id = t_1.historyid) "
                            << "JOIN object_registry t_2 ON (t_1.contactid = t_2.id)";
      contacts_query.order_by() << "t_1.nssetid, t_2.id ";
      
      std::auto_ptr<DBase::Result> r_contacts(conn->exec(contacts_query));
      std::auto_ptr<DBase::ResultIterator> cit(r_contacts->getIterator());
      for (cit->first(); !cit->isDone(); cit->next()) {
        DBase::ID nsset_historyid = cit->getNextValue();
        DBase::ID nsset_id = cit->getNextValue();
        std::string contact_handle = cit->getNextValue();
        
        NSSetImpl *nsset_ptr = dynamic_cast<NSSetImpl *>(findIDSequence(nsset_id));
        if (nsset_ptr)
          nsset_ptr->addAdminHandle(contact_handle);
      }
      
      resetIDSequence();
      DBase::SelectQuery hosts_query;
      hosts_query.select() << "tmp.id, t_1.nssetid, t_1.fqdn, t_2.ipaddr";
      //  hosts_query.from() << getTempTableName() << " tmp "
      //                     << "JOIN host t_1 ON (tmp.id = t_1.nssetid) "
      //                     << "LEFT JOIN host_ipaddr_map t_2 ON (t_1.id = t_2.hostid)";
      hosts_query.from() << getTempTableName() << " tmp "
                         << "JOIN host_history t_1 ON (tmp.id = t_1.historyid) "
                         << "LEFT JOIN host_ipaddr_map_history t_2 ON (t_1.historyid = t_2.historyid AND t_1.id = t_2.hostid)";
      hosts_query.order_by() << "t_1.nssetid, t_1.id, t_2.id";
      
      std::auto_ptr<DBase::Result> r_hosts(conn->exec(hosts_query));
      std::auto_ptr<DBase::ResultIterator> hit(r_hosts->getIterator());
      for (hit->first(); !hit->isDone(); hit->next()) {
        DBase::ID nsset_historyid = hit->getNextValue();
        DBase::ID nsset_id = hit->getNextValue();
        std::string host_fqdn = hit->getNextValue();
        std::string host_ip = hit->getNextValue();
        
        NSSetImpl *nsset_ptr = dynamic_cast<NSSetImpl *>(findIDSequence(nsset_id));
        if (nsset_ptr) {
          HostImpl* host_ptr = nsset_ptr->addHost(host_fqdn);
          host_ptr->addAddr(host_ip);
        }
      }
      
      /// load object state
      ObjectListImpl::reload2(conn.get());
    }
    catch (DBase::Exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }
  }
  void clearFilter() {
    ObjectListImpl::clearFilter();
    handle = "";
    admin = "";
    ip = "";
    hostname = "";
  }
  virtual const char *getTempTableName() const {
    return "tmp_nsset_filter_result";
  }
  
  /// sort by column
  virtual void sort(MemberType _member, bool _asc) {
    switch (_member) {
      case MT_HANDLE:
        stable_sort(data_.begin(), data_.end(), CompareHandle(_asc));
        break;
      case MT_CRDATE:
        stable_sort(data_.begin(), data_.end(), CompareCreateDate(_asc));
        break;
      case MT_ERDATE:
        stable_sort(data_.begin(), data_.end(), CompareDeleteDate(_asc));
        break;
      case MT_REGISTRAR_HANDLE:
        stable_sort(data_.begin(), data_.end(), CompareRegistrarHandle(_asc));
        break;
    }
  }
};


class ManagerImpl : public virtual Manager {
  DB *db; ///< connection do db
  Zone::Manager *zm; ///< needed for hostname checking
  bool restrictedHandle; ///< format of handle is more restrictive
  /// check if handle is in valid format
  bool checkHandleFormat(const std::string& handle) const {
    try {
      // format is global variable, because creating online has problems
      // with strange exceptions thrown in constructor
      return boost::regex_match(
          handle,restrictedHandle ? formatRestricted : format
      );
    } catch (...) {
      // TODO: log error
      return false;
    }
  }
  /// check if object is in database
  bool checkHandleRegistration(const std::string& handle, NameIdPair& conflict,
      bool lock) const throw (SQL_ERROR) {
    std::ostringstream sql;
    sql << "SELECT id,name FROM object_registry "
        << "WHERE type=2 AND erDate ISNULL AND " << "UPPER(name)=UPPER('"
        << handle << "')";
    if (lock)
      sql << " FOR UPDATE ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    bool result = db->GetSelectRows() >= 1;
    conflict.id = result ? STR_TO_ID(db->GetFieldValue(0, 0)) : 0;
    conflict.name = result ? db->GetFieldValue(0, 1) : "";
    db->FreeSelect();
    return result;
  }
  /// check if object handle is in protection period (true=protected)
  bool checkProtection(const std::string& name, unsigned type,
      const std::string& monthPeriodSQL) const throw (SQL_ERROR) {
    std::stringstream sql;
    sql << "SELECT COALESCE(" << "MAX(erdate) + INTERVAL '" << monthPeriodSQL
        << "'" << " > CURRENT_DATE, false) " << "FROM object_registry "
        << "WHERE NOT(erdate ISNULL) " << "AND type=" << type << " "
        << "AND UPPER(name)=UPPER('" << name << "')";
    if (!db->ExecSelect(sql.str().c_str())) {
      db->FreeSelect();
      throw SQL_ERROR();
    }
    bool ret = (db->GetFieldValue(0,0)[0] == 't');
    db->FreeSelect();
    return ret;
  }
public:
  ManagerImpl(DB *_db, Zone::Manager *_zm, bool _restrictedHandle) :
    db(_db), zm(_zm), restrictedHandle(_restrictedHandle) {
  }
  virtual List *createList() {
    return new ListImpl(db, zm);
  }
  virtual CheckAvailType checkAvail(const std::string& handle,
      NameIdPair& conflict, bool lock) const throw (SQL_ERROR) {
    conflict.id = 0;
    conflict.name = "";
    if (!checkHandleFormat(handle))
      return CA_INVALID_HANDLE;
    if (checkHandleRegistration(handle, conflict, lock))
      return CA_REGISTRED;
    if (checkProtection(handle, 2, "2 month"))
      return CA_PROTECTED;
    return CA_FREE;
  }
  virtual unsigned checkHostname(const std::string& hostname, 
   															 bool glue, 
   															 bool allowIDN) const {
    try {
      // test according to database limit
      if (hostname.length()> 255) return 1;
      // parse hostname (will throw exception on invalid)
      Zone::DomainName name;
      zm->parseDomainName(hostname,name,allowIDN);
      // if glue is specified, hostname must be under one of managed zones 
      if (glue && !zm->findZoneId(hostname)) return 1;
      // if glue is not specified, hostname must be under any valid zone
      if (!glue && !zm->checkTLD(name)) return 1;
      return 0;
    }
    catch (...) {return 1;}
    // INVALID_DOMAIN_NAME
  }
};


Manager *Manager::create(DB *db, Zone::Manager *zm, bool restrictedHandle) {
  return new ManagerImpl(db, zm, restrictedHandle);
}
}
}
