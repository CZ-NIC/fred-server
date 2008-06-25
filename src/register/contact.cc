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

#include "contact.h"
#include "object_impl.h"
#include "sql.h"
#include "old_utils/dbsql.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>
#include <vector>
#include "old_utils/log.h"
#include "db/dbs.h"
#include "model/model_filters.h"
#include "log/logger.h"

/* FIXME: Use types as defined in IDL files and not this horrible macro */
#define SET_SSNTYPE(t) (t == 0 ? "EMPTY" : \
                        t == 1 ? "RC" : \
                        t == 2 ? "OP" : \
                        t == 3 ? "PASSPORT" : \
                        t == 4 ? "ICO" : \
                        t == 5 ? "MPSV" : \
                        t == 6 ? "BIRTHDAY" : \
                        "UNKNOWN")

#define CONTACT_REGEX_RESTRICTED "[cC][iI][dD]:[a-zA-Z0-9_:.-]{1,59}"
#define CONTACT_REGEX "[a-zA-Z0-9_:.-]{1,63}"

namespace Register {
namespace Contact {
static boost::regex format(CONTACT_REGEX);
static boost::regex formatRestricted(CONTACT_REGEX_RESTRICTED);

class ContactImpl : public ObjectImpl, public virtual Contact {
  std::string handle;
  std::string name;
  std::string organization;
  std::string street1;
  std::string street2;
  std::string street3;
  std::string province;
  std::string postalCode;
  std::string city;
  std::string country;
  std::string telephone;
  std::string fax;
  std::string email;
  std::string notifyEmail;
  std::string ssn;
  std::string ssnType;
  unsigned ssnTypeId;
  std::string vat;
  bool discloseName;
  bool discloseOrganization;
  bool discloseAddr;
  bool discloseEmail;
  bool discloseTelephone;
  bool discloseFax;
  bool discloseVat;
  bool discloseIdent;
  bool discloseNotifyEmail;
public:
  ContactImpl(TID _id, const std::string& _handle, const std::string& _name,
      TID _registrar, const std::string& _registrarHandle, ptime _crDate,
      ptime _trDate, ptime _upDate, date _erDate, TID _createRegistrar,
      const std::string& _createRegistrarHandle, TID _updateRegistrar,
      const std::string& _updateRegistrarHandle, const std::string& _authPw,
      const std::string& _roid, const std::string& _organization,
      const std::string& _street1, const std::string& _street2,
      const std::string& _street3, const std::string& _province,
      const std::string& _postalCode, const std::string& _city,
      const std::string& _country, const std::string& _telephone,
      const std::string& _fax, const std::string& _email,
      const std::string& _notifyEmail, const std::string& _ssn,
      unsigned _ssnType, const std::string& _vat, bool _discloseName,
      bool _discloseOrganization, bool _discloseAddr, bool _discloseEmail,
      bool _discloseTelephone, bool _discloseFax, bool _discloseVat,
      bool _discloseIdent, bool _discloseNotifyEmail) :
    ObjectImpl(_id, _crDate, _trDate, _upDate, _erDate, _registrar, 
               _registrarHandle, _createRegistrar, _createRegistrarHandle, 
               _updateRegistrar, _updateRegistrarHandle, _authPw, _roid), 
               handle(_handle), name(_name), organization(_organization), 
               street1(_street1), street2(_street2), street3(_street3), 
               province(_province), postalCode(_postalCode), city(_city), 
               country(_country), telephone(_telephone), fax(_fax), 
               email(_email), notifyEmail(_notifyEmail), ssn(_ssn), 
               ssnType(SET_SSNTYPE(_ssnType)), ssnTypeId(_ssnType), vat(_vat), 
               discloseName(_discloseName), 
               discloseOrganization(_discloseOrganization), 
               discloseAddr(_discloseAddr), discloseEmail(_discloseEmail),
               discloseTelephone(_discloseTelephone), discloseFax(_discloseFax),
               discloseVat(_discloseVat), discloseIdent(_discloseIdent),
               discloseNotifyEmail(_discloseNotifyEmail) {
  }

  const std::string& getHandle() const {
    return handle;
  }
  const std::string& getName() const {
    return name;
  }
  const std::string& getOrganization() const {
    return organization;
  }
  const std::string& getStreet1() const {
    return street1;
  }
  const std::string& getStreet2() const {
    return street2;
  }
  const std::string& getStreet3() const {
    return street3;
  }
  const std::string& getProvince() const {
    return province;
  }
  const std::string& getPostalCode() const {
    return postalCode;
  }
  const std::string& getCity() const {
    return city;
  }
  const std::string& getCountry() const {
    return country;
  }
  const std::string& getTelephone() const {
    return telephone;
  }
  const std::string& getFax() const {
    return fax;
  }
  const std::string& getEmail() const {
    return email;
  }
  const std::string& getNotifyEmail() const {
    return notifyEmail;
  }
  const std::string& getSSN() const {
    return ssn;
  }
  virtual const std::string& getSSNType() const {
    return ssnType;
  }
  virtual unsigned getSSNTypeId() const {
    return ssnTypeId;
  }
  virtual const std::string& getVAT() const {
    return vat;
  }
  virtual bool getDiscloseName() const {
    return discloseName;
  }
  virtual bool getDiscloseOrganization() const {
    return discloseOrganization;
  }
  virtual bool getDiscloseAddr() const {
    return discloseAddr;
  }
  virtual bool getDiscloseEmail() const {
    return discloseEmail;
  }
  virtual bool getDiscloseTelephone() const {
    return discloseTelephone;
  }
  virtual bool getDiscloseFax() const {
    return discloseFax;
  }
  virtual bool getDiscloseVat() const {
    return discloseVat;
  }
  virtual bool getDiscloseIdent() const {
    return discloseIdent;
  }
  virtual bool getDiscloseNotifyEmail() const {
    return discloseNotifyEmail;
  }
};


COMPARE_CLASS_IMPL(ContactImpl, Handle)
COMPARE_CLASS_IMPL(ContactImpl, Name)
COMPARE_CLASS_IMPL(ContactImpl, Organization)
COMPARE_CLASS_IMPL(ContactImpl, CreateDate)
COMPARE_CLASS_IMPL(ContactImpl, DeleteDate)
COMPARE_CLASS_IMPL(ContactImpl, RegistrarHandle)


class ListImpl : public virtual List, public ObjectListImpl {
  std::string handle;
  std::string name;
  std::string ident;
  std::string email;
  std::string org;
  std::string vat;
public:
  ListImpl(DB *_db) :
    ObjectListImpl(_db) {
  }
  Contact *getContact(unsigned idx) const {
    return dynamic_cast<ContactImpl *>(get(idx));
  }
  void setHandleFilter(const std::string& _handle) {
    handle = _handle;
  }
  void setNameFilter(const std::string& _name) {
    name = _name;
    nonHandleFilterSet = true;
  }
  void setIdentFilter(const std::string& _ident) {
    ident = _ident;
    nonHandleFilterSet = true;
  }
  void setEmailFilter(const std::string& _email) {
    email = _email;
    nonHandleFilterSet = true;
  }
  void setOrganizationFilter(const std::string& _org) {
    org = _org;
    nonHandleFilterSet = true;
  }
  void setVATFilter(const std::string& _vat) {
    vat = _vat;
    nonHandleFilterSet = true;
  }
  void makeQuery(bool count, bool limit, std::stringstream& sql) const {
    std::stringstream from, where;
    sql.str("");
    if (!count)
      sql << "INSERT INTO " << getTempTableName() << " ";
    sql << "SELECT " << (count ? "COUNT(" : "") << "DISTINCT c.id"
        << (count ? ") " : " ");
    from << "FROM contact c ";
    where << "WHERE 1=1 ";
    SQL_ID_FILTER(where, "c.id", idFilter);
    SQL_HANDLE_FILTER(where, "c.name", name);
    SQL_HANDLE_FILTER(where, "c.ssn", ident);
    SQL_HANDLE_FILTER(where, "c.email", email);
    SQL_HANDLE_FILTER(where, "c.organization", org);
    SQL_HANDLE_FILTER(where, "c.vat", vat);
    if (registrarFilter || !registrarHandleFilter.empty()
        || updateRegistrarFilter || !updateRegistrarHandleFilter.empty()
        || TIME_FILTER_SET(updateIntervalFilter)
        || TIME_FILTER_SET(trDateIntervalFilter) ) {
      from << ",object o ";
      where << "AND c.id=o.id ";
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
      where << "AND obr.id=c.id AND obr.type=1 ";
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
    if (!count)
      where << "ORDER BY c.id ASC ";
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
    sql << "SELECT obr.id,obr.name,c.name," << "o.clid,"
        << "obr.crdate,o.trdate,o.update,"
        << "obr.crid,o.upid,o.authinfopw,obr.roid,"
        << "c.organization,c.street1,c.street2,c.street3,"
        << "c.stateorprovince,"
        << "c.postalcode,c.city,c.country,c.telephone,c.fax,c.email,"
        << "c.notifyEmail,c.ssn,c.ssntype,c.vat,"
        << "c.disclosename,c.discloseorganization,c.discloseaddress,"
        << "c.discloseemail,c.disclosetelephone,c.disclosefax, "
        << "c.disclosevat,c.discloseident,c.disclosenotifyemail " << "FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "contact c, object_registry obr, object o "
        << "WHERE tmp.id=c.id AND c.id=o.id AND o.id=obr.id ";
    if (!useTempTable) {
      sql << "AND tmp.name=UPPER('" << db->Escape2(handle) << "') "
          << "AND tmp.erdate ISNULL AND tmp.type=1 "; 
    }    
    sql << "ORDER BY tmp.id ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      data_.push_back(new ContactImpl(
          STR_TO_ID(db->GetFieldValue(i,0)), // id
          db->GetFieldValue(i,1), // handle
          db->GetFieldValue(i,2), // name
          STR_TO_ID(db->GetFieldValue(i,3)), // registrar id
          registrars[STR_TO_ID(db->GetFieldValue(i,3))], // reg. handle
          MAKE_TIME(i,4), // crdate
          MAKE_TIME(i,5), // trdate
          MAKE_TIME(i,6), // update
          date(not_a_date_time),
          STR_TO_ID(db->GetFieldValue(i,7)), // crid
          registrars[STR_TO_ID(db->GetFieldValue(i,7))], // crid handle
          STR_TO_ID(db->GetFieldValue(i,8)), // upid
          registrars[STR_TO_ID(db->GetFieldValue(i,8))], // upid handle
          db->GetFieldValue(i,9), // authinfo
          db->GetFieldValue(i,10), // roid
          db->GetFieldValue(i,11), //organization
          db->GetFieldValue(i,12), //street1
          db->GetFieldValue(i,13), //street2
          db->GetFieldValue(i,14), //street3
          db->GetFieldValue(i,15), //province
          db->GetFieldValue(i,16), //postalcode
          db->GetFieldValue(i,17), //city
          db->GetFieldValue(i,18), //country
          db->GetFieldValue(i,19), //telephone
          db->GetFieldValue(i,20), //fax
          db->GetFieldValue(i,21), //email
          db->GetFieldValue(i,22), //notifyEmail
          db->GetFieldValue(i,23), //ssn
          atoi(db->GetFieldValue(i,24)), //ssntype
          db->GetFieldValue(i,25), //vat
          (*db->GetFieldValue(i,26) == 't'), //discloseName
          (*db->GetFieldValue(i,27) == 't'), //discloseOrganization
          (*db->GetFieldValue(i,28) == 't'), //discloseAddr
          (*db->GetFieldValue(i,29) == 't'), //discloseEmail
          (*db->GetFieldValue(i,30) == 't'), //discloseTelephone
          (*db->GetFieldValue(i,31) == 't'), //discloseFax
          (*db->GetFieldValue(i,32) == 't'), //discloseVat
          (*db->GetFieldValue(i,33) == 't'), //discloseIdent
          (*db->GetFieldValue(i,34) == 't') //discloseNotifyEmail
      ));
    }
    db->FreeSelect();
    ObjectListImpl::reload(useTempTable ? NULL : handle.c_str(),1);
  }
  void reload2(DBase::Filters::Union &uf, DBase::Manager* dbm) {
    TRACE("[CALL] ContactListImpl::reload2()");
    clear();
    DBase::SelectQuery id_query;

    // TEMP: should be cached
    std::map<DBase::ID, std::string> registrars_table;

    std::auto_ptr<DBase::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      DBase::Filters::Contact *cf =
          dynamic_cast<DBase::Filters::Contact* >(fit->get());
      if (!cf)
        continue;
      DBase::SelectQuery *tmp = new DBase::SelectQuery();
      tmp->addSelect(new DBase::Column("historyid", cf->joinContactTable(), "DISTINCT"));
      uf.addQuery(tmp);
    }
    id_query.limit(5000);
    uf.serialize(id_query);

    DBase::InsertQuery tmp_table_query = DBase::InsertQuery(getTempTableName(),
        id_query);
    LOGGER("db").debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    DBase::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, t_1.name, t_2.name, t_3.clid, "
        << "t_1.crdate, t_3.trdate, t_3.update, t_1.erdate, t_1.crid, t_3.upid, "
        << "t_3.authinfopw, t_1.roid, t_2.organization, t_2.street1, "
        << "t_2.street2, t_2.street3, t_2.stateorprovince, t_2.postalcode, "
        << "t_2.city, t_2.country, t_2.telephone, t_2.fax, t_2.email, "
        << "t_2.notifyEmail, t_2.ssn, t_2.ssntype,t_2.vat, "
        << "t_2.disclosename, t_2.discloseorganization, "
        << "t_2.discloseaddress, t_2.discloseemail, "
        << "t_2.disclosetelephone, t_2.disclosefax, "
        << "t_2.disclosevat, t_2.discloseident, t_2.disclosenotifyemail";
//    object_info_query.from() << getTempTableName()
//        << " tmp JOIN contact t_2 ON (tmp.id = t_2.id) "
//          " JOIN object t_3 ON (t_2.id = t_3.id) JOIN object_registry t_1 ON (t_3.id = t_1.id)";
    object_info_query.from() << getTempTableName() << " tmp "
        << "JOIN contact_history t_2 ON (tmp.id = t_2.historyid) "
        << "JOIN object_history t_3 ON (t_2.historyid = t_3.historyid) "
        << "JOIN object_registry t_1 ON (t_3.historyid = t_1.historyid)";
    
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
        DBase::ID cid = it->getNextValue();
        std::string handle = it->getNextValue();
        std::string name = it->getNextValue();
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
        std::string organization = it->getNextValue();
        std::string street1 = it->getNextValue();
        std::string street2 = it->getNextValue();
        std::string street3 = it->getNextValue();
        std::string province = it->getNextValue();
        std::string postal_code = it->getNextValue();
        std::string city = it->getNextValue();
        std::string country = it->getNextValue();
        std::string telephone = it->getNextValue();
        std::string fax = it->getNextValue();
        std::string email = it->getNextValue();
        std::string notify_email = it->getNextValue();
        std::string ssn = it->getNextValue();
        unsigned ssn_type = it->getNextValue();
        std::string vat = it->getNextValue();
        bool disclose_name = ((std::string)it->getNextValue() == "t");
        bool disclose_organization = ((std::string)it->getNextValue() == "t");
        bool disclose_address = ((std::string)it->getNextValue() == "t");
        bool disclose_email = ((std::string)it->getNextValue() == "t");
        bool disclose_telephone = ((std::string)it->getNextValue() == "t");
        bool disclose_fax = ((std::string)it->getNextValue() == "t");
        bool disclose_vat = ((std::string)it->getNextValue() == "t");
        bool disclose_ident = ((std::string)it->getNextValue() == "t");
        bool disclose_notify_email = ((std::string)it->getNextValue() == "t");

        data_.push_back(
            new ContactImpl(
                cid,
                handle,
                name,
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
                organization,
                street1,
                street2,
                street3,
                province,
                postal_code,
                city,
                country,
                telephone,
                fax,
                email,
                notify_email,
                ssn,
                ssn_type,
                vat,
                disclose_name,
                disclose_organization,
                disclose_address,
                disclose_email,
                disclose_telephone,
                disclose_fax,
                disclose_vat,
                disclose_ident,
                disclose_notify_email
            ));
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
    name = "";
    ident = "";
    email = "";
    org = "";
    vat = "";
  }
  
  virtual const char *getTempTableName() const {
    return "tmp_contact_filter_result";
  }

  virtual void sort(MemberType _member, bool _asc) {
    switch (_member) {
      case MT_HANDLE:
        stable_sort(data_.begin(), data_.end(), CompareHandle(_asc));
        break;
      case MT_NAME:
        stable_sort(data_.begin(), data_.end(), CompareName(_asc));
        break;
      case MT_ORGANIZATION:
        stable_sort(data_.begin(), data_.end(), CompareOrganization(_asc));
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
        << "WHERE type=1 AND erDate ISNULL AND " << "UPPER(name)=UPPER('"
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
  ManagerImpl(DB *_db, bool _restrictedHandle) :
    db(_db), restrictedHandle(_restrictedHandle) {
  }
  virtual List *createList() {
    return new ListImpl(db);
  }
  virtual CheckAvailType checkAvail(const std::string& handle,
      NameIdPair& conflict, bool lock) const throw (SQL_ERROR) {
    conflict.id = 0;
    conflict.name = "";
    if (!checkHandleFormat(handle))
      return CA_INVALID_HANDLE;
    if (checkHandleRegistration(handle, conflict, lock))
      return CA_REGISTRED;
    if (checkProtection(handle, 1, "2 month"))
      return CA_PROTECTED;
    return CA_FREE;
  }
};
Manager *Manager::create(DB *db, bool restrictedHandle) {
  return new ManagerImpl(db, restrictedHandle);
}

}
}
