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
#include <sstream>
#include <memory>
#include <functional>
#include <algorithm>
#include <boost/date_time/posix_time/time_parsers.hpp>

#include "src/deprecated/libfred/registrable_object/domain.hh"
#include "src/deprecated/libfred/blacklist.hh"
#include "src/deprecated/libfred/object_impl.hh"
#include "src/deprecated/libfred/sql.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/model/model_filters.hh"
#include "util/log/logger.hh"
#include "src/deprecated/libfred/registrar.hh"

namespace LibFred {
namespace Domain {
class DomainImpl : public ObjectImpl, public virtual Domain {
  struct AdminInfo
  {
    TID id;
    std::string handle;
    std::string name;
    std::string organization;
    std::string phone;

    AdminInfo(TID _id
            , const std::string& _handle
            , const std::string& _name
            , const std::string& _organization
            , const std::string& _phone)
        : id(_id)
        , handle(_handle)
        , name(_name)
        , organization(_organization)
        , phone( _phone)
    {}

    bool operator==(const AdminInfo& _ai) const
    {
      return (id == _ai.id
              && handle == _ai.handle
              && name == _ai.name
              && organization == _ai.organization
              && phone == _ai.phone
              );
    }
  };//struct AdminInfo

  typedef std::vector<AdminInfo> AdminInfoList;
  std::string fqdn;
  std::string fqdnIDN;
  TID zone;
  TID nsset;
  std::string nssetHandle;
  TID registrant;
  std::string registrantHandle;
  std::string registrantName;
  std::string registrantOrganization;
  std::string registrantPhone;
  AdminInfoList adminList;
  AdminInfoList tempList;
  date exDate;
  date valExDate;
  bool publish;
  unsigned zoneStatus;
  ptime zoneStatusTime;
  ptime outZoneDate;
  ptime cancelDate;

  TID keyset;
  std::string keysetHandle;
public:
  DomainImpl(TID _id, const Database::ID& _history_id, const std::string& _fqdn, TID _zone, TID _nsset,
             const std::string& _nssetHandle, TID _registrant,
             const std::string& _registrantHandle,
             const std::string& _registrantName,
             const std::string& _registrantOrganization,
             const std::string& _registrantPhone,
             TID _registrar,
             const std::string& _registrarHandle, const ptime& _crDate, const ptime& _trDate,
             const ptime& _upDate, const ptime& _erDate, TID _createRegistrar,
             const std::string& _createRegistrarHandle, TID _updateRegistrar,
             const std::string& _updateRegistrarHandle,
             const std::string& _authPw, const std::string& _roid,
             const date& _exDate, const date& _valExDate, unsigned _zoneStatus,
             const ptime& _zoneStatusTime, const ptime& _outZoneDate, const ptime& _cancelDate,
             Zone::Manager *zm, TID _keyset, const std::string &_keysetHandle) :
    ObjectImpl(_id, _history_id, _crDate, _trDate, _upDate, _erDate, _registrar,
               _registrarHandle, _createRegistrar, _createRegistrarHandle,
               _updateRegistrar, _updateRegistrarHandle, _authPw, _roid),
        fqdn(_fqdn), fqdnIDN(zm->punycode_to_utf8(fqdn)), zone(_zone), nsset(_nsset), nssetHandle(_nssetHandle),
        registrant(_registrant), registrantHandle(_registrantHandle),
        registrantName(_registrantName), registrantOrganization(_registrantOrganization),
        registrantPhone(_registrantPhone),exDate(_exDate),
        valExDate(_valExDate), publish(false), zoneStatus(_zoneStatus),
        zoneStatusTime(_zoneStatusTime),
        outZoneDate(_outZoneDate), cancelDate(_cancelDate),
        keyset(_keyset), keysetHandle(_keysetHandle) {
  }
  virtual const std::string& getFQDN() const {
    return fqdn;
  }
  virtual const std::string& getHandle() const {
    return getFQDN();
  }
  virtual const std::string& getFQDNIDN() const {
    return fqdnIDN;
  }
  virtual TID getZoneId() const {
    return zone;
  }
  virtual const std::string& getNssetHandle() const {
    return nssetHandle;
  }
  virtual TID getNssetId() const {
    return nsset;
  }
  virtual void setNssetId(TID nsset) {
    // check existance and set handle
    this->nsset = nsset;
    modified_ = true;
  }
  virtual const std::string &getKeysetHandle() const
  {
      return keysetHandle;
  }
  virtual TID getKeysetId() const
  {
      return keyset;
  }
  virtual void setKeysetId(TID keyset)
  {
      this->keyset = keyset;
      modified_ = true;
  }
  virtual const std::string& getRegistrantHandle() const {
    return registrantHandle;
  }
  virtual const std::string& getRegistrantName() const {
    return registrantName;
  }
  virtual const std::string& getRegistrantOrganization() const {
    return registrantOrganization;
  }
  virtual const std::string& getRegistrantPhone() const {
      return registrantPhone;
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
  virtual bool getPublish() const {
    return publish;
  }
  virtual unsigned getZoneStatus() const {
    return zoneStatus;
  }
  virtual ptime getZoneStatusTime() const {
    return zoneStatusTime;
  }
  virtual ptime getOutZoneDate() const {
    return outZoneDate;
  }
  virtual ptime getCancelDate() const {
    return cancelDate;
  }
  virtual unsigned getAdminCount(unsigned role) const
  {
      TRACE(boost::format
          ("[CALL] DomainImpl::getAdminCount role: %1% ")
          % role );
    return role == 1 ? adminList.size() : tempList.size();
  }//getAdminCount
  virtual TID getAdminIdByIdx(unsigned idx, unsigned role=1) const
  {
    TRACE(boost::format
        ("[CALL] DomainImpl::getAdminIdByIdx idx: %1% role: %2% ")
        % idx % role );
      try
      {
        if (idx >= getAdminCount(role))
          throw NOT_FOUND();
        return role == 1 ? adminList[idx].id : tempList[idx].id;
      }//try
      catch(...)
      {
          LOGGER.error(boost::format
                  ("DomainImpl::getAdminIdByIdx error idx: %1% role: %2% ")
                          % idx % role );
          throw NOT_FOUND();
      }//catch(...)
  }//getAdminIdByIdx
  virtual const std::string& getAdminHandleByIdx(unsigned idx, unsigned role=1) const
  {
      TRACE(boost::format
          ("[CALL] DomainImpl::getAdminHandleByIdx idx: %1% role: %2% ")
          % idx % role );
        try
        {
            if (idx >= getAdminCount(role))
              throw NOT_FOUND();
            return role == 1 ? adminList[idx].handle : tempList[idx].handle;
        }//try
        catch(...)
        {
            LOGGER.error(boost::format
                    ("DomainImpl::getAdminHandleByIdx error idx: %1% role: %2% ")
                            % idx % role );
            throw NOT_FOUND();
        }//catch(...)
  }//getAdminHandleByIdx
  virtual const std::string getAdminNameByIdx(unsigned idx, unsigned role=1) const
  {
      TRACE(boost::format
          ("[CALL] DomainImpl::getAdminNameByIdx idx: %1% role: %2% ")
          % idx % role );
        try
        {
          if (idx >= getAdminCount(role))
              return std::string("");
          return role == 1 ? adminList[idx].name : tempList[idx].name;
        }//try
        catch(...)
        {
            LOGGER.error(boost::format
                    ("DomainImpl::getAdminNameByIdx error idx: %1% role: %2% ")
                            % idx % role );
            throw;
        }//catch(...)
  }//getAdminNameByIdx
  virtual const std::string getAdminOrganizationByIdx(unsigned idx, unsigned role=1) const
  {
      TRACE(boost::format
          ("[CALL] DomainImpl::getAdminOrganizationByIdx idx: %1% role: %2% ")
          % idx % role );
        try
        {
          if (idx >= getAdminCount(role))
              return std::string("");
          return role == 1 ? adminList[idx].organization : tempList[idx].organization;
        }//try
        catch(...)
        {
            LOGGER.error(boost::format
                    ("DomainImpl::getAdminOrganizationByIdx error idx: %1% role: %2% ")
                            % idx % role );
            throw;
        }//catch(...)
  }//getAdminOrganizationByIdx
  virtual const std::string getAdminPhoneByIdx(unsigned idx, unsigned role=1) const
  {
      TRACE(boost::format
          ("[CALL] DomainImpl::getAdminPhoneByIdx idx: %1% role: %2% ")
          % idx % role );
        try
        {
          if (idx >= getAdminCount(role))
              return std::string("");
          return role == 1 ? adminList[idx].phone : tempList[idx].phone;
        }//try
        catch(...)
        {
            LOGGER.error(boost::format
                    ("DomainImpl::getAdminPhoneByIdx error idx: %1% role: %2% ")
                            % idx % role );
            throw;
        }//catch(...)
  }//getAdminPhoneByIdx
  virtual void removeAdminId(TID id [[gnu::unused]]) {
    // find id in list and delete
  }
  virtual void insertAdminId(TID id [[gnu::unused]]) {
    // check existance of id
  }
  /// id lookup function
  bool hasId(TID id) const {
    return id_ == id;
  }
  /// add one admin handle - for domain intialization
  void addAdminHandle(TID id
          , const std::string& handle
          , const std::string& name
          , const std::string& organization
          , const std::string& phone
          , unsigned role=1)
  {
    if (role == 1)
      adminList.push_back(AdminInfo(id, handle, name, organization, phone));
    else
      tempList.push_back(AdminInfo(id, handle, name, organization, phone));
  }
  /// add nsset handle - for domain intialization
  void addNssetHandle(const std::string& handle) {
    nssetHandle = handle;
  }
  ///add keyset handle - for domain initialization
  void addKeysetHandle(const std::string &handle)
  {
      keysetHandle = handle;
  }
  /// setting validation date - for domain initialization
  void setValExDate(date _valExDate) {
    valExDate = _valExDate;
  }
  void setPublish(bool _publish) {
    publish = _publish;
  }
  virtual void insertStatus(const StatusImpl& _state) {
    ObjectImpl::insertStatus(_state);
    // trigger setting ouzone status TODO: make define
    if (_state.getStatusId() == 15) {
      zoneStatus = 0;
      zoneStatusTime = _state.getFrom();
    }
  }
};


COMPARE_CLASS_IMPL(DomainImpl, FQDN)
COMPARE_CLASS_IMPL(DomainImpl, CreateDate)
COMPARE_CLASS_IMPL(DomainImpl, DeleteDate)
COMPARE_CLASS_IMPL(DomainImpl, RegistrantHandle)
COMPARE_CLASS_IMPL(DomainImpl, RegistrantName)
COMPARE_CLASS_IMPL(DomainImpl, RegistrantOrganization)
COMPARE_CLASS_IMPL(DomainImpl, RegistrantPhone)
COMPARE_CLASS_IMPL(DomainImpl, RegistrarHandle)
COMPARE_CLASS_IMPL(DomainImpl, ZoneStatus)
COMPARE_CLASS_IMPL(DomainImpl, ExpirationDate)
COMPARE_CLASS_IMPL(DomainImpl, OutZoneDate)
COMPARE_CLASS_IMPL(DomainImpl, CancelDate)


class CompareAdminNameByIdx
{
    bool asc_;
    unsigned idx_;
public:
  CompareAdminNameByIdx(bool _asc, unsigned _idx) : asc_(_asc), idx_(_idx) { }
  bool operator()(CommonObject *_left, CommonObject *_right) const {
      DomainImpl *l_casted = dynamic_cast<DomainImpl *>(_left);
      DomainImpl *r_casted = dynamic_cast<DomainImpl *>(_right);
    if (l_casted == 0 || r_casted == 0) {
      /* this should never happen */
      throw std::bad_cast();
    }

    return (asc_ ? l_casted->getAdminNameByIdx(idx_) < r_casted->getAdminNameByIdx(idx_)
                 : l_casted->getAdminNameByIdx(idx_) > r_casted->getAdminNameByIdx(idx_));
    }

};//class CompareAdminNameByIdx

class CompareAdminOrganizationByIdx
{
    bool asc_;
    unsigned idx_;
public:
  CompareAdminOrganizationByIdx(bool _asc, unsigned _idx) : asc_(_asc), idx_(_idx) { }
  bool operator()(CommonObject *_left, CommonObject *_right) const {
      DomainImpl *l_casted = dynamic_cast<DomainImpl *>(_left);
      DomainImpl *r_casted = dynamic_cast<DomainImpl *>(_right);
    if (l_casted == 0 || r_casted == 0) {
      /* this should never happen */
      throw std::bad_cast();
    }

    return (asc_ ? l_casted->getAdminOrganizationByIdx(idx_) < r_casted->getAdminOrganizationByIdx(idx_)
                 : l_casted->getAdminOrganizationByIdx(idx_) > r_casted->getAdminOrganizationByIdx(idx_));
    }

};//class CompareAdminOrganizationByIdx

class CompareAdminPhoneByIdx
{
    bool asc_;
    unsigned idx_;
public:
  CompareAdminPhoneByIdx(bool _asc, unsigned _idx) : asc_(_asc), idx_(_idx) { }
  bool operator()(CommonObject *_left, CommonObject *_right) const {
      DomainImpl *l_casted = dynamic_cast<DomainImpl *>(_left);
      DomainImpl *r_casted = dynamic_cast<DomainImpl *>(_right);
    if (l_casted == 0 || r_casted == 0) {
      /* this should never happen */
      throw std::bad_cast();
    }

    return (asc_ ? l_casted->getAdminPhoneByIdx(idx_) < r_casted->getAdminPhoneByIdx(idx_)
                 : l_casted->getAdminPhoneByIdx(idx_) > r_casted->getAdminPhoneByIdx(idx_));
    }

};//class CompareAdminPhoneByIdx


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
  TID keyset;
  std::string keysetHandle;
public:
  ListImpl(DBSharedPtr _db, Zone::Manager *_zm) :
    ObjectListImpl(_db), zoneFilter(0), registrantFilter(0), nsset(0),
        admin(0), temp(0), contactFilter(0), exDate(ptime(neg_infin),
                                                    ptime(pos_infin)),
        valExDate(ptime(neg_infin), ptime(pos_infin)), zoneStatus(0),
        zm(_zm), keyset(0) {
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
  virtual void setNssetFilter(TID _nssetId) {
    nsset = _nssetId;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setNssetHandleFilter(const std::string& _nssetHandle) {
    nssetHandle = _nssetHandle;
    setFilterModified();
    nonHandleFilterSet = true;
  }
  virtual void setKeysetFilter(TID _keysetId)
  {
      keyset = _keysetId;
      setFilterModified();
      nonHandleFilterSet = true;
  }
  virtual void setKeysetHandleFilter(const std::string &_keysetHandle)
  {
      keysetHandle = _keysetHandle;
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
    SQL_ID_FILTER(where, "d.keyset", keyset);
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
    if (!keysetHandle.empty()) {
        from << ", object_registry kor ";
        where << "AND d.keyset=kor.id AND kor.type=4";
        SQL_HANDLE_WILDCHECK_FILTER(where, "kor.name", keysetHandle, wcheck, true);
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
  virtual void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] Domain::ListImpl::reload()");
    clear();
    uf.clearQueries();

    // TEMP: should be cached for quicker
    std::map<Database::ID, std::string> registrars_table;

    bool at_least_one = false;
    Database::SelectQuery id_query;
    std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      Database::Filters::Domain *df =
          dynamic_cast<Database::Filters::DomainHistoryImpl*>(fit->get());
      if (!df)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("historyid", df->joinDomainTable(), "DISTINCT"));
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
    object_info_query.select() << "t_1.id, tmp.id, t_1.name, t_2.zone, t_2.nsset, t_2.keyset, "
        << "t_3.id, t_3.name, t_4.name, t_4.organization, t_4.telephone, t_5.clid, "
        << "t_1.crdate, t_5.trdate, t_5.update, t_1.erdate, t_1.crid, t_5.upid, "
        << "t_5.authinfopw, t_1.roid, t_2.exdate, "
        << "(((t_2.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval)::timestamp + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as outzonedate, "
        << "(((t_2.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval)::timestamp + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as canceldate";
    //    object_info_query.from() << getTempTableName()
    //        << " tmp JOIN domain t_2 ON (tmp.id = t_2.id) "
    //        << "JOIN object t_5 ON (t_2.id = t_5.id) "
    //        << "JOIN contact t_4 ON (t_2.registrant = t_4.id) "
    //        << "JOIN object_registry t_3 ON (t_4.id = t_3.id) "
    //        << "JOIN object_registry t_1 ON (t_5.id = t_1.id)";
    object_info_query.from() << getTempTableName()
        << " tmp JOIN domain_history t_2 ON (tmp.id = t_2.historyid) "
        << "JOIN object_history t_5 ON (t_2.historyid = t_5.historyid) "
        << "JOIN object_registry t_1 ON (t_5.id = t_1.id) "
        << "JOIN contact_history t_4 ON (t_2.registrant = t_4.id) "
        << "JOIN object_registry t_3 ON (t_4.historyid = t_3.historyid) ";
    //    << "JOIN history h ON (tmp.id = h.id) JOIN action a ON (a.id = h.action) ";
    object_info_query.order_by() << "tmp.id";

    try {
        Database::Connection conn = Database::Manager::acquire();

      Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
      conn.exec(create_tmp_table);
      conn.exec(tmp_table_query);

      // TODO: use this and rewrite conn to conn_ specified in CommonListImpl
      // fillTempTable(tmp_table_query);

      // TEMP: should be cached somewhere
      Database::Query registrars_query("SELECT id, handle FROM registrar");
      Database::Result r_registrars = conn.exec(registrars_query);
      for (Database::Result::Iterator it = r_registrars.begin(); it != r_registrars.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID id      = *col;
        std::string handle   = *(++col);
        registrars_table[id] = handle;
      }

      Database::Result r_info = conn.exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID       did                   = *col;
        Database::ID       history_id            = *(++col);
        std::string        fqdn                  = *(++col);
        Database::ID       zone                  = *(++col);
        Database::ID       nsset                 = *(++col);
        std::string        nssetHandle           = ""; //*(++col);
        Database::ID       keyset                = *(++col);
        std::string        keysetHandle          = ""; //*(++col);
        Database::ID       registrant            = *(++col);
        std::string        registrantHandle      = *(++col);
        std::string        registrantName        = *(++col);
        std::string        registrantOrg         = *(++col);
        std::string        registrantPhone       = *(++col);
        Database::ID       registrar             = *(++col);
        std::string        registrarHandle       = registrars_table[registrar];
        Database::DateTime crDate                = *(++col);
        Database::DateTime trDate                = *(++col);
        Database::DateTime upDate                = *(++col);
        Database::DateTime erDate                = *(++col);
        Database::ID       createRegistrar       = *(++col);
        std::string        createRegistrarHandle = registrars_table[createRegistrar];
        Database::ID       updateRegistrar       = *(++col);
        std::string        updateRegistrarHandle = registrars_table[updateRegistrar];
        std::string        authPw                = *(++col);
        std::string        roid                  = *(++col);
        Database::Date     exDate                = *(++col);
        Database::Date     valExDate             = Database::Date();
        unsigned           zoneStatus            = 1;
        ptime              zoneStatusTime        = ptime();
        Database::DateTime outZoneDate           = *(++col);
        Database::DateTime cancelDate            = *(++col);

        /**
         * check erdate date with default parameters selected outzone and cancel dates
         * and compute right ones
         */
        if (!erDate.is_special()) {
          if (erDate < outZoneDate) {
            outZoneDate = erDate;
          }
          /* if erDate is set in database it is also canceldate */
          cancelDate = erDate;
        }

        data_.push_back (
            new DomainImpl(
                did,
                history_id,
                fqdn,
                zone,
                nsset,
                nssetHandle,
                registrant,
                registrantHandle,
                registrantName,
                registrantOrg,
                registrantPhone,
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
                outZoneDate,
                cancelDate,
                zm,
                keyset,
                keysetHandle
            ));
      }

      if (data_.empty())
      return;

      /// load admin contacts info
      resetHistoryIDSequence();
      Database::SelectQuery admins_query;
      admins_query.select() << "tmp.id, t_2.domainid, t_1.id, t_1.name, t_3.name, t_3.organization, t_3.telephone, t_2.role";
      // admins_query.from() << getTempTableName() << " tmp "
      //                     << "JOIN domain_contact_map t_2 ON (tmp.id = t_2.domainid) "
      //                     << "JOIN object_registry t_1 ON (t_2.contactid = t_1.id)";
      admins_query.from() << getTempTableName() << " tmp "
                             "JOIN domain_contact_map_history t_2 ON (tmp.id = t_2.historyid) "
                             "JOIN object_registry t_1 ON (t_2.contactid = t_1.id) "
                             "JOIN contact_history t_3 ON (t_1.historyid = t_3.historyid) "
                              ;
      admins_query.order_by() << "tmp.id";

      Database::Result r_admins = conn.exec(admins_query);
      for (Database::Result::Iterator it = r_admins.begin(); it != r_admins.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID domain_historyid = *col;
                                         (++col);//Database::ID domain_id
        Database::ID admin_id         = *(++col);
        std::string  admin_handle     = *(++col);
        std::string  admin_name       = *(++col);
        std::string  admin_organization    = *(++col);
        std::string  admin_phone      = *(++col);
        unsigned     admin_role       = *(++col);

        DomainImpl *domain_ptr = dynamic_cast<DomainImpl *>(findHistoryIDSequence(domain_historyid));
        if (domain_ptr)
          domain_ptr->addAdminHandle(admin_id, admin_handle
                  , admin_name, admin_organization, admin_phone, admin_role);
      }

      /// load nssets info
      resetHistoryIDSequence();
      Database::SelectQuery nssets_query;

      nssets_query.select() << "tmp.id, t_2.id, t_1.name";
      // nssets_query.from() << getTempTableName() << " tmp "
      //                     << "JOIN domain t_2 ON (tmp.id = t_2.id) "
      //                     << "JOIN object_registry t_1 ON (t_1.id = t_2.nsset)";
      nssets_query.from() << getTempTableName() << " tmp "
      << "JOIN domain_history t_2 ON (tmp.id = t_2.historyid) "
      << "JOIN object_registry t_1 ON (t_1.id = t_2.nsset)";
      nssets_query.order_by() << "tmp.id";

      Database::Result r_nssets = conn.exec(nssets_query);
      for (Database::Result::Iterator it = r_nssets.begin(); it != r_nssets.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID domain_historyid = *col;
                                         (++col);//Database::ID domain_id
        std::string  nsset_handle     = *(++col);

        DomainImpl *domain_ptr = dynamic_cast<DomainImpl *>(findHistoryIDSequence(domain_historyid));
        if (domain_ptr)
          domain_ptr->addNssetHandle(nsset_handle);
      }

      /// load keyset info
      resetHistoryIDSequence();
      Database::SelectQuery keysets_query;

      keysets_query.select() << "tmp.id, t_2.id, t_1.name";
      keysets_query.from()
          << getTempTableName() << " tmp "
          << "JOIN domain_history t_2 ON (tmp.id = t_2.historyid) "
          << "JOIN object_registry t_1 ON (t_1.id = t_2.keyset)";
      keysets_query.order_by() << "tmp.id";

      Database::Result r_keysets = conn.exec(keysets_query);
      for (Database::Result::Iterator it = r_keysets.begin(); it != r_keysets.end(); ++it) {
          Database::Row::Iterator col = (*it).begin();

          Database::ID domain_historyid = *col;
                                           (++col);//Database::ID domain_id
          std::string  keyset_handle    = *(++col);

          DomainImpl *domain_ptr = dynamic_cast<DomainImpl *>(findHistoryIDSequence(domain_historyid));
          if (domain_ptr)
              domain_ptr->addKeysetHandle(keyset_handle);
      }

      /// load validation (for enum domains)
      resetHistoryIDSequence();
      Database::SelectQuery validation_query;
      validation_query.select() << "tmp.id, t_1.domainid, t_1.exdate, t_1.publish";
      validation_query.from() << getTempTableName() << " tmp "
                              << "JOIN enumval_history t_1 ON (tmp.id = t_1.historyid)";
      validation_query.order_by() << "tmp.id";

      Database::Result r_validation = conn.exec(validation_query);
      Database::Result::Iterator it = r_validation.begin();
      for (; it != r_validation.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID   domain_historyid = *col;
                                           (++col);//Database::ID   domain_id
        Database::Date validation_date  = *(++col);
        bool           publish          = *(++col);

        DomainImpl *domain_ptr = dynamic_cast<DomainImpl *>(findHistoryIDSequence(domain_historyid));
        if (domain_ptr) {
          domain_ptr->setValExDate(validation_date);
          domain_ptr->setPublish(publish);
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
    } catch (std::exception& ex) {
        LOGGER.error(boost::format("%1%") % ex.what());
    }

  }//reload(Database::Filters::Union &uf)
  virtual void reload() {
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
        << "cor.id,cor.name,c.name,c.organization,"
    // registrar
        << "o.clid,"
    // registration dates
        << "obr.crdate,o.trdate,o.update,"
    // creating and updating registrar
        << "obr.crid,o.upid,"
    // repository data
        << "o.authinfopw,obr.roid,"
    // expiration and validation dates (validation in seperate query)
        << "d.exdate,NULL,"
    // outzone data and cancel date from enum_parameters compute
        << "(((d.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval)::timestamp + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as outzonedate, "
        << "(((d.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval)::timestamp + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as canceldate, "
    // keyset id and keyset handle
        << "d.keyset, '' "
        << "FROM "
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
          (Database::ID)(0), // history_id
          db->GetFieldValue(i,1), // fqdn
          STR_TO_ID(db->GetFieldValue(i,2)), // zone
          STR_TO_ID(db->GetFieldValue(i,3)), // nsset id
          db->GetFieldValue(i,4), // nsset handle
          STR_TO_ID(db->GetFieldValue(i,5)), // registrant id
          db->GetFieldValue(i,6), // registrant handle
          db->GetFieldValue(i,7), // registrant organization
          db->GetFieldValue(i,8), // registrant name
          std::string(""),
          STR_TO_ID(db->GetFieldValue(i,9)), // registrar
          registrars[STR_TO_ID(db->GetFieldValue(i,9))], // registrar handle
          MAKE_TIME(i,10), // crdate
          MAKE_TIME(i,11), // trdate
          MAKE_TIME(i,12), // update
          ptime(not_a_date_time),
          STR_TO_ID(db->GetFieldValue(i,13)), // crid
          registrars[STR_TO_ID(db->GetFieldValue(i,13))], // crid handle
          STR_TO_ID(db->GetFieldValue(i,14)), // upid
          registrars[STR_TO_ID(db->GetFieldValue(i,14))], // upid handle
          db->GetFieldValue(i,15), // authinfo
          db->GetFieldValue(i,16), // roid
          MAKE_DATE(i,17), // exdate
          MAKE_DATE(i,18), // valexdate
          true, // zone status
          ptime(), // zone status time
          MAKE_TIME(i,19),
          MAKE_TIME(i,20),
          zm,
          STR_TO_ID(db->GetFieldValue(i, 21)), // keyset id
          db->GetFieldValue(i, 22) // keyset handle
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
              db->GetFieldValue(i, 2)
              , std::string("") , std::string("") , std::string("")
              , atoi(db->GetFieldValue(i, 3)));
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
      dom->addNssetHandle(db->GetFieldValue(i, 1) );
    }
    db->FreeSelect();

    // add keyset handles
    resetIDSequence();
    sql.str("");
    sql << "SELECT " << "tmp.id, kor.name " << "FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "domain d, object_registry kor "
        << "WHERE tmp.id=d.id AND d.keyset=kor.id ";
    if (!useTempTable) {
        sql << "AND tmp.name=LOWER('" << db->Escape2(fqdn) << "') "
            << "AND tmp.erdate ISNULL AND tmp.type=3 ";
    }
    sql << "ORDER BY tmp.id";
    if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
    for (unsigned int i = 0; i < (unsigned int)db->GetSelectRows(); i++) {
        DomainImpl *dom = dynamic_cast<DomainImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
                            i, 0))));
        if (!dom)
            throw SQL_ERROR();
        dom->addKeysetHandle(db->GetFieldValue(i, 1));
    }
    db->FreeSelect();

    // add validation (for enum domains)
    resetIDSequence();
    sql.str("");
    sql << "SELECT " << "tmp.id, ev.exdate, ev.publish " << "FROM "
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
      dom->setPublish(*db->GetFieldValue(i, 2) == 't');
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
    keyset = 0;
    keysetHandle = "";
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
      case MT_REGISTRANT_ORG:
        stable_sort(data_.begin(), data_.end(), CompareRegistrantOrganization(asc));
        break;
      case MT_REGISTRANT_PHONE:
        stable_sort(data_.begin(), data_.end(), CompareRegistrantPhone(asc));
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
      case MT_1ADMIN_NAME:
          stable_sort(data_.begin(), data_.end(), CompareAdminNameByIdx(asc,0));
          break;
      case MT_1ADMIN_ORG:
          stable_sort(data_.begin(), data_.end(), CompareAdminOrganizationByIdx(asc,0));
          break;
      case MT_1ADMIN_PHONE:
          stable_sort(data_.begin(), data_.end(), CompareAdminPhoneByIdx(asc,0));
          break;
      case MT_2ADMIN_NAME:
          stable_sort(data_.begin(), data_.end(), CompareAdminNameByIdx(asc,1));
          break;
      case MT_2ADMIN_ORG:
          stable_sort(data_.begin(), data_.end(), CompareAdminOrganizationByIdx(asc,1));
          break;
      case MT_2ADMIN_PHONE:
          stable_sort(data_.begin(), data_.end(), CompareAdminPhoneByIdx(asc,1));
          break;
      case MT_3ADMIN_NAME:
          stable_sort(data_.begin(), data_.end(), CompareAdminNameByIdx(asc,2));
          break;
      case MT_3ADMIN_ORG:
          stable_sort(data_.begin(), data_.end(), CompareAdminOrganizationByIdx(asc,2));
          break;
      case MT_3ADMIN_PHONE:
          stable_sort(data_.begin(), data_.end(), CompareAdminPhoneByIdx(asc,2));
          break;
    }
  }
  virtual const char *getTempTableName() const {
    return "tmp_domain_filter_result";
  }
};


class ManagerImpl : virtual public Manager {
  DBSharedPtr db; ///< connection do db
  Zone::Manager *zm; ///< zone management api
  std::unique_ptr<Blacklist> blacklist; ///< black list manager
public:
  ManagerImpl(DBSharedPtr _db, Zone::Manager *_zm) :
      db(_db), zm(_zm), blacklist(Blacklist::create(_db)) { }
  CheckAvailType checkHandle(const std::string& fqdn,
      bool allowIDN) const {
    Zone::DomainName domain; // parsed domain name
    try { zm->parseDomainName(fqdn,domain,allowIDN); }
    catch (Zone::INVALID_DOMAIN_NAME) {return CA_INVALID_HANDLE;}
    const Zone::Zone *z = zm->findApplicableZone(fqdn);
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
                            bool allowIDN,
                            bool lock) const {
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
    if (zm->findApplicableZone(fqdn)->isEnumZone())
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
  unsigned long getSignedDomainCount(const std::string & _fqdn) const
  {
    std::stringstream sql;
    unsigned long ret = 0;
    sql << "SELECT count(*) FROM domain d "
        "JOIN zone z ON (d.zone = z.id) AND z.fqdn = LOWER('" << db->Escape2(_fqdn) << "') "
      << "WHERE d.keyset IS NOT NULL";
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

  bool isDeletePending(const std::string &_fqdn) const
  {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result result = conn.exec_params(
              "SELECT oreg.name, oreg.id, oreg.crdate FROM object_registry oreg"
              " JOIN object_state os ON os.object_id = oreg.id"
              " JOIN enum_object_states eos ON  eos.id = os.state_id"
              " WHERE oreg.type = 3"
              " AND eos.name = 'deleteCandidate'"
              " AND os.valid_from < current_timestamp"
              " AND (os.valid_to > current_timestamp OR os.valid_to is null)"
              " AND (oreg.erdate is null"
              " OR ((oreg.erdate::timestamp AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague'"
              " >= date_trunc('day', (current_timestamp::timestamp AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague')"
              " AND (oreg.erdate::timestamp AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague'"
              " < date_trunc('day', (current_timestamp::timestamp AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague' +  interval '1 day')))"
              " AND oreg.name = $1::text", Database::query_param_list(_fqdn));
      if (result.size() > 0) {
          LOGGER.debug(boost::format("delete pending check for fqdn %1% selected id=%2%")
                                      % static_cast<std::string>(result[0][0])
                                      % static_cast<std::string>(result[0][1]));
          Database::Result check = conn.exec_params(
                  "SELECT oreg.name, oreg.id FROM object_registry oreg"
                  " WHERE oreg.type = 3"
                  " AND oreg.name = $1::text"
                  " AND oreg.id != $2::bigint"
                  " AND oreg.crdate > $3::timestamp",
                  Database::query_param_list
                    (_fqdn)
                    (static_cast<unsigned long long>(result[0][1]))
                    (static_cast<std::string>(result[0][2])));
          if (check.size() > 0) {
              LOGGER.debug(boost::format("delete pending check found newer domain %1% with id=%2%")
                      % static_cast<std::string>(check[0][0])
                      % static_cast<std::string>(check[0][1]));
              return false;
          }
          else {
              return true;
          }
      }
      else {
          return false;
      }
  }

  virtual List *createList() {
    return new ListImpl(db, zm);
  }
};

Manager *Manager::create(DBSharedPtr db, Zone::Manager *zm) {
  return new ManagerImpl(db,zm);
}

/*
 * return number of domains under regid to date 'date'
 * date is in local time
 */
unsigned long long getRegistrarDomainCount(Database::ID regid, const boost::gregorian::date &date, unsigned int zone_id)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result res_count = conn.exec_params(
            "SELECT count(distinct oreg.id) FROM object_registry oreg"
            " JOIN object_history oh ON oh.id = oreg.id"
            " JOIN history hist ON hist.id = oh.historyid"
            " JOIN domain_history dh ON dh.historyid = hist.id"
            " WHERE dh.zone = $1::integer"
            " AND oh.clid = $2::integer"
            " AND hist.valid_from < ($3::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC' "
            " AND (hist.valid_to >= ($3::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC' "
            " OR hist.valid_to IS NULL)",
            Database::query_param_list(zone_id)(regid)(date));

    if(res_count.size() != 1 || res_count[0][0].isnull()) {
        throw std::runtime_error(
            (boost::format("Couldn't get domain count for registrar ID %1% to date %2%.")
                % regid
                % date).str());
    }

    unsigned long long count = res_count[0][0];

    LOGGER.info( (boost::format("Domain count for registrar ID %1%: %2%")
            % regid
            % count
            ).str()
        );

    return count;

}

std::vector<unsigned long long> getExpiredDomainSummary(const std::string &registrar, const std::vector<date_period> &date_intervals)
{
    DBSharedPtr nodb;
    LibFred::Registrar::Manager::AutoPtr regman(
        LibFred::Registrar::Manager::create(nodb));

    if(!regman->checkHandle(registrar)) {
        throw LibFred::NOT_FOUND();
    }

    Database::Connection conn = Database::Manager::acquire();

    std::vector<unsigned long long> ret;
    ret.reserve(date_intervals.size());

    for (std::vector<date_period>::const_iterator it = date_intervals.begin(); it != date_intervals.end(); it++) {

        // from included, to is behind the period
        Database::Result result = conn.exec_params("SELECT count(*) FROM domain d "
                                        "JOIN object_registry oreg ON oreg.id = d.id AND oreg.erdate IS NULL "
                                        "JOIN object_history oh ON oh.historyid = oreg.historyid "
                                        "JOIN registrar r ON r.id = oh.clid "
                                    "WHERE r.handle = $1::text AND d.exdate >= $2::date AND d.exdate < $3::date ",
                Database::query_param_list (registrar)
                                           (it->begin())
                                           (it->end())
        );

        ret.push_back(result[0][0]);
    }

    return ret;
}


}
}

