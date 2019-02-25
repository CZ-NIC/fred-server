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
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>
#include <vector>

#include "src/deprecated/libfred/registrable_object/nsset.hh"
#include "src/deprecated/libfred/object_impl.hh"
#include "src/deprecated/libfred/sql.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/util/util.hh"
#include "src/deprecated/model/model_filters.hh"
#include "util/log/logger.hh"

#define NSSET_REGEX_RESTRICTED "[nN][sS][sS][iI][dD]:[a-zA-Z0-9_:.-]{1,57}"
#define NSSET_REGEX "[a-zA-Z0-9_:.-]{1,63}"

namespace LibFred {
namespace Nsset {
static boost::regex format(NSSET_REGEX);
static boost::regex formatRestricted(NSSET_REGEX_RESTRICTED);

class HostImpl : public virtual Host {
  typedef std::vector<std::string> AddrListType;
  AddrListType addrList;
  std::string name;
  std::string nameIDN;
public:
  HostImpl(const std::string& _name, Zone::Manager *zm) :
      name(_name), nameIDN(zm->punycode_to_utf8(name)) {
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

  bool operator==(const Host& _other) const {
    return (name == _other.getName() && addrList == _other.getAddrList());
  }
  bool operator!=(const Host& _other) const {
    return !(*this == _other);
  }

  const std::vector<std::string>& getAddrList() const {
    return addrList;
  }
};
class NssetImpl : public ObjectImpl, public virtual Nsset {
  struct AdminInfo {
    TID id;
    std::string handle;

    AdminInfo(TID _id, const std::string& _handle) :
      id(_id), handle(_handle) {
    }

    bool operator==(const AdminInfo& _ai) const {
      return (id == _ai.id && handle == _ai.handle);
    }
  };

  std::string handle;
  typedef std::vector<AdminInfo> ContactListType;
  typedef std::vector<HostImpl> HostListType;

  ContactListType admins;
  HostListType hosts;
  unsigned checkLevel;
  Zone::Manager *zm;
public:
  NssetImpl(TID _id, const Database::ID& _history_id, const std::string& _handle, TID _registrar,
      const std::string& _registrarHandle, ptime _crDate, ptime _trDate,
      ptime _upDate, ptime _erDate, TID _createRegistrar,
      const std::string& _createRegistrarHandle, TID _updateRegistrar,
      const std::string& _updateRegistrarHandle, const std::string& _authPw,
      const std::string& _roid, unsigned _checkLevel, Zone::Manager *_zm) :
    ObjectImpl(_id, _history_id, _crDate, _trDate, _upDate, _erDate, _registrar,
               _registrarHandle, _createRegistrar, _createRegistrarHandle,
               _updateRegistrar, _updateRegistrarHandle,
               _authPw, _roid), handle(_handle), checkLevel(_checkLevel),
               zm(_zm) {
  }
  const std::string& getHandle() const {
    return handle;
  }
  virtual const unsigned &getCheckLevel() const {
    return checkLevel;
  }
  unsigned getAdminCount() const {
    return admins.size();
  }
  std::string getAdminByIdx(unsigned idx) const {
    if (idx>=admins.size())
      return "";
    else
      return admins[idx].handle;
  }
  virtual const std::string& getAdminHandleByIdx(unsigned idx) const
      {
    if (idx >= getAdminCount())
      throw NOT_FOUND();
    return admins[idx].handle;
  }
  virtual TID getAdminIdByIdx(unsigned idx) const
      {
    if (idx >= getAdminCount())
      throw NOT_FOUND();
    return admins[idx].id;
  }
  void addAdminHandle(TID id, const std::string& handle) {
    admins.push_back(AdminInfo(id, handle));
  }
  unsigned getHostCount() const {
    return hosts.size();
  }
  const Host *getHostByIdx(unsigned idx) const
      {
    if (idx>=hosts.size())
      throw NOT_FOUND();
      // return NULL; <-- it is not checked anywhere!!
    else
      return &hosts[idx];
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


COMPARE_CLASS_IMPL(NssetImpl, Handle)
COMPARE_CLASS_IMPL(NssetImpl, CreateDate)
COMPARE_CLASS_IMPL(NssetImpl, DeleteDate)
COMPARE_CLASS_IMPL(NssetImpl, RegistrarHandle)


class ListImpl : public virtual List, public ObjectListImpl {
  std::string handle;
  std::string hostname;
  std::string ip;
  std::string admin;
  Zone::Manager *zm;
public:
  ListImpl(DBSharedPtr _db, Zone::Manager *_zm) :
    ObjectListImpl(_db), zm(_zm) {
  }
  Nsset *getNsset(unsigned idx) const {
    return dynamic_cast<Nsset *>(get(idx));
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
  void reload() {
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
      data_.push_back(new NssetImpl(
          STR_TO_ID(db->GetFieldValue(i,0)), // nsset id
          (Database::ID)(0), // history_id
          db->GetFieldValue(i,1), // nsset handle
          STR_TO_ID(db->GetFieldValue(i,2)), // registrar id
          registrars[STR_TO_ID(db->GetFieldValue(i,2))], // reg. handle
          MAKE_TIME(i,3), // registrar crdate
          MAKE_TIME(i,4), // registrar trdate
          MAKE_TIME(i,5), // registrar update
          ptime(not_a_date_time),
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
      NssetImpl *ns =
          dynamic_cast<NssetImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
              i, 0)) ));
      if (!ns)
        throw SQL_ERROR();
      ns->addAdminHandle(0, db->GetFieldValue(i, 1));
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
      NssetImpl *ns =
          dynamic_cast<NssetImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
              i, 0)) ));
      if (!ns)
        throw SQL_ERROR();
      HostImpl* h = ns->addHost(db->GetFieldValue(i, 1));
      h->addAddr(db->GetFieldValue(i, 2));
    }
    db->FreeSelect();
    ObjectListImpl::reload(useTempTable ? NULL : handle.c_str(),2);
  }
  virtual void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] Nsset::ListImpl::reload()");
    clear();
    uf.clearQueries();

    // TEMP: should be cached for quicker
    std::map<Database::ID, std::string> registrars_table;

    bool at_least_one = false;
    Database::SelectQuery id_query;
    std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      Database::Filters::NSSet *df = dynamic_cast<Database::Filters::NSSetHistoryImpl* >(fit->get());
      if (!df)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("historyid", df->joinNSSetTable(), "DISTINCT"));
      uf.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER.error("wrong filter passed for reload!");
      return;
    }

    id_query.limit(load_limit_);
    uf.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(),
        id_query);
    LOGGER.debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, tmp.id, t_1.name, t_2.clid, t_1.crdate, "
        << "t_2.trdate, t_2.update, t_1.erdate, t_1.crid, t_2.upid, "
        << "t_2.authinfopw, t_1.roid, t_3.checklevel";
//    object_info_query.from() << getTempTableName()
//        << " tmp JOIN nsset t_3 ON (tmp.id = t_3.id) "
//        << "JOIN object t_2 ON (t_3.id = t_2.id) "
//        << "JOIN object_registry t_1 ON (t_1.id = t_2.id)";
    object_info_query.from() << getTempTableName()
        << " tmp JOIN nsset_history t_3 ON (tmp.id = t_3.historyid) "
        << "JOIN object_history t_2 ON (t_3.historyid = t_2.historyid) "
        << "JOIN object_registry t_1 ON (t_1.id = t_2.id)";
    object_info_query.order_by() << "tmp.id";

    try {
      Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
      Database::Connection conn = Database::Manager::acquire();
      conn.exec(create_tmp_table);
      conn.exec(tmp_table_query);

      // TEMP: should be cached somewhere
      Database::Query registrars_query("SELECT id, handle FROM registrar");
      Database::Result r_registrars = conn.exec(registrars_query);
      Database::Result::Iterator it = r_registrars.begin();
      for (; it != r_registrars.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID id      = *col;
        std::string  handle  = *(++col);
        registrars_table[id] = handle;
      }

      Database::Result r_info = conn.exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID       nid              = *col;
        Database::ID       history_id       = *(++col);
        std::string        handle           = *(++col);
        Database::ID       registrar_id     = *(++col);
        std::string        registrar_handle = registrars_table[registrar_id];
        Database::DateTime cr_date          = *(++col);
        Database::DateTime tr_date          = *(++col);
        Database::DateTime up_date          = *(++col);
        Database::DateTime er_date          = *(++col);
        Database::ID       crid             = *(++col);
        std::string        crid_handle      = registrars_table[crid];
        Database::ID       upid             = *(++col);
        std::string        upid_handle      = registrars_table[upid];
        std::string        authinfo         = *(++col);
        std::string        roid             = *(++col);
        unsigned           check_level      = *(++col);

        data_.push_back(
            new NssetImpl(
                nid,
                history_id,
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

      resetHistoryIDSequence();
      Database::SelectQuery contacts_query;
      contacts_query.select() << "tmp.id, t_1.nssetid, t_2.id, t_2.name";
      //  contacts_query.from() << getTempTableName() << " tmp "
      //                        << "JOIN nsset_contact_map t_1 ON (tmp.id = t_1.nssetid) "
      //                        << "JOIN object_registry t_2 ON (t_1.contactid = _t_2.id)";
      contacts_query.from() << getTempTableName() << " tmp "
                            << "JOIN nsset_contact_map_history t_1 ON (tmp.id = t_1.historyid) "
                            << "JOIN object_registry t_2 ON (t_1.contactid = t_2.id)";
      contacts_query.order_by() << "t_1.nssetid, tmp.id ";

      Database::Result r_contacts = conn.exec(contacts_query);
      for (Database::Result::Iterator it = r_contacts.begin(); it != r_contacts.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID nsset_historyid = *col;
                                        (++col);//Database::ID nsset_id
        Database::ID contact_id      = *(++col);
        std::string  contact_handle  = *(++col);

        NssetImpl *nsset_ptr = dynamic_cast<NssetImpl *>(findHistoryIDSequence(nsset_historyid));
        if (nsset_ptr)
          nsset_ptr->addAdminHandle(contact_id, contact_handle);
      }

      resetHistoryIDSequence();
      Database::SelectQuery hosts_query;
      hosts_query.select() << "tmp.id, t_1.nssetid, t_1.fqdn, t_2.ipaddr";
      //  hosts_query.from() << getTempTableName() << " tmp "
      //                     << "JOIN host t_1 ON (tmp.id = t_1.nssetid) "
      //                     << "LEFT JOIN host_ipaddr_map t_2 ON (t_1.id = t_2.hostid)";
      hosts_query.from() << getTempTableName() << " tmp "
                         << "JOIN host_history t_1 ON (tmp.id = t_1.historyid) "
                         << "LEFT JOIN host_ipaddr_map_history t_2 ON (t_1.historyid = t_2.historyid AND t_1.id = t_2.hostid)";
      hosts_query.order_by() << "t_1.nssetid, tmp.id, t_1.fqdn, t_2.ipaddr";

      Database::Result r_hosts = conn.exec(hosts_query);
      for (Database::Result::Iterator it = r_hosts.begin(); it != r_hosts.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID nsset_historyid = *col;
                                        (++col);//Database::ID nsset_id
        std::string  host_fqdn       = *(++col);
        std::string  host_ip         = *(++col);

        NssetImpl *nsset_ptr = dynamic_cast<NssetImpl *>(findHistoryIDSequence(nsset_historyid));
        if (nsset_ptr) {
          HostImpl* host_ptr = nsset_ptr->addHost(host_fqdn);
          host_ptr->addAddr(host_ip);
        }
      }

      bool history = false;
      if (uf.settings()) {
        history = uf.settings()->get("filter.history") == "on";
      }

      /// load object state
      ObjectListImpl::reload(history);
      /* checks if row number result load limit is active and set flag */
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
        std::string message = ex.what();
        if (message.find(Database::Connection::getTimeoutString())
                != std::string::npos) {
            LOGGER.info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER.error(boost::format("%1%") % ex.what());
            clear();
        }
    }
    catch (std::exception& ex) {
      LOGGER.error(boost::format("%1%") % ex.what());
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
  DBSharedPtr db; ///< connection do db
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
      bool lock) const {
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
      const std::string& monthPeriodSQL) const {
    std::stringstream sql;
    sql << "SELECT COALESCE(" << "MAX(erdate) + (" << monthPeriodSQL
        << ")::interval" << " > CURRENT_TIMESTAMP, false) " << "FROM object_registry "
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
  ManagerImpl(DBSharedPtr _db, Zone::Manager *_zm, bool _restrictedHandle) :
    db(_db), zm(_zm), restrictedHandle(_restrictedHandle) {
  }
  virtual List *createList() {
    return new ListImpl(db, zm);
  }
  virtual CheckAvailType checkAvail(const std::string& handle,
      NameIdPair& conflict, bool lock) const {
    conflict.id = 0;
    conflict.name = "";
    if (!checkHandleFormat(handle))
      return CA_INVALID_HANDLE;
    if (checkHandleRegistration(handle, conflict, lock))
      return CA_REGISTRED;
    if (checkProtection(handle, 2, "(SELECT val FROM enum_parameters WHERE id = 12) || ' month'"))
      return CA_PROTECTED;
    return CA_FREE;
  }
  virtual unsigned checkHostname(const std::string& hostname,
    bool glue, bool allowIDN) const
  {
    try {
      // 255 - 2 = 253 (difference between wire and textual representation of fqdn)
      // fix for #9673, we don't care about possible trailing dot here because it
      // is forbidden in actual implementation
      if (hostname.length() > 253) return 1;
      // parse hostname (will throw exception on invalid)
      Zone::DomainName name;
      zm->parseDomainName(hostname,name,allowIDN);
      // if glue is specified, hostname must be under one of managed zones
      if (glue && !zm->findApplicableZone(hostname)) return 2;
      // if glue is not specified, hostname must be under any valid zone
      if (!glue && !zm->checkTLD(name)) return 1;
      return 0;
    }
    catch (...) {return 1;}
    // INVALID_DOMAIN_NAME
  }
};


Manager *Manager::create(DBSharedPtr db, Zone::Manager *zm, bool restrictedHandle) {
  return new ManagerImpl(db, zm, restrictedHandle);
}
}
}
