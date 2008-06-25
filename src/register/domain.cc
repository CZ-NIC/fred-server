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

#include <sstream>
#include <memory>
#include <functional>
#include <algorithm>
#include <boost/date_time/posix_time/time_parsers.hpp>

#include "domain.h"
#include "blacklist.h"
#include "object_impl.h"
#include "sql.h"
#include "old_utils/dbsql.h"
#include "db/dbs.h"
#include "model/model_filters.h"
#include "log/logger.h"

namespace Register {
namespace Domain {
class DomainImpl : public ObjectImpl, public virtual Domain {
  struct AdminInfo {
    TID id;
    std::string handle;
    AdminInfo(TID _id, const std::string& _handle) :
      id(_id), handle(_handle) {
    }
  };
  typedef std::vector<AdminInfo> AdminInfoList;
  std::string fqdn;
  std::string fqdnIDN;
  TID zone;
  TID nsset;
  std::string nssetHandle;
  TID registrant;
  std::string registrantHandle;
  std::string registrantName;
  AdminInfoList adminList;
  AdminInfoList tempList;
  date exDate;
  date valExDate;
  unsigned zoneStatus;
  ptime zoneStatusTime;
  date outZoneDate;
  date cancelDate;
public:
  DomainImpl(TID _id, const std::string& _fqdn, TID _zone, TID _nsset,
             const std::string& _nssetHandle, TID _registrant,
             const std::string& _registrantHandle,
             const std::string& _registrantName, TID _registrar,
             const std::string& _registrarHandle, ptime _crDate, ptime _trDate,
             ptime _upDate, date _erDate, TID _createRegistrar,
             const std::string& _createRegistrarHandle, TID _updateRegistrar,
             const std::string& _updateRegistrarHandle,
             const std::string& _authPw, const std::string& _roid,
             date _exDate, date _valExDate, unsigned _zoneStatus,
             ptime _zoneStatusTime, Zone::Manager *zm) :
    ObjectImpl(_id, _crDate, _trDate, _upDate, _erDate, _registrar,
               _registrarHandle, _createRegistrar, _createRegistrarHandle,
               _updateRegistrar, _updateRegistrarHandle, _authPw, _roid),
        fqdn(_fqdn), fqdnIDN(zm->decodeIDN(fqdn)), zone(_zone), nsset(_nsset), nssetHandle(_nssetHandle),
        registrant(_registrant), registrantHandle(_registrantHandle),
        registrantName(_registrantName), exDate(_exDate),
        valExDate(_valExDate), zoneStatus(_zoneStatus),
        zoneStatusTime(_zoneStatusTime) {
    outZoneDate = exDate + days(30);
    cancelDate = exDate + days(45);
  }
  virtual const std::string& getFQDN() const {
    return fqdn;
  }
  virtual const std::string& getFQDNIDN() const {
		return fqdnIDN;
	}
  virtual TID getZoneId() const {
    return zone;
  }
  virtual const std::string& getNSSetHandle() const {
    return nssetHandle;
  }
  virtual TID getNSSetId() const {
    return nsset;
  }
  virtual void setNSSetId(TID nsset) {
    // check existance and set handle
    this->nsset = nsset;
    modified_ = true;
  }
  virtual const std::string& getRegistrantHandle() const {
    return registrantHandle;
  }
  virtual const std::string& getRegistrantName() const {
    return registrantName;
  }
  virtual TID getRegistrantId() const {
    return registrant;
  }
  virtual void setRegistrantId(TID registrant) {
    this->registrant = registrant;
  }
  virtual date getExpirationDate() const {
    return exDate;
  }
  virtual date getValExDate() const {
    return valExDate;
  }
  virtual unsigned getZoneStatus() const {
    return zoneStatus;
  }
  virtual ptime getZoneStatusTime() const {
    return zoneStatusTime;
  }
  virtual date getOutZoneDate() const {
    return outZoneDate;
  }
  virtual date getCancelDate() const {
    return cancelDate;
  }
  virtual unsigned getAdminCount(unsigned role) const {
    return role == 1 ? adminList.size() : tempList.size();
  }
  virtual TID getAdminIdByIdx(unsigned idx, unsigned role) const
      throw (NOT_FOUND) {
    if (idx >= getAdminCount(role))
      throw NOT_FOUND();
    return role == 1 ? adminList[idx].id : tempList[idx].id;
  }
  virtual const std::string& getAdminHandleByIdx(unsigned idx, unsigned role) const
      throw (NOT_FOUND) {
    if (idx >= getAdminCount(role))
      throw NOT_FOUND();
    return role == 1 ? adminList[idx].handle : tempList[idx].handle;
  }
  virtual void removeAdminId(TID id) {
    // find id in list and delete
  }
  virtual void insertAdminId(TID id) {
    // check existance of id
  }
  /// id lookup function
  bool hasId(TID id) const {
    return id_ == id;
  }
  /// add one admin handle - for domain intialization
  void addAdminHandle(TID id, const std::string& handle, unsigned role=1) {
    if (role == 1)
      adminList.push_back(AdminInfo(id, handle));
    else
      tempList.push_back(AdminInfo(id, handle));
  }
  /// add nsset handle - for domain intialization
  void addNSSetHandle(const std::string& handle) {
    nssetHandle = handle;
  }
  /// setting validation date - for domain initialization
  void setValExDate(date _valExDate) {
    valExDate = _valExDate;
  }
  virtual void insertStatus(TID id, ptime timeFrom, ptime timeTo) {
    ObjectImpl::insertStatus(id, timeFrom, timeTo);
    // trigger setting ouzone status TODO: make define
    if (id == 15) {
      zoneStatus = 0;
      zoneStatusTime = timeFrom;
    }
  }
};


COMPARE_CLASS_IMPL(DomainImpl, FQDN)
COMPARE_CLASS_IMPL(DomainImpl, CreateDate)
COMPARE_CLASS_IMPL(DomainImpl, DeleteDate)
COMPARE_CLASS_IMPL(DomainImpl, RegistrantHandle)
COMPARE_CLASS_IMPL(DomainImpl, RegistrantName)
COMPARE_CLASS_IMPL(DomainImpl, RegistrarHandle)
COMPARE_CLASS_IMPL(DomainImpl, ZoneStatus)
COMPARE_CLASS_IMPL(DomainImpl, ExpirationDate)
COMPARE_CLASS_IMPL(DomainImpl, OutZoneDate)
COMPARE_CLASS_IMPL(DomainImpl, CancelDate)


class ListImpl : virtual public List, public ObjectListImpl {
  TID zoneFilter;
  TID registrantFilter;
  std::string registrantHandleFilter;
  TID nsset;
  std::string nssetHandle;
  TID admin;
  std::string adminHandle;
  TID temp;
  std::string tempHandle;
  TID contactFilter;
  std::string contactHandleFilter;
  std::string fqdn;
  time_period exDate;
  time_period valExDate;
  std::string techAdmin;
  std::string hostIP;
  unsigned zoneStatus;
  Zone::Manager *zm;
public:
  ListImpl(DB *_db, Zone::Manager *_zm) :
    ObjectListImpl(_db), zoneFilter(0), registrantFilter(0), nsset(0),
        admin(0), temp(0), contactFilter(0), exDate(ptime(neg_infin),
                                                    ptime(pos_infin)),
        valExDate(ptime(neg_infin), ptime(pos_infin)), zoneStatus(0),
        zm(_zm) {
  }
  virtual Domain *getDomain(unsigned idx) const {
    return dynamic_cast<DomainImpl *>(get(idx));
  }
  virtual void setZoneFilter(TID zoneId) {
    zoneFilter = zoneId;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setRegistrantFilter(TID registrantId) {
    registrantFilter = registrantId;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  void setRegistrantHandleFilter(const std::string& _registrantHandle) {
    registrantHandleFilter = _registrantHandle;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setNSSetFilter(TID _nssetId) {
    nsset = _nssetId;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setNSSetHandleFilter(const std::string& _nssetHandle) {
    nssetHandle = _nssetHandle;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setAdminFilter(TID _adminId) {
    admin = _adminId;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setAdminHandleFilter(const std::string& _adminHandle) {
    adminHandle = _adminHandle;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setTempFilter(TID _tempId) {
    temp = _tempId;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setTempHandleFilter(const std::string& _tempHandle) {
    tempHandle = _tempHandle;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setContactFilter(TID contactId) {
    contactFilter = contactId;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setContactHandleFilter(const std::string& cHandle) {
    contactHandleFilter = cHandle;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setFQDNFilter(const std::string& _fqdn) {
    fqdn = _fqdn;
    boost::algorithm::to_lower(fqdn);
    setFilterModified();
  }
  virtual void setExpirationDateFilter(time_period period) {
    exDate = period;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setValExDateFilter(time_period period) {
    valExDate = period;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setTechAdminHandleFilter(const std::string& handle) {
    techAdmin = handle;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setHostIPFilter(const std::string& ip) {
    hostIP = ip;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setZoneStatusFilter(unsigned status) {
    zoneStatus = status;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  void makeQuery(bool count, bool limit, std::stringstream& sql) const {
    std::stringstream from, where;
    sql.str("");
    if (!count)
      sql << "INSERT INTO " << getTempTableName() << " ";
    sql << "SELECT " << (count ? "COUNT(" : "") << "DISTINCT d.id"
        << (count ? ") " : " ");
    from << "FROM domain d ";
    where << "WHERE 1=1 ";
    SQL_ID_FILTER(where, "d.id", idFilter);
    SQL_DATE_FILTER(where, "d.exdate", exDate)
    ;
    SQL_ID_FILTER(where, "d.nsset", nsset);
    SQL_ID_FILTER(where, "d.registrant", registrantFilter);
    if (registrarFilter || !registrarHandleFilter.empty()
        || updateRegistrarFilter || !updateRegistrarHandleFilter.empty()
        || TIME_FILTER_SET(updateIntervalFilter) || TIME_FILTER_SET(trDateIntervalFilter)) {
      from << ",object o ";
      where << "AND d.id=o.id ";
      SQL_ID_FILTER(where, "o.clid", registrarFilter);
      SQL_ID_FILTER(where, "o.upid", updateRegistrarFilter);
      SQL_DATE_FILTER(where, "o.upDate", updateIntervalFilter)
      ;
      SQL_DATE_FILTER(where, "o.trDate", trDateIntervalFilter)
      ;
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
        || TIME_FILTER_SET(crDateIntervalFilter) || !fqdn.empty()) {
      from << ",object_registry obr ";
      where << "AND obr.id=d.id AND obr.type=3 ";
      SQL_ID_FILTER(where, "obr.crid", createRegistrarFilter);
      SQL_DATE_FILTER(where, "obr.crdate", crDateIntervalFilter)
      ;
      SQL_HANDLE_WILDCHECK_FILTER(where, "obr.name", fqdn, wcheck, false);
      if (!createRegistrarHandleFilter.empty()) {
        from << ",registrar creg ";
        where << "AND obr.crid=creg.id ";
        SQL_HANDLE_WILDCHECK_FILTER(where, "creg.handle",
            createRegistrarHandleFilter, wcheck, false);
      }
    }
    if (TIME_FILTER_SET(valExDate)) {
      from << ",enumval ev ";
      where << "AND d.id=ev.domainid ";
      SQL_DATE_FILTER(where, "ev.exdate", valExDate)
      ;
    }
    if (!registrantHandleFilter.empty()) {
      from << ",object_registry cor ";
      where << "AND d.registrant=cor.id AND cor.type=1 ";
      SQL_HANDLE_WILDCHECK_FILTER(where, "cor.name", registrantHandleFilter,
          wcheck, true);
    }
    if (admin || !adminHandle.empty()) {
      from << ",domain_contact_map dcm ";
      where << "AND d.id=dcm.domainid AND dcm.role=1 ";
      SQL_ID_FILTER(where, "dcm.contactid", admin);
      if (!adminHandle.empty()) {
        from << ",object_registry dcor ";
        where << "AND dcm.contactid=dcor.id AND dcor.type=1 ";
        SQL_HANDLE_WILDCHECK_FILTER(where, "dcor.name", adminHandle, wcheck,
            true);
      }
    }
    if (temp || !tempHandle.empty()) {
      from << ",domain_contact_map dcmt ";
      where << "AND d.id=dcmt.domainid AND dcmt.role=2 ";
      SQL_ID_FILTER(where, "dcmt.contactid", temp);
      if (!tempHandle.empty()) {
        from << ",object_registry dcort ";
        where << "AND dcmt.contactid=dcort.id AND dcort.type=1 ";
        SQL_HANDLE_WILDCHECK_FILTER(where, "dcort.name", tempHandle, wcheck,
            true);
      }
    }
    if (!nssetHandle.empty()) {
      from << ",object_registry nor ";
      where << "AND d.nsset=nor.id AND nor.type=2 ";
      SQL_HANDLE_WILDCHECK_FILTER(where, "nor.name", nssetHandle, wcheck, true);
    }
    if (!techAdmin.empty()) {
      from << ",nsset_contact_map ncm, object_registry tcor ";
      where << "AND d.nsset=ncm.nssetid AND ncm.contactid=tcor.id "
          << "AND tcor.type=1 ";
      SQL_HANDLE_WILDCHECK_FILTER(where, "tcor.name", techAdmin, wcheck, true);
    }
    if (!hostIP.empty()) {
      from << ",host_ipaddr_map him ";
      where << "AND d.nsset=him.nssetid ";
      SQL_HANDLE_FILTER(where, "host(him.ipaddr)", hostIP);
    }
    if (add)
      where << "AND d.id NOT IN " << "(SELECT id FROM " << getTempTableName()
          << ") ";
    if (!count)
      where << "ORDER BY d.id ASC ";
    if (limit)
      where << "LIMIT " << load_limit_ << " ";
    sql << from.rdbuf();
    sql << where.rdbuf();
  }
  virtual void reload2(DBase::Filters::Union &uf, DBase::Manager* dbm) {
    TRACE("[CALL] Domain::ListImpl::reload2()");
    clear();
    uf.clearQueries();

    // TEMP: should be cached for quicker
    std::map<DBase::ID, std::string> registrars_table;

    DBase::SelectQuery id_query;
    std::auto_ptr<DBase::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      DBase::Filters::Domain *df =
          dynamic_cast<DBase::Filters::Domain*>(fit->get());
      if (!df)
        continue;
      DBase::SelectQuery *tmp = new DBase::SelectQuery();
      tmp->addSelect(new DBase::Column("historyid", df->joinDomainTable(), "DISTINCT"));
      uf.addQuery(tmp);
    }
    id_query.limit(5000);
    uf.serialize(id_query);

    DBase::InsertQuery tmp_table_query = DBase::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER("db").debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    DBase::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, t_1.name, t_2.zone, t_2.nsset, "
        << "t_3.id, t_3.name, t_4.name, t_5.clid, "
        << "t_1.crdate, t_5.trdate, t_5.update, t_1.erdate, t_1.crid, t_5.upid, "
        << "t_5.authinfopw, t_1.roid, t_2.exdate, NULL";
    //    object_info_query.from() << getTempTableName()
    //        << " tmp JOIN domain t_2 ON (tmp.id = t_2.id) "
    //        << "JOIN object t_5 ON (t_2.id = t_5.id) "
    //        << "JOIN contact t_4 ON (t_2.registrant = t_4.id) "
    //        << "JOIN object_registry t_3 ON (t_4.id = t_3.id) "
    //        << "JOIN object_registry t_1 ON (t_5.id = t_1.id)";
    object_info_query.from() << getTempTableName()
        << " tmp JOIN domain_history t_2 ON (tmp.id = t_2.historyid) "
        << "JOIN object_history t_5 ON (t_2.historyid = t_5.historyid) "
        << "JOIN contact_history t_4 ON (t_2.registrant = t_4.id) "
        << "JOIN object_registry t_3 ON (t_4.historyid = t_3.historyid) "
        << "JOIN object_registry t_1 ON (t_5.historyid = t_1.historyid)";

    object_info_query.order_by() << "t_1.id";

    try {
      std::auto_ptr<DBase::Connection> conn(dbm->getConnection());
      
      DBase::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
      std::auto_ptr<DBase::Result> r_create_tmp_table(conn->exec(create_tmp_table));
      conn->exec(tmp_table_query);
      
      // TODO: use this and rewrite conn to conn_ specified in CommonListImpl
      // fillTempTable(tmp_table_query);

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
        DBase::ID did = it->getNextValue();
        std::string fqdn = it->getNextValue();
        DBase::ID zone = it->getNextValue();
        DBase::ID nsset = it->getNextValue();
        std::string nssetHandle = ""; //it->getNextValue();
        DBase::ID registrant = it->getNextValue();
        std::string registrantHandle = it->getNextValue();
        std::string registrantName = it->getNextValue();
        DBase::ID registrar = it->getNextValue();
        std::string registrarHandle = registrars_table[registrar];
        DBase::DateTime crDate = it->getNextValue();
        DBase::DateTime trDate = it->getNextValue();
        DBase::DateTime upDate = it->getNextValue();
        DBase::Date erDate = it->getNextValue();
        DBase::ID createRegistrar = it->getNextValue();
        std::string createRegistrarHandle = registrars_table[createRegistrar];
        DBase::ID updateRegistrar = it->getNextValue();
        std::string updateRegistrarHandle = registrars_table[updateRegistrar];
        std::string authPw = it->getNextValue();
        std::string roid = it->getNextValue();
        DBase::Date exDate = it->getNextValue();
        DBase::Date valExDate = it->getNextValue();
        unsigned zoneStatus = 1;
        ptime zoneStatusTime = ptime();

        data_.push_back (
            new DomainImpl(
                did,
                fqdn,
                zone,
                nsset,
                nssetHandle,
                registrant,
                registrantHandle,
                registrantName,
                registrar,
                registrarHandle,
                crDate,
                trDate,
                upDate,
                erDate,
                createRegistrar,
                createRegistrarHandle,
                updateRegistrar,
                updateRegistrarHandle,
                authPw,
                roid,
                exDate,
                valExDate,
                zoneStatus,
                zoneStatusTime,
                zm
            ));
      }

      if (data_.empty())
      return;

      /// load admin contacts info
      resetIDSequence();
      DBase::SelectQuery admins_query;
      admins_query.select() << "tmp.id, t_2.domainid, t_1.id, t_1.name, t_2.role";
      // admins_query.from() << getTempTableName() << " tmp "
      //                     << "JOIN domain_contact_map t_2 ON (tmp.id = t_2.domainid) "
      //                     << "JOIN object_registry t_1 ON (t_2.contactid = t_1.id)";
      admins_query.from() << getTempTableName() << " tmp "
      << "JOIN domain_contact_map_history t_2 ON (tmp.id = t_2.historyid) "
      << "JOIN object_registry t_1 ON (t_2.contactid = t_1.id)";
      admins_query.order_by() << "t_2.domainid";

      std::auto_ptr<DBase::Result> r_admins(conn->exec(admins_query));
      std::auto_ptr<DBase::ResultIterator> ait(r_admins->getIterator());
      for (ait->first(); !ait->isDone(); ait->next()) {
        DBase::ID domain_historyid = ait->getNextValue();
        DBase::ID domain_id = ait->getNextValue();
        DBase::ID admin_id = ait->getNextValue();
        std::string admin_handle = ait->getNextValue();
        unsigned admin_role = ait->getNextValue();

        DomainImpl *domain_ptr = dynamic_cast<DomainImpl *>(findIDSequence(domain_id));
        if (domain_ptr)
        domain_ptr->addAdminHandle(admin_id, admin_handle, admin_role);
      }

      /// load nssets info
      resetIDSequence();
      DBase::SelectQuery nssets_query;

      nssets_query.select() << "tmp.id, t_2.id, t_1.name";
      // nssets_query.from() << getTempTableName() << " tmp "
      //                     << "JOIN domain t_2 ON (tmp.id = t_2.id) "
      //                     << "JOIN object_registry t_1 ON (t_1.id = t_2.nsset)";
      nssets_query.from() << getTempTableName() << " tmp "
      << "JOIN domain_history t_2 ON (tmp.id = t_2.historyid) "
      << "JOIN object_registry t_1 ON (t_1.id = t_2.nsset)";
      nssets_query.order_by() << "t_2.id";

      std::auto_ptr<DBase::Result> r_nssets(conn->exec(nssets_query));
      std::auto_ptr<DBase::ResultIterator> nit(r_nssets->getIterator());
      for (nit->first(); !nit->isDone(); nit->next()) {
        DBase::ID domain_historyid = nit->getNextValue();
        DBase::ID domain_id = nit->getNextValue();
        std::string nsset_handle = nit->getNextValue();

        DomainImpl *domain_ptr = dynamic_cast<DomainImpl *>(findIDSequence(domain_id));
        if (domain_ptr)
        domain_ptr->addNSSetHandle(nsset_handle);
      }

      /// load validation (for enum domains)
      resetIDSequence();
      DBase::SelectQuery validation_query;
      validation_query.select() << "tmp.id, t_1.domainid, t_1.exdate";
      validation_query.from() << getTempTableName() << " tmp "
      << "JOIN enumval_history t_1 ON (tmp.id = t_1.historyid)";
      validation_query.order_by() << "t_1.domainid";
      std::auto_ptr<DBase::Result> r_validation(conn->exec(validation_query));
      std::auto_ptr<DBase::ResultIterator> vit(r_validation->getIterator());
      for (vit->first(); !vit->isDone(); vit->next()) {
        DBase::ID domain_historyid = vit->getNextValue();
        DBase::ID domain_id = vit->getNextValue();
        DBase::Date validation_date = vit->getNextValue();

        DomainImpl *domain_ptr = dynamic_cast<DomainImpl *>(findIDSequence(domain_id));
        if (domain_ptr)
        domain_ptr->setValExDate(validation_date);
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
  virtual void reload() throw (SQL_ERROR) {
    std::map<TID,std::string> registrars;
    std::ostringstream sql;
    sql << "SELECT id, handle FROM registrar";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      registrars[STR_TO_ID(db->GetFieldValue(i,0))] = db->GetFieldValue(i, 1);
    }
    db->FreeSelect();
    clear();
    bool useTempTable = nonHandleFilterSet || fqdn.empty(); 
    if (useTempTable)
      fillTempTable(true);
    // load domain data
    sql.str("");
    sql << "SELECT "
    // domain, zone, nsset
        << "obr.id,obr.name,d.zone,d.nsset,'',"
    // registrant
        << "cor.id,cor.name,c.name,"
    // registrar
        << "o.clid,"
    // registration dates
        << "obr.crdate,o.trdate,o.update,"
    // creating and updating registrar
        << "obr.crid,o.upid,"
    // repository data
        << "o.authinfopw,obr.roid,"
    // expiration and validation dates (validation in seperate query)
        << "d.exdate,NULL " << "FROM " 
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "contact c, object_registry cor, " << "object_registry obr, "
        << "object o, " << "domain d "
        << "WHERE tmp.id=d.id AND d.id=o.id AND d.registrant=c.id "
        << "AND c.id=cor.id " << "AND obr.id=o.id ";
    if (!useTempTable) {
      sql << "AND tmp.name=LOWER('" << db->Escape2(fqdn) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=3 "; 
    }
    sql << "ORDER BY tmp.id ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      DomainImpl *d;
      d = new DomainImpl(
          STR_TO_ID(db->GetFieldValue(i,0)), // id
          db->GetFieldValue(i,1), // fqdn
          STR_TO_ID(db->GetFieldValue(i,2)), // zone
          STR_TO_ID(db->GetFieldValue(i,3)), // nsset id
          db->GetFieldValue(i,4), // nsset handle
          STR_TO_ID(db->GetFieldValue(i,5)), // registrant id
          db->GetFieldValue(i,6), // registrant handle
          db->GetFieldValue(i,7), // registrant name
          STR_TO_ID(db->GetFieldValue(i,8)), // registrar
          registrars[STR_TO_ID(db->GetFieldValue(i,8))], // registrar handle
          MAKE_TIME(i,9), // crdate
          MAKE_TIME(i,10), // trdate
          MAKE_TIME(i,11), // update
          date(not_a_date_time),
          STR_TO_ID(db->GetFieldValue(i,12)), // crid
          registrars[STR_TO_ID(db->GetFieldValue(i,12))], // crid handle
          STR_TO_ID(db->GetFieldValue(i,13)), // upid
          registrars[STR_TO_ID(db->GetFieldValue(i,13))], // upid handle
          db->GetFieldValue(i,14), // authinfo
          db->GetFieldValue(i,15), // roid
          MAKE_DATE(i,16), // exdate
          MAKE_DATE(i,17), // valexdate
          true, // zone status
          ptime(), // zone status time 
          zm
      );
      data_.push_back(d);
    }
    db->FreeSelect();
    // no need to proceed when nothing was loaded
    if (!getCount())
      return;
    // add admin contacts
    resetIDSequence();
    sql.str("");
    sql << "SELECT " << "tmp.id, obr.id, obr.name, dcm.role " << "FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "domain_contact_map dcm, "
        << "object_registry obr "
        << "WHERE tmp.id=dcm.domainid and dcm.contactid=obr.id ";
    if (!useTempTable) {
      sql << "AND tmp.name=LOWER('" << db->Escape2(fqdn) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=3 "; 
    }
    sql << "ORDER BY tmp.id";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      DomainImpl *dom = dynamic_cast<DomainImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
              i, 0))));
      if (!dom)
        throw SQL_ERROR();
      dom->addAdminHandle(STR_TO_ID(db->GetFieldValue(i, 1)),
      db->GetFieldValue(i, 2), atoi(db->GetFieldValue(i, 3)));
    }
    db->FreeSelect();
    // add nsset handles (instead of LEFT JOIN)
    resetIDSequence();
    sql.str("");
    sql << "SELECT " << "tmp.id, nor.name " << "FROM " 
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "domain d, object_registry nor "
        << "WHERE tmp.id=d.id AND d.nsset=nor.id ";
    if (!useTempTable) {
      sql << "AND tmp.name=LOWER('" << db->Escape2(fqdn) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=3 "; 
    }
    sql << "ORDER BY tmp.id";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      DomainImpl *dom = dynamic_cast<DomainImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
              i, 0))));
      if (!dom)
        throw SQL_ERROR();
      dom->addNSSetHandle(db->GetFieldValue(i, 1) );
    }
    db->FreeSelect();
    // add validation (for enum domains)
    resetIDSequence();
    sql.str("");
    sql << "SELECT " << "tmp.id, ev.exdate " << "FROM " 
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "enumval ev " << "WHERE tmp.id=ev.domainid ";
    if (!useTempTable) {
      sql << "AND tmp.name=LOWER('" << db->Escape2(fqdn) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=3 "; 
    }
    sql << "ORDER BY tmp.id";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      DomainImpl *dom = dynamic_cast<DomainImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
              i, 0))));
      if (!dom)
        throw SQL_ERROR();
      dom->setValExDate(MAKE_DATE(i, 1));
    }
    db->FreeSelect();
    ObjectListImpl::reload(useTempTable ? NULL : fqdn.c_str(),3);
  }
  void clearFilter() {
    ObjectListImpl::clearFilter();
    registrantFilter = 0;
    registrantHandleFilter = "";
    nsset = 0;
    nssetHandle = "";
    admin = 0;
    adminHandle = "";
    fqdn = "";
    zoneFilter = 0;
    techAdmin = "";
    hostIP = "";
    zoneStatus = 0;
  }

  virtual void sort(MemberType member, bool asc) {
    switch (member) {
      case MT_FQDN:
        stable_sort(data_.begin(), data_.end(), CompareFQDN(asc));
        break;
      case MT_CRDATE:
        stable_sort(data_.begin(), data_.end(), CompareCreateDate(asc));
        break;
      case MT_ERDATE:
        stable_sort(data_.begin(), data_.end(), CompareDeleteDate(asc));
        break;
      case MT_REGISTRANT_HANDLE:
        stable_sort(data_.begin(), data_.end(), CompareRegistrantHandle(asc));
        break;
      case MT_REGISTRANT_NAME:
        stable_sort(data_.begin(), data_.end(), CompareRegistrantName(asc));
        break;
      case MT_REGISTRAR_HANDLE:
        stable_sort(data_.begin(), data_.end(), CompareRegistrarHandle(asc));
        break;
      case MT_ZONE_STATUS:
        stable_sort(data_.begin(), data_.end(), CompareZoneStatus(asc));
        break;
      case MT_EXDATE:
        stable_sort(data_.begin(), data_.end(), CompareExpirationDate(asc));
        break;
      case MT_OUTZONEDATE:
        stable_sort(data_.begin(), data_.end(), CompareOutZoneDate(asc));
        break;
      case MT_CANCELDATE:
        stable_sort(data_.begin(), data_.end(), CompareCancelDate(asc));
        break;
    }
  }
  virtual const char *getTempTableName() const {
    return "tmp_domain_filter_result";
  }
};


class ManagerImpl : virtual public Manager {
  DB *db; ///< connection do db
  Zone::Manager *zm; ///< zone management api
  std::auto_ptr<Blacklist> blacklist; ///< black list manager
public:
  ManagerImpl(DB *_db, Zone::Manager *_zm) :
			db(_db), zm(_zm), blacklist(Blacklist::create(_db)) {
	}
  CheckAvailType checkHandle(const std::string& fqdn, 
  													 bool allowIDN) const {
    Zone::DomainName domain; // parsed domain name
    try { zm->parseDomainName(fqdn,domain,allowIDN); }
    catch (Zone::INVALID_DOMAIN_NAME) {return CA_INVALID_HANDLE;}
    const Zone::Zone *z = zm->findZoneId(fqdn);
    // TLD domain allowed only if zone.fqdn='' is in zone list 
    if (!z && domain.size() == 1)
      return CA_INVALID_HANDLE;
    if (!z)
      return CA_BAD_ZONE;
    if (domain.size()> z->getMaxLevel())
      return CA_BAD_LENGHT;
    return CA_AVAILABLE;
  }
  /// interface method implementation  
  CheckAvailType checkAvail(const std::string& _fqdn,
  												  NameIdPair& conflictFqdn,
  												  bool lock,
  												  bool allowIDN) const throw (SQL_ERROR) {
    std::string fqdn = _fqdn;
    boost::algorithm::to_lower(fqdn);
    // clear output
    conflictFqdn.id = 0;
    conflictFqdn.name = "";
    CheckAvailType ret = checkHandle(fqdn, allowIDN);
    if (ret != CA_AVAILABLE) return ret;
    std::stringstream sql;
    // domain can be subdomain or parent domain of registred domain
    // there could be a lot of subdomains therefor LIMIT 1
    if (zm->findZoneId(fqdn)->isEnumZone())
      sql << "SELECT o.name, o.id FROM object_registry o "
          << "WHERE o.type=3 AND o.erdate ISNULL AND " << "(('" << fqdn
          << "' LIKE '%.'|| o.name) OR " << "(o.name LIKE '%.'||'" << fqdn
          << "') OR " << "o.name='" << fqdn << "') " << "LIMIT 1";
    else
      sql << "SELECT o.name, o.id FROM object_registry o "
          << "WHERE o.type=3 AND o.erdate ISNULL AND " << "o.name='" << fqdn
          << "' " << "LIMIT 1";
    if (lock)
      sql << " FOR UPDATE ";
    if (!db->ExecSelect(sql.str().c_str())) {
      db->FreeSelect();
      throw SQL_ERROR();
    }
    if (db->GetSelectRows() == 1) {
      conflictFqdn.name = db->GetFieldValue(0, 0);
      conflictFqdn.id = STR_TO_ID(db->GetFieldValue(0, 1));
      if (fqdn == conflictFqdn.name)
        ret = CA_REGISTRED;
      else if (fqdn.size()> conflictFqdn.name.size())
        ret = CA_PARENT_REGISTRED;
      else
        ret = CA_CHILD_REGISTRED;
    }
    db->FreeSelect();
    if (ret != CA_AVAILABLE)
      return ret;
    if (blacklist->checkDomain(fqdn))
      return CA_BLACKLIST;
    return ret;
  }
  /// interface method implementation
  unsigned long getDomainCount(const std::string& zone) const {
    std::stringstream sql;
    unsigned long ret = 0;
    sql << "SELECT COUNT(*) " << "FROM object_registry o, domain d, zone z "
        << "WHERE d.id=o.id AND d.zone=z.id " << "AND z.fqdn='" << zone << "'";
    if (db->ExecSelect(sql.str().c_str()) && db->GetSelectRows() == 1)
      ret = atol(db->GetFieldValue( 0, 0));
    db->FreeSelect();
    return ret;
  }
  /// interface method implementation
  unsigned long getEnumNumberCount() const {
    std::stringstream sql;
    unsigned long ret = 0;
    sql << "SELECT SUM(power(10,(33-char_length(name))/2)) "
        << "FROM object_registry o, domain d, zone z "
        << "WHERE d.id=o.id AND d.zone=z.id AND z.fqdn='0.2.4.e164.arpa'";
    if (db->ExecSelect(sql.str().c_str()) && db->GetSelectRows() == 1)
      ret = atol(db->GetFieldValue( 0, 0));
    db->FreeSelect();
    return ret;
  }
  virtual List *createList() {
    return new ListImpl(db, zm);
  }
};
Manager *Manager::create(DB *db, Zone::Manager *zm) {
  return new ManagerImpl(db,zm);
}
}
;
}
;
