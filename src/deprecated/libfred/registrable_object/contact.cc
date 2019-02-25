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
#include "src/deprecated/libfred/registrable_object/contact.hh"
#include "src/deprecated/libfred/object_impl.hh"
#include "src/deprecated/libfred/sql.hh"
#include "src/deprecated/util/dbsql.hh"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include "src/deprecated/util/log.hh"
#include "src/deprecated/model/model_filters.hh"
#include "util/log/logger.hh"

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

namespace LibFred {
namespace Contact {
static boost::regex format(CONTACT_REGEX);
static boost::regex formatRestricted(CONTACT_REGEX_RESTRICTED);

const std::string Address::Type::MAILING  = "MAILING";
const std::string Address::Type::SHIPPING = "SHIPPING";
const std::string Address::Type::BILLING  = "BILLING";

class AddressImpl : public virtual Address
{
private:
  std::string type;
  std::string company_name;
  std::string street1;
  std::string street2;
  std::string street3;
  std::string province;
  std::string postal_code;
  std::string city;
  std::string country;

public:
  AddressImpl(
          const std::string &_type,
          const std::string &_company_name,
          const std::string &_street1,
          const std::string &_street2,
          const std::string &_street3,
          const std::string &_province,
          const std::string &_postal_code,
          const std::string &_city,
          const std::string &_country
          ) :
      type(_type),
      company_name(_company_name),
      street1(_street1),
      street2(_street2),
      street3(_street3),
      province(_province),
      postal_code(_postal_code),
      city(_city),
      country(_country)
    {
    }

  virtual const std::string& getType() const
  {
      return type;
  }
  /// return contact company name
  virtual const std::string& getCompanyName() const
  {
      return company_name;
  }
  /// return contact street address part 1
  virtual const std::string& getStreet1() const
  {
      return street1;
  }
  /// return contact street address part 2
  virtual const std::string& getStreet2() const
  {
      return street2;
  }
  /// return contact street address part 3
  virtual const std::string& getStreet3() const
  {
      return street3;
  }
  /// return contact state or province
  virtual const std::string& getProvince() const
  {
      return province;
  }
  /// return contact postal code
  virtual const std::string& getPostalCode() const
  {
      return postal_code;
  }
  /// return contact city
  virtual const std::string& getCity() const
  {
      return city;
  }
  /// return contact contry code
  virtual const std::string& getCountry() const
  {
      return country;
  }

  virtual bool operator==(const Address &_other) const
  {
      return getType() == _other.getType() &&
             getCompanyName() == _other.getCompanyName() &&
             getStreet1() == _other.getStreet1() &&
             getStreet2() == _other.getStreet2() &&
             getStreet3() == _other.getStreet3() &&
             getProvince() == _other.getProvince() &&
             getPostalCode() == _other.getPostalCode() &&
             getCity() == _other.getCity() &&
             getCountry() == _other.getCountry();

  }
  virtual bool operator!=(const Address &_other) const
  {
      return !(*this == _other);
  }
};

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
  std::vector<AddressImpl> addresses;
public:
  ContactImpl(TID _id, const Database::ID& _history_id, const std::string& _handle, const std::string& _name,
      TID _registrar, const std::string& _registrarHandle, ptime _crDate,
      ptime _trDate, ptime _upDate, ptime _erDate, TID _createRegistrar,
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
    ObjectImpl(_id, _history_id, _crDate, _trDate, _upDate, _erDate, _registrar, 
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
  virtual unsigned int getAddressCount() const {
      return addresses.size();
  }
  virtual const Address* getAddressByIdx(const unsigned int &_idx) const
  {
      if (_idx < addresses.size())
      {
          return &(addresses[_idx]);
      }
      throw NOT_FOUND();
  }
  AddressImpl* addAddress(const AddressImpl &_addr)
  {
      addresses.push_back(_addr);
      return &(addresses.back());
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
  ListImpl(DBSharedPtr _db) :
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
    std::ostringstream from, where;
    sql.str("");
    if (!count) {
      sql << "INSERT INTO " << getTempTableName() << " ";
    }
    sql << "SELECT " << (count ? "COUNT(DISTINCT c.id) " : "DISTINCT c.id ");
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
    if (!count) {
      where << "ORDER BY c.id ASC ";
    }
    if (limit) {
      where << "LIMIT " << load_limit_ << " ";
    }
    sql << from.str();
    sql << where.str();
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
          (Database::ID)(0), // history_id
          db->GetFieldValue(i,1), // handle
          db->GetFieldValue(i,2), // name
          STR_TO_ID(db->GetFieldValue(i,3)), // registrar id
          registrars[STR_TO_ID(db->GetFieldValue(i,3))], // reg. handle
          MAKE_TIME(i,4), // crdate
          MAKE_TIME(i,5), // trdate
          MAKE_TIME(i,6), // update
          ptime(not_a_date_time),
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
  void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] ContactListImpl::reload()");
    clear();
    Database::SelectQuery id_query;

    // TEMP: should be cached
    std::map<Database::ID, std::string> registrars_table;

    bool at_least_one = false;
    std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      Database::Filters::Contact *cf =
          dynamic_cast<Database::Filters::ContactHistoryImpl* >(fit->get());
      if (!cf)
        continue;
      
      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("historyid", cf->joinContactTable(), "DISTINCT"));
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
    object_info_query.select() << "t_1.id, tmp.id, t_1.name, t_2.name, t_3.clid, "
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
        << "JOIN object_registry t_1 ON (t_3.id = t_1.id)";
    object_info_query.order_by() << "tmp.id";

    try {
      Database::Connection conn = Database::Manager::acquire();

      Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
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

        Database::ID       cid              = *col;
        Database::ID       history_id       = *(++col);
        std::string        handle           = *(++col);
        std::string        name             = *(++col);
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
        std::string        organization     = *(++col);
        std::string        street1          = *(++col);
        std::string        street2          = *(++col);
        std::string        street3          = *(++col);
        std::string        province         = *(++col);
        std::string        postal_code      = *(++col);
        std::string        city             = *(++col);
        std::string        country          = *(++col);
        std::string        telephone        = *(++col);
        std::string        fax              = *(++col);
        std::string        email            = *(++col);
        std::string        notify_email     = *(++col);
        std::string        ssn              = *(++col);
        unsigned           ssn_type         = *(++col);
        std::string        vat              = *(++col);
        bool               disclose_name         = *(++col);
        bool               disclose_organization = *(++col);
        bool               disclose_address      = *(++col);
        bool               disclose_email        = *(++col);
        bool               disclose_telephone    = *(++col);
        bool               disclose_fax          = *(++col);
        bool               disclose_vat          = *(++col);
        bool               disclose_ident        = *(++col);
        bool               disclose_notify_email = *(++col);

        data_.push_back(
            new ContactImpl(
                cid,
                history_id,
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

      resetHistoryIDSequence();
      std::ostringstream addr_query;
      addr_query << "SELECT tmp.id,cah.contactid,cah.type,cah.company_name,cah.street1,"
                 << " cah.street2,cah.street3,cah.stateorprovince,cah.postalcode,"
                 << " cah.city,cah.country"
                 << " FROM " << getTempTableName() << " tmp"
                 << " JOIN contact_address_history cah ON cah.historyid = tmp.id"
                 << " ORDER BY cah.contactid, tmp.id, cah.id";
      Database::Result r_addr = conn.exec(addr_query.str());
      for (Database::Result::Iterator it = r_addr.begin(); it != r_addr.end(); ++it)
      {
          Database::Row::Iterator col = (*it).begin();
          unsigned long long c_hid = *col;
          ++col;//contact id
          std::string type         = *(++col);
          std::string company_name = *(++col);
          std::string street1      = *(++col);
          std::string street2      = *(++col);
          std::string street3      = *(++col);
          std::string province     = *(++col);
          std::string pc           = *(++col);
          std::string city         = *(++col);
          std::string country      = *(++col);

          ContactImpl *cptr = dynamic_cast<ContactImpl*>(findHistoryIDSequence(c_hid));
          if (cptr)
          {
              cptr->addAddress(AddressImpl(type, company_name, street1, street2, street3, province, pc, city, country));
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
  DBSharedPtr db; ///< connection do db
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
                               bool lock) const
  {
      try {
          std::ostringstream sql;
          sql << "SELECT id,name FROM object_registry "
              << "WHERE type=1 AND erDate ISNULL AND " << "UPPER(name)=UPPER('"
              << handle << "')";
          if (lock)
            sql << " FOR UPDATE ";

          /* we really don't want to break epp because of different transaction */
          if (db.get()) {
              if (!db->ExecSelect(sql.str().c_str()))
                  throw SQL_ERROR();
              bool result = db->GetSelectRows() >= 1;
              conflict.id = result ? STR_TO_ID(db->GetFieldValue(0, 0)) : 0;
              conflict.name = result ? db->GetFieldValue(0, 1) : "";
              db->FreeSelect();
              return result;
          }
          else {
              Database::Connection conn = Database::Manager::acquire();
              Database::Result result = conn.exec(sql.str());
              bool result_ok = result.size() >= 1;
              conflict.id = result_ok ? static_cast<unsigned long long>(result[0][0]) : 0;
              conflict.name = result_ok ? static_cast<std::string>(result[0][1]) : "";
              return result_ok;
          }
      }
      catch (...) {
          throw SQL_ERROR();
      }
  }
  /// check if object handle is in protection period (true=protected)
  bool checkProtection(const std::string& name, unsigned type,
                       const std::string& monthPeriodSQL) const
  {
      try {
          std::stringstream sql;
          sql << "SELECT COALESCE(" << "MAX(erdate) + (" << monthPeriodSQL
              << ")::interval" << " > CURRENT_TIMESTAMP, false) " << "FROM object_registry "
              << "WHERE NOT(erdate ISNULL) " << "AND type=" << type << " "
              << "AND UPPER(name)=UPPER('" << name << "')";

          /* we really don't want to break epp because of different transaction */
          if (db.get()) {
              if (!db->ExecSelect(sql.str().c_str())) {
                db->FreeSelect();
                throw SQL_ERROR();
              }
              bool ret = (db->GetFieldValue(0,0)[0] == 't');
              db->FreeSelect();
              return ret;
          }
          else {
              Database::Connection conn = Database::Manager::acquire();
              Database::Result result = conn.exec(sql.str());
              return static_cast<bool>(result[0][0]) == true;
          }
      }
      catch (...) {
          throw SQL_ERROR();
      }
  }
public:
  ManagerImpl(DBSharedPtr _db, bool _restrictedHandle) :
    db(_db), restrictedHandle(_restrictedHandle) {
  }
  virtual List *createList() {
    return new ListImpl(db);
  }
  virtual CheckAvailType checkAvail(const std::string& handle,
      NameIdPair& conflict, bool lock) const {
    conflict.id = 0;
    conflict.name = "";
    if (!checkHandleFormat(handle))
      return CA_INVALID_HANDLE;
    if (checkHandleRegistration(handle, conflict, lock))
      return CA_REGISTRED;
    if (checkProtection(handle, 1, "(SELECT val FROM enum_parameters WHERE id = 12) || ' month'"))
      return CA_PROTECTED;
    return CA_FREE;
  }

  virtual CheckAvailType checkAvail(
          const unsigned long long &_id,
          NameIdPair& conflict, bool lock) const
  {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result rcheck = conn.exec_params(
              "SELECT oreg.name FROM object_registry oreg"
              " JOIN contact c ON c.id = oreg.id WHERE c.id = $1::integer",
              Database::query_param_list(_id));
      if (rcheck.size() == 1) {
          return this->checkAvail(
                  static_cast<std::string>(rcheck[0][0]),
                  conflict,
                  lock);
      }
      else {
          return CA_FREE;
      }
  }

  virtual unsigned long long findRegistrarId(
          const unsigned long long &_id) const 
  {
      Database::Connection conn = Database::Manager::acquire();

      Database::Result res = conn.exec_params(
              "SELECT o.clid FROM object o "
                 "JOIN contact c ON o.id=c.id "
                 "WHERE c.id = $1::integer",
                 Database::query_param_list(_id));
      if(res.size() == 1) {
          return res[0][0];
      } else {
          return 0;
      }

  }
};
Manager *Manager::create(DBSharedPtr db, bool restrictedHandle) {
  return new ManagerImpl(db, restrictedHandle);
}

}
}
