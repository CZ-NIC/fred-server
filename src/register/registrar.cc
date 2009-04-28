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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <vector>
#include <algorithm>
#include <functional>

#include "registrar.h"
#include "common_impl.h"
#include "sql.h"
#include "old_utils/dbsql.h"
#include "old_utils/log.h"

#include "db_settings.h"
#include "model/model_filters.h"
#include "log/logger.h"

#define SET(a,b) { a = b; changed = true; }

namespace Register {
namespace Registrar {

class RegistrarImpl;


class ACLImpl : virtual public ACL {
  unsigned id;
  std::string certificateMD5;
  std::string password;
  bool changed;
  friend class RegistrarImpl;

public:
  ACLImpl() :
    id(0), changed(true) {
  }
  ACLImpl(unsigned _id,
          const std::string _certificateMD5,
          const std::string _password) :
    id(_id), certificateMD5(_certificateMD5), password(_password),
        changed(false) {
  }
  virtual const std::string& getCertificateMD5() const {
    return certificateMD5;
  }
  virtual void setCertificateMD5(const std::string& newCertificateMD5) {
    SET(certificateMD5,newCertificateMD5);
  }
  virtual const std::string& getPassword() const {
    return password;
  }
  virtual void setPassword(const std::string& newPassword) {
    SET(password,newPassword);
  }
  bool operator==(unsigned _id) {
    return id == _id;
  }
  bool hasChanged() const {
    return changed;
  }
  std::string makeSQL(unsigned registrarId) {
    std::ostringstream sql;
    sql << "INSERT INTO registraracl (registrarid,cert,password) VALUES "
        << "(" << registrarId << ",'" << certificateMD5 << "','" << password
        << "');";
    return sql.str();
  }
  void resetId() {
    changed = true;
  }
};


class RegistrarImpl : public Register::CommonObjectImpl,
                      virtual public Registrar {
  
  typedef std::vector<ACLImpl *> ACLList;
  typedef ACLList::iterator ACLListIter;
  DB *db; ///< db connection
  std::string ico; ///< DB: registrar.ico
  std::string dic; ///< DB: registrar.dic
  std::string var_symb; ///< DB: registrar.varsymb
  bool vat; ///< DB: registrar.vat
  std::string handle; ///< DB: registrar.handle
  std::string name; ///< DB: registrar.name
  std::string url; ///< DB: registrar.name
  std::string password; ///< DB: registraracl.passwd
  std::string organization; ///< DB: registrar.organization
  std::string street1; ///< DB: registrar.street1
  std::string street2; ///< DB: registrar.street2
  std::string street3; ///< DB: registrar.street3
  std::string city; ///< DB: registrar.city
  std::string province; ///< DB: registrar.stateorprovince
  std::string postalCode; ///< DB: registrar.postalcode
  std::string country; ///< DB: registrar.country
  std::string telephone; ///< DB: registrar.telephone
  std::string fax; ///< DB: registrar.fax
  std::string email; ///< DB: registrar.email
  bool system; ///< DB: registrar.system
  unsigned long credit; ///< DB: registrar.credit
  bool changed; ///< object was changed, need sync to database
  ACLList acl; ///< access control
  std::map<Database::ID, unsigned long> zone_credit_map;

public:
  RegistrarImpl(DB *_db) :
    CommonObjectImpl(0), db(_db), changed(true) {
  }
  RegistrarImpl(DB *_db,
                TID _id,
                const std::string& _ico,
                const std::string& _dic,
                const std::string& _var_symb,
                bool _vat,
                const std::string& _handle,
                const std::string& _name,
                const std::string& _url,
                const std::string& _organization,
                const std::string& _street1,
                const std::string& _street2,
                const std::string& _street3,
                const std::string& _city,
                const std::string& _province,
                const std::string& _postalCode,
                const std::string& _country,
                const std::string& _telephone,
                const std::string& _fax,
                const std::string& _email,
                bool _system,
                unsigned long _credit) :
        CommonObjectImpl(_id),
        db(_db), ico(_ico), dic(_dic), var_symb(_var_symb), vat(_vat),
        handle(_handle), name(_name), url(_url), organization(_organization),
        street1(_street1), street2(_street2), street3(_street3), city(_city),
        province(_province), postalCode(_postalCode), country(_country),
        telephone(_telephone), fax(_fax), email(_email), system(_system), 
        credit(_credit) {
  }
  void clear() {
    for (unsigned i=0; i<acl.size(); i++)
      delete acl[i];
    acl.clear();
  }
  ~RegistrarImpl() {
    clear();
  }

  virtual const std::string& getIco() const {
    return ico;
  }
    
  virtual void setIco(const std::string& _ico) {
    SET(ico, _ico);
  }
  
  virtual const std::string& getDic() const {
    return dic;
  }
  
  virtual void setDic(const std::string& _dic) {
    SET(dic, _dic);
  }
  
  virtual const std::string& getVarSymb() const {
    return var_symb;
  }
  
  virtual void setVarSymb(const std::string& _var_symb) {
    SET(var_symb, _var_symb);
  }
  
  virtual bool getVat() const {
    return vat;
  }
  
  virtual void setVat(bool _vat) {
    SET(vat, _vat);
  }
    
  virtual const std::string& getHandle() const {
    return handle;
  }
  
  virtual void setHandle(const std::string& newHandle) {
    SET(handle,newHandle);
  }
  
  virtual const std::string& getName() const {
    return name;
  }
  
  virtual void setName(const std::string& newName) {
    SET(name,newName);
  }
  
  virtual const std::string& getURL() const {
    return url;
  }
  
  virtual void setURL(const std::string& newURL) {
    SET(url,newURL);
  }
  
  virtual const std::string& getOrganization() const {
    return organization;
  }
  
  virtual void setOrganization(const std::string& _organization) {
    SET(organization,_organization);
  }
  
  virtual const std::string& getStreet1() const {
    return street1;
  }
  
  virtual void setStreet1(const std::string& _street1) {
    SET(street1,_street1);
  }
  
  virtual const std::string& getStreet2() const {
    return street2;
  }
  
  virtual void setStreet2(const std::string& _street2) {
    SET(street2,_street2);
  }
  
  virtual const std::string& getStreet3() const {
    return street3;
  }
  
  virtual void setStreet3(const std::string& _street3) {
    SET(street3,_street3);
  }
  
  virtual const std::string& getCity() const {
    return city;
  }
  
  virtual void setCity(const std::string& _city) {
    SET(city,_city);
  }
  
  virtual const std::string& getProvince() const {
    return province;
  }
  
  virtual void setProvince(const std::string& _province) {
    SET(province,_province);
  }
  
  virtual const std::string& getPostalCode() const {
    return postalCode;
  }
  
  virtual void setPostalCode(const std::string& _postalCode) {
    SET(postalCode,_postalCode);
  }
  
  virtual const std::string& getCountry() const {
    return country;
  }
  
  virtual void setCountry(const std::string& _country) {
    SET(country,_country);
  }
  
  virtual const std::string& getTelephone() const {
    return telephone;
  }
  
  virtual void setTelephone(const std::string& _telephone) {
    SET(telephone,_telephone);
  }
  
  virtual const std::string& getFax() const {
    return fax;
  }
  
  virtual void setFax(const std::string& _fax) {
    SET(fax,_fax);
  }
  
  virtual const std::string& getEmail() const {
    return email;
  }
  
  virtual void setEmail(const std::string& _email) {
    SET(email,_email);
  }
  
  virtual bool getSystem() const {
    return system;
  }
  
  virtual void setSystem(bool _system) {
    SET(system, _system);
  }
  
  virtual unsigned long getCredit() const {
    return credit;
  }
  
  virtual unsigned long getCredit(Database::ID _zone_id) {
    return zone_credit_map[_zone_id];
  }
  
  virtual void setCredit(Database::ID _zone_id, unsigned long _credit) {
    zone_credit_map[_zone_id] = _credit;
    credit += _credit;
  }
  
  virtual unsigned getACLSize() const {
    return acl.size();
  }
  
  virtual ACL* getACL(unsigned idx) const {
    return idx < acl.size() ? acl[idx] : NULL;
  }
  
  virtual ACL* newACL() {
    ACLImpl* newACL = new ACLImpl();
    acl.push_back(newACL);
    return newACL;
  }
  
  virtual void deleteACL(unsigned idx) {
    if (idx < acl.size()) {
      delete acl[idx];
      acl.erase(acl.begin()+idx);
    }
  }
  
  virtual void clearACLList() {
    clear();
  }
  
  virtual void save() throw (SQL_ERROR) {
    if (changed) {
      // save registrar data
      /*
       * SQL_SAVE(sql,"registrar",id);
       * SQL_SAVE_ADD(sql,"name",name);
       * SQL_SAVE_ADD(sql,"handle",handle);
       * SQL_SAVE_ADD(sql,"url",url);
       * SQL_SAVE_DOIT(sql,db);
       */
      
      /*
       * TODO: Folowing SQLs should be propably done in transaction
       */ 
      std::ostringstream sql;
      if (id_) {
        sql << "UPDATE registrar SET " 
            << "ico = '" << getIco() << "', "
            << "dic = '" << getDic() << "', " 
            << "varsymb = '" << getVarSymb() << "', "
            << "vat = " << (getVat() ? "true" : "false") << ", "
            << "name = '" << getName() << "', "
            << "handle = '" << getHandle() << "', " 
            << "url = '" << getURL() << "', " 
            << "organization = '" << getOrganization() << "', "
            << "street1 = '" << getStreet1() << "', " 
            << "street2 = '" << getStreet2() << "', " 
            << "street3 = '" << getStreet3() << "', "
            << "city = '" << getCity() << "', " 
            << "stateorprovince = '" << getProvince() << "', " 
            << "postalcode = '" << getPostalCode() << "', " 
            << "country = '" << getCountry() << "', " 
            << "telephone = '" << getTelephone() << "', " 
            << "fax = '" << getFax() << "', "
            << "email = '" << getEmail() << "', "
            << "system = " << (getSystem() ? "true" : "false") << " "
            << "WHERE id = " << id_;
      } else {
        id_ = db->GetSequenceID("registrar");
        sql << "INSERT INTO registrar "
            << "(id, ico, dic, varsymb, vat, name, handle, url, organization, street1, street2, "
            << "street3, city, stateorprovince, postalcode, country, "
            << "telephone, fax, email, system) " 
            << "VALUES " << "(" 
            // << "DEFAULT" << ", "
            << id_ << ", "
            << "'" << getIco() << "', "
            << "'" << getDic() << "', "
            << "'" << getVarSymb() << "', "
            << "" << (getVat() ? "true" : "false") << ", "
            << "'" << getName() << "', " 
            << "'" << getHandle() << "', " 
            << "'" << getURL() << "', " 
            << "'" << getOrganization() << "', "
            << "'" << getStreet1() << "', " 
            << "'" << getStreet2() << "', "
            << "'" << getStreet3() << "', " 
            << "'" << getCity() << "', " 
            << "'" << getProvince() << "', " 
            << "'" << getPostalCode() << "', " 
            << "'" << getCountry() << "', " 
            << "'" << getTelephone() << "', " 
            << "'" << getFax() << "', " 
            << "'" << getEmail() << "', "
            << "" << (getSystem() ? "true" : "false") << ""
            << ")";
      }
      if (!db->ExecSQL(sql.str().c_str()))
        throw SQL_ERROR();
    }
    ACLList::const_iterator i = find_if(acl.begin(),
                                        acl.end(),
                                        std::mem_fun(&ACLImpl::hasChanged) );
    {
      std::ostringstream sql;
      sql << "DELETE FROM registraracl WHERE registrarid=" << id_;
      if (!db->ExecSQL(sql.str().c_str()))
        throw SQL_ERROR();
      for (unsigned j = 0; j < acl.size(); j++) {
        sql.str("");
        sql << acl[j]->makeSQL(id_);
        if (!db->ExecSQL(sql.str().c_str()))
          throw SQL_ERROR();
      }
    }
  }
  
  void putACL(unsigned id,
              const std::string& certificateMD5,
              const std::string& password) {
    acl.push_back(new ACLImpl(id,certificateMD5,password));
  }
  bool hasId(unsigned _id) const {
    return id_ == _id;
  }
  void resetId() {
    id_ = 0;
    changed = true;
    for (unsigned i = 0; i < acl.size(); i++)
      acl[i]->resetId();
  }
};

COMPARE_CLASS_IMPL(RegistrarImpl, Name)
COMPARE_CLASS_IMPL(RegistrarImpl, Handle)
COMPARE_CLASS_IMPL(RegistrarImpl, URL)
COMPARE_CLASS_IMPL(RegistrarImpl, Email)
COMPARE_CLASS_IMPL(RegistrarImpl, Credit)

class RegistrarListImpl : public Register::CommonListImpl,
                          virtual public RegistrarList {
  
  std::string name;
  std::string handle;
  std::string xml;
  TID idFilter;
  std::string zoneFilter;
public:
  RegistrarListImpl(DB *_db) :
    CommonListImpl(_db), idFilter(0) {
  }
  ~RegistrarListImpl() {
    clear();
  }
  virtual void setIdFilter(TID _idFilter) {
    idFilter = _idFilter;
  }
  virtual void setHandleFilter(const std::string& _handle) {
    handle = _handle;
  }
  virtual void setXMLFilter(const std::string& _xml) {
    xml = _xml;
  }
  virtual void setNameFilter(const std::string& _name) {
    name = _name;
  }
  virtual void setZoneFilter(const std::string& _zone) {
    zoneFilter = _zone;
  }
  virtual void reload() throw (SQL_ERROR) {
    clear();
    std::ostringstream sql;
    sql << "SELECT r.id,r.handle,r.name,r.url,r.organization,"
        << "r.street1,r.street2,r.street3,r.city,r.stateorprovince,"
        << "r.postalcode,r.country,r.telephone,r.fax,r.email,r.system,"
        << "COALESCE(SUM(i.credit),0) " << "FROM registrar r "
        << "LEFT JOIN invoice i ON (r.id=i.registrarid AND "
        << "NOT(i.credit ISNULL)) ";
    if (!zoneFilter.empty())
      sql << "LEFT JOIN (SELECT z.fqdn, ri.registrarid "
          << "FROM zone z, registrarinvoice ri "
          << "WHERE z.id=ri.zone AND ri.fromdate<=CURRENT_DATE) t "
          << "ON (t.registrarid=r.id AND t.fqdn='" << zoneFilter << "') ";
    sql << "WHERE 1=1 ";
    if (!zoneFilter.empty())
      sql << "AND NOT(t.fqdn ISNULL) ";
    SQL_ID_FILTER(sql, "r.id", idFilter);
    SQL_HANDLE_FILTER(sql, "r.name", name);
    SQL_HANDLE_WILDCHECK_FILTER(sql, "r.handle", handle, false, false);
    sql << "GROUP BY r.id,r.handle,r.name,r.url,r.organization,"
        << "r.street1,r.street2,r.street3,r.city,r.stateorprovince,"
        << "r.postalcode,r.country,r.telephone,r.fax,r.email,r.system ";
    sql << "ORDER BY r.id ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      data_.push_back(new RegistrarImpl(
          db,
          STR_TO_ID(db->GetFieldValue(i,0)),
          "",
          "",
          "",
          true,
          db->GetFieldValue(i,1),
          db->GetFieldValue(i,2),
          db->GetFieldValue(i,3),
          db->GetFieldValue(i,4),
          db->GetFieldValue(i,5),
          db->GetFieldValue(i,6),
          db->GetFieldValue(i,7),
          db->GetFieldValue(i,8),
          db->GetFieldValue(i,9),
          db->GetFieldValue(i,10),
          db->GetFieldValue(i,11),
          db->GetFieldValue(i,12),
          db->GetFieldValue(i,13),
          db->GetFieldValue(i,14),
          (*db->GetFieldValue(i,15) == 't'),
          atol(db->GetFieldValue(i,16))  
      ));
    }
    db->FreeSelect();
    sql.str("");
    sql << "SELECT registrarid,cert,password " << "FROM registraracl ORDER BY registrarid";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();

    resetIDSequence();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      // find associated registrar
      unsigned registrarId = STR_TO_ID(db->GetFieldValue(i, 0));
      RegistrarImpl *r = dynamic_cast<RegistrarImpl* >(findIDSequence(registrarId));
      if (r) {
        r->putACL(0, db->GetFieldValue(i, 1), db->GetFieldValue(i, 2));
      }
    }
    db->FreeSelect();
  }
  virtual void reload(Database::Filters::Union &uf, Database::Manager *dbm) {
    TRACE("[CALL] RegistrarListImpl::reload()");
    clear();
    uf.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery info_query;
    std::auto_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      Database::Filters::Registrar *rf =
          dynamic_cast<Database::Filters::Registrar*>(fit->get());
      if (!rf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect("id ico dic varsymb vat handle name url organization street1 street2 street3 "
                       "city stateorprovince postalcode country telephone fax email system",
                     rf->joinRegistrarTable());
      uf.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    info_query.limit(load_limit_);
    uf.serialize(info_query);
    try {
      std::auto_ptr<Database::Connection> conn(dbm->getConnection());
      Database::Result r_info = conn->exec(info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID  rid          = *col;
        std::string   ico          = *(++col);
        std::string   dic          = *(++col);
        std::string   var_symb     = *(++col);
        bool          vat          = *(++col);
        std::string   handle       = *(++col);
        std::string   name         = *(++col);
        std::string   url          = *(++col);
        std::string   organization = *(++col);
        std::string   street1      = *(++col);
        std::string   street2      = *(++col);
        std::string   street3      = *(++col);
        std::string   city         = *(++col);
        std::string   province     = *(++col);
        std::string   postal_code  = *(++col);
        std::string   country      = *(++col);
        std::string   telephone    = *(++col);
        std::string   fax          = *(++col);
        std::string   email        = *(++col);
        bool          system       = *(++col);
        unsigned long credit       = 0;

        data_.push_back(new RegistrarImpl(
                db,
                rid,
                ico,
                dic,
                var_symb,
                vat,
                handle,
                name,
                url,
                organization,
                street1,
                street2,
                street3,
                city,
                province,
                postal_code,
                country,
                telephone,
                fax,
                email,
                system,
                credit));
      }

      if (data_.empty())
        return;
      
      Database::SelectQuery credit_query;
      credit_query.select() << "zone, registrarid, COALESCE(SUM(credit), 0)";
      credit_query.from() << "invoice";
      credit_query.group_by() << "registrarid, zone";
      credit_query.order_by() << "registrarid";

      resetIDSequence();
      Database::Result r_credit = conn->exec(credit_query);
      for (Database::Result::Iterator it = r_credit.begin(); it != r_credit.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID  zone_id      = *col;
        Database::ID  registrar_id = *(++col);
        unsigned long credit       = *(++col);
        
        RegistrarImpl *registrar_ptr = dynamic_cast<RegistrarImpl* >(findIDSequence(registrar_id));
        if (registrar_ptr) {
          registrar_ptr->setCredit(zone_id, credit);
        }
      }
      
      resetIDSequence();
      Database::SelectQuery acl_query;
      acl_query.select() << "id, registrarid, cert, password ";
      acl_query.from() << "registraracl";
      acl_query.order_by() << "registrarid";
      
      Database::Result r_acl = conn->exec(acl_query);
      for (Database::Result::Iterator it = r_acl.begin(); it != r_acl.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID acl_id       = *col;
        Database::ID registrar_id = *(++col);
        std::string  cert_md5     = *(++col);
        std::string  password     = *(++col);

        RegistrarImpl *registrar_ptr = dynamic_cast<RegistrarImpl* >(findIDSequence(registrar_id));
        if (registrar_ptr) {
          registrar_ptr->putACL(acl_id, cert_md5, password);
        }
      }
      /* checks if row number result load limit is active and set flag */ 
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }
//  virtual const Registrar* get(unsigned idx) const {
//    if (idx> size())
//      return NULL;
//    return data_[idx];
//  }
  
  virtual Registrar* get(unsigned _idx) const {
    try {
      Registrar *registrar = dynamic_cast<Registrar* >(data_.at(_idx));
      if (registrar) 
        return registrar;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    } 
  }
  
//  Registrar* findId(Database::ID _id) const throw (Register::NOT_FOUND) {
//    RegistrarListType::const_iterator it = std::find_if(data_.begin(),
//                                                        data_.end(),
//                                                        CheckId(_id));
//    if (it != data_.end()) {
//      LOGGER(PACKAGE).debug(boost::format("object list hit! object id=%1% found")
//          % _id);
//      return *it;
//    }
//    LOGGER(PACKAGE).debug(boost::format("object list miss! object id=%1% not found")
//        % _id);
//    throw Register::NOT_FOUND();
//  }
  virtual Registrar* create() {
    return new RegistrarImpl(db);
  }
  void clearFilter() {
    name = "";
    handle = "";
    zoneFilter = "";
  }
  
  virtual void sort(MemberType _member, bool _asc) {
    switch (_member) {
      case MT_NAME:
        stable_sort(data_.begin(), data_.end(), CompareName(_asc));
        break;
      case MT_HANDLE:
        stable_sort(data_.begin(), data_.end(), CompareHandle(_asc));
        break;
      case MT_URL:
        stable_sort(data_.begin(), data_.end(), CompareURL(_asc));
        break;
      case MT_MAIL:
        stable_sort(data_.begin(), data_.end(), CompareEmail(_asc));
        break;
      case MT_CREDIT:
        stable_sort(data_.begin(), data_.end(), CompareCredit(_asc));
        break;
    }
  }
  
  virtual void makeQuery(bool, bool, std::stringstream&) const {
    /* dummy implementation */
  }
  
  virtual const char* getTempTableName() const {
    return ""; /* dummy implementation */ 
  }
};

class EPPActionImpl : public CommonObjectImpl,
                      virtual public EPPAction {
  TID sessionId;
  unsigned type;
  std::string typeName;
  ptime startTime;
  std::string serverTransactionId;
  std::string clientTransactionId;
  unsigned result;
  TID registrarId;
  std::string registrarHandle;
  std::string handle;
  std::string message;
  std::string message_out;
  
public:
  EPPActionImpl(TID _id,
                TID _sessionId,
                unsigned _type,
                const std::string& _typeName,
                ptime _startTime,
                const std::string& _serverTransactionId,
                const std::string& _clientTransactionId,
                unsigned _result,
                TID _registrarId,
                const std::string& _registrarHandle,
                const std::string& _handle,
                const std::string& _message = std::string(),
                const std::string& _message_out = std::string()) :
    CommonObjectImpl(_id), sessionId(_sessionId), type(_type), typeName(_typeName),
        startTime(_startTime), serverTransactionId(_serverTransactionId),
        clientTransactionId(_clientTransactionId), result(_result), registrarId(_registrarId),
        registrarHandle(_registrarHandle), handle(_handle), 
        message(_message), message_out(_message_out) {
  }
  
  virtual TID getSessionId() const {
    return sessionId;
  }
  virtual unsigned getType() const {
    return type;
  }
  virtual const std::string& getTypeName() const {
    return typeName;
  }
  virtual const ptime getStartTime() const {
    return startTime;
  }
  virtual const std::string& getServerTransactionId() const {
    return serverTransactionId;
  }
  virtual const std::string& getClientTransactionId() const {
    return clientTransactionId;
  }
  virtual const std::string& getEPPMessageIn() const {
    return message;
  }
  virtual const std::string& getEPPMessageOut() const {
    return message_out;
  }
  virtual unsigned getResult() const {
    return result;
  }
  virtual std::string getResultStatus() const {
    return (result < 2000 && result) ? "OK" : "FAILED";
  }
  virtual TID getRegistrarId() const {
    return registrarId;
  }
  virtual const std::string& getRegistrarHandle() const {
    return registrarHandle;
  }
  virtual const std::string& getHandle() const {
    return handle;
  }
};


/*
 * TEMP: Have to use own COMPARE_CLASS_IMPL because EPPAction class
 * is in same namespace as Registrar need prefix compare class name
 * - split file? split managers?
 */

#undef COMPARE_CLASS_IMPL
#define COMPARE_CLASS_IMPL(_object_type, _by_what)                        \
class EPPActionCompare##_by_what {                                        \
public:                                                                   \
  EPPActionCompare##_by_what(bool _asc) : asc_(_asc) { }                  \
  bool operator()(Register::CommonObject *_left,                          \
                  Register::CommonObject *_right) const {                 \
    _object_type *l_casted = dynamic_cast<_object_type *>(_left);         \
    _object_type *r_casted = dynamic_cast<_object_type *>(_right);        \
    if (l_casted == 0 || r_casted == 0) {                                 \
      /* this should never happen */                                      \
      throw std::bad_cast();                                              \
    }                                                                     \
                                                                          \
    bool result = l_casted->get##_by_what() <= r_casted->get##_by_what(); \
    return (asc_ && result) || (!asc_ && !result);  	                    \
}                                                                         \
private:                                                                  \
  bool asc_;                                                              \
};


COMPARE_CLASS_IMPL(EPPActionImpl, ServerTransactionId)
COMPARE_CLASS_IMPL(EPPActionImpl, ClientTransactionId)
COMPARE_CLASS_IMPL(EPPActionImpl, Type)
COMPARE_CLASS_IMPL(EPPActionImpl, Handle)
COMPARE_CLASS_IMPL(EPPActionImpl, RegistrarHandle)
COMPARE_CLASS_IMPL(EPPActionImpl, StartTime)
COMPARE_CLASS_IMPL(EPPActionImpl, Result)

#undef COMPARE_CLASS_IMPL

/// List of EPPAction objects
class EPPActionListImpl : public Register::CommonListImpl,
                          virtual public EPPActionList {
  TID id;
  TID sessionId;
  TID registrarId;
  std::string registrarHandle;
  time_period period;
  unsigned typeId;
  std::string type;
  unsigned returnCodeId;
  std::string clTRID;
  std::string svTRID;
  std::string handle;
  std::string xml;
  EPPActionResultFilter result;
  bool partialLoad;
public:
  EPPActionListImpl(DB *_db) : CommonListImpl(_db),
                               id(0), 
                               sessionId(0), 
                               registrarId(0), 
                               period(ptime(neg_infin), ptime(pos_infin)), 
                               typeId(0),
                               returnCodeId(0), 
                               result(EARF_ALL),  
                               partialLoad(false) {
  }
  ~EPPActionListImpl() {
    clear();
  }
  void setIdFilter(TID _id) {
    id = _id;
  }
  void setSessionFilter(TID _sessionId) {
    sessionId = _sessionId;
  }
  void setRegistrarFilter(TID _registrarId) {
    registrarId = _registrarId;
  }
  void setRegistrarHandleFilter(const std::string& _registrarHandle) {
    registrarHandle = _registrarHandle;
  }
  void setTimePeriodFilter(const time_period& _period) {
    period = _period;
  }
  virtual void setTypeFilter(unsigned _typeId) {
    typeId = _typeId;
  }
  virtual void setReturnCodeFilter(unsigned _returnCodeId) {
    returnCodeId = _returnCodeId;
  }
  virtual void setResultFilter(EPPActionResultFilter _result) {
    result = _result;
  }
  virtual void setHandleFilter(const std::string& _handle) {
    handle = _handle;
  }
  virtual void setXMLFilter(const std::string& _xml) {
    xml = _xml;
  }
  virtual void setTextTypeFilter(const std::string& _textType) {
    type = _textType;
  }
  virtual void setClTRIDFilter(const std::string& _clTRID) {
    clTRID = _clTRID;
  }
  virtual void setSvTRIDFilter(const std::string& _svTRID) {
    svTRID = _svTRID;
  }
#define DB_NULL_INT(i,j) \
  (db->IsNotNull(i,j)) ? atoi(db->GetFieldValue(i,j)) : 0
#define DB_NULL_STR(i,j) \
  (db->IsNotNull(i,j)) ? db->GetFieldValue(i,j) : ""
  virtual void reload() {
    clear();
    std::ostringstream sql;
    sql << "SELECT a.id,a.clientid,a.action,ea.status,a.startdate,"
        << "a.servertrid,a.clienttrid, a.response,r.id,r.handle,MIN(al.value) ";
    if (partialLoad)
      sql << ",'','' ";
    else
      sql << ",ax.xml,ax.xml_out ";
    sql << "FROM action a "
        << "JOIN enum_action ea ON (a.action=ea.id) "
        << "JOIN enum_error er ON (a.response=er.id) "
        << "JOIN login l ON (l.id=a.clientid) "
        << "JOIN registrar r ON (r.id=l.registrarid) "
        << "LEFT JOIN action_elements al ON (a.id=al.actionid) ";
    if (!partialLoad)
      sql << "LEFT JOIN action_xml ax ON (a.id=ax.actionid) ";
    sql << "WHERE 1=1 ";
    SQL_ID_FILTER(sql, "a.id", id);
    SQL_ID_FILTER(sql, "r.id", registrarId);
    SQL_HANDLE_WILDCHECK_FILTER(sql, "r.handle", registrarHandle, 1, 0);
    SQL_TIME_FILTER(sql, "a.startdate", period)
    ;
    SQL_HANDLE_WILDCHECK_FILTER(sql, "ea.status", type, 1, 0);
    SQL_ID_FILTER(sql, "a.response", returnCodeId);
    SQL_HANDLE_WILDCHECK_FILTER(sql, "a.clienttrid", clTRID, 1, 0);
    SQL_HANDLE_WILDCHECK_FILTER(sql, "a.servertrid", svTRID, 1, 0);
    /// TODO - handle has to have special data column
    SQL_HANDLE_WILDCHECK_FILTER(sql, "al.value", handle, 1, 0);
    if (result != EARF_ALL)
      sql << "AND (a.response "
          << (result == EARF_OK ? "<" : " IS NULL OR a.response >=")
          << " 2000) ";
    sql << "GROUP BY a.id,a.clientid,a.action,ea.status,a.startdate,"
        << "a.servertrid,a.clienttrid,a.response,r.handle ";
    if (!partialLoad)
      sql << ",ax.xml,ax.xml_out ";
    sql << "LIMIT 1000";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      data_.push_back(new EPPActionImpl(
          STR_TO_ID(db->GetFieldValue(i,0)),
          DB_NULL_INT(i,1),
          atoi(db->GetFieldValue(i,2)),
          db->GetFieldValue(i,3),
          ptime(time_from_string(db->GetFieldValue(i,4))),
          db->GetFieldValue(i,5),
          db->GetFieldValue(i,6),
          DB_NULL_INT(i,7),
          STR_TO_ID(db->GetFieldValue(i,8)),
          DB_NULL_STR(i,9),
          DB_NULL_STR(i,10),
          DB_NULL_STR(i,11),
          DB_NULL_STR(i,12)
      ));
    }
    db->FreeSelect();
  }

  virtual EPPAction* get(unsigned _idx) const {
    try {
      EPPAction *action = dynamic_cast<EPPAction* >(data_.at(_idx));
      if (action) 
        return action;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    } 
  }
  
  virtual void clearFilter() {
    id = 0;
    sessionId = 0;
    registrarId = 0;
    registrarHandle = "";
    period = time_period(ptime(neg_infin), ptime(pos_infin));
    typeId = 0;
    type = "";
    returnCodeId = 0;
    clTRID = "";
    svTRID = "";
    handle = "";
    xml = "";
    result = EARF_ALL;
    partialLoad = false;
  }
  virtual void reload(Database::Filters::Union &uf, Database::Manager *dbm) {
    TRACE("[CALL] EPPActionListImpl::reload()");
    clear();
    uf.clearQueries();

    // TEMP: should be cached for quicker
    std::map<Database::ID, std::string> registrars_table;

    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Union::iterator fit = uf.begin();
    for (; fit != uf.end(); ++fit) {
      Database::Filters::EppAction *eaf =
          dynamic_cast<Database::Filters::EppAction*>(*fit);
      if (!eaf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect("id", eaf->joinActionTable());
      uf.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    id_query.limit(load_limit_);
    uf.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery("tmp_eppaction_filter_result", id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%")
        % "tmp_eppaction_filter_result" % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select()
        << "t_1.id, t_1.clientid, t_1.action, t_2.status, t_1.startdate, "
        << "t_1.servertrid, t_1.clienttrid, t_1.response, "
        << "t_4.registrarid, MIN(t_5.value)";
    if (!partialLoad) {
      object_info_query.select() << ", t_6.xml, t_6.xml_out";
    }
    else {
      object_info_query.select() << ", '', ''";
    }
    object_info_query.from() << "tmp_eppaction_filter_result tmp "
                             << "JOIN action t_1 ON(tmp.id = t_1.id) "
                             << "JOIN enum_action t_2 ON (t_1.action = t_2.id) "
                             << "LEFT JOIN enum_error t_3 ON (t_1.response = t_3.id) "
                             << "LEFT JOIN login t_4 ON (t_1.clientid = t_4.id) "
                             << "LEFT JOIN action_elements t_5 ON (t_1.id = t_5.actionid)";
    if (!partialLoad)
      object_info_query.from() << "LEFT JOIN action_xml t_6 ON (t_1.id = t_6.actionid) ";

    object_info_query.order_by() << "tmp.id";
    object_info_query.group_by() << "t_1.id, t_1.clientid, t_1.action, "
                                 << "tmp.id, t_2.status, t_1.startdate, t_1.servertrid, t_1.clienttrid, "
                                 << "t_1.response, t_4.registrarid";
    if (!partialLoad) {
      object_info_query.group_by() << ", t_6.xml, t_6.xml_out";
    }

    try {
      std::auto_ptr<Database::Connection> conn(dbm->getConnection());

      Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string("tmp_eppaction_filter_result") + "')");
      conn->exec(create_tmp_table);
      conn->exec(tmp_table_query);

      // TEMP: should be cached somewhere
      Database::Query registrars_query("SELECT id, handle FROM registrar");
      Database::Result r_registrars = conn->exec(registrars_query);
      Database::Result::Iterator it = r_registrars.begin();
      for (; it != r_registrars.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID      id = *col;
        std::string   handle = *(++col);
        registrars_table[id] = handle;
      }

      Database::Result r_info = conn->exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID       aid              = *col;
        Database::ID       clid             = *(++col);
        unsigned           type             = *(++col);
        std::string        type_name        = *(++col);
        Database::DateTime start_time       = *(++col);
        std::string        server_trid      = *(++col);
        std::string        client_trid      = *(++col);
        unsigned           result           = *(++col);
        Database::ID       registrar_id     = *(++col);
        std::string        registrar_handle = registrars_table[registrar_id];
        std::string        object_handle    = *(++col);
        std::string        message          = *(++col);
        std::string        message_out      = *(++col);

        data_.push_back(
            new EPPActionImpl(
                aid,
                clid,
                type,
                type_name,
                start_time,
                server_trid,
                client_trid,
                result,
                registrar_id,
                registrar_handle,
                object_handle,
                message,
                message_out
            ));
      }
      /* checks if row number result load limit is active and set flag */ 
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }
  virtual void setPartialLoad(bool _partialLoad) {
    partialLoad = _partialLoad;
  }

  virtual void sort(EPPActionMemberType member, bool asc) {
    switch (member) {
      case MT_SVTRID:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareServerTransactionId(asc));
        break;
      case MT_CLTRID:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareClientTransactionId(asc));
        break;
      case MT_TYPE:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareType(asc));
        break;
      case MT_OBJECT_HANDLE:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareHandle(asc));
        break;
      case MT_REGISTRAR_HANDLE:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareRegistrarHandle(asc));
        break;
      case MT_TIME:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareStartTime(asc));
        break;
      case MT_RESULT:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareResult(asc));
        break;
    }
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
    /* dummy implementation */
  }

  virtual const char* getTempTableName() const {
    return ""; /* dummy implementation */
  }

};

class ManagerImpl : virtual public Manager {
  DB *db; ///< connection do db
  RegistrarListImpl rl;
  EPPActionListImpl eal;
  std::vector<EPPActionType> actionTypes;
public:
  ManagerImpl(DB *_db) :
    db(_db), rl(_db), eal(db) {
    
    if (!db->ExecSelect("SELECT * FROM enum_action")) {
      throw SQL_ERROR();
    }
    
    actionTypes.clear();
    for (unsigned i = 0; i < (unsigned)db->GetSelectRows(); i++) {
      EPPActionType action;
      action.id = STR_TO_ID(db->GetFieldValue(i, 0));
      action.name = db->GetFieldValue(i, 1);
      actionTypes.push_back(action);
    }
    db->FreeSelect();    
  }
  virtual RegistrarList *getList() {
    return &rl;
  }
  virtual EPPActionList *getEPPActionList() {
    return &eal;
  }
  virtual EPPActionList *createEPPActionList() {
    return new EPPActionListImpl(db);
  }
  
  virtual unsigned getEPPActionTypeCount() {
    return actionTypes.size();
  }
  virtual const EPPActionType& getEPPActionTypeByIdx(unsigned idx) const
      throw (NOT_FOUND) {
    if (idx >= actionTypes.size())
      throw NOT_FOUND();
    return actionTypes[idx];
  }
  virtual bool checkHandle(const std::string handle) const throw (SQL_ERROR) {
    if (!boost::regex_match(handle, boost::regex("[rR][eE][gG]-.*")))
      return false;
    std::stringstream sql;
    sql << "SELECT COUNT(*) FROM registrar " << "WHERE UPPER(handle)=UPPER('"
        << handle << "')";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    bool result = atoi(db->GetFieldValue(0, 0));
    db->FreeSelect();
    return result;
  }
  virtual void addRegistrar(const std::string& registrarHandle)
      throw (SQL_ERROR) {
    RegistrarListImpl rlist(db);
    rlist.setHandleFilter("REG-FRED_A");
    rlist.reload();
    if (rlist.size() < 1)
      return;
    RegistrarImpl *r = dynamic_cast<RegistrarImpl *>(rlist.get(0));
    if (!r)
      return;
    r->resetId();
    r->setHandle(registrarHandle);
    r->save();
  }
  virtual void addRegistrarAcl(
          const std::string &registrarHandle,
          const std::string &cert,
          const std::string &pass)
      throw (SQL_ERROR)
  {
      std::stringstream sql;
      sql << "INSERT INTO registraracl (registrarid, cert, password) "
          << "SELECT r.id, '" << cert << "','" << pass << "' FROM registrar r "
          << "WHERE r.handle='" << registrarHandle << "'";
      if (!db->ExecSQL(sql.str().c_str())) {
          throw SQL_ERROR();
      }
  }
  virtual Registrar *createRegistrar()
  {
      return dynamic_cast<RegistrarImpl *>(new RegistrarImpl(db));
  }

  virtual void addRegistrarZone(
          const std::string& registrarHandle,
          const std::string zone,
          const Database::Date &fromDate,
          const Database::Date &toDate) throw (SQL_ERROR) {
    std::string fromStr;
    std::string toStr;

    if (fromDate != Database::Date()) {
        fromStr = "'" + fromDate.to_string() + "'";
    } else {
        fromStr = "CURRENT_DATE";
    }
    if (toDate != Database::Date()) {
        toStr = "'" + toDate.to_string() + "'";
    } else {
        toStr = "NULL";
    }
    std::stringstream sql;
    sql << "INSERT INTO registrarinvoice (registrarid,zone,fromdate,lastdate) "
        << "SELECT r.id,z.id," << fromStr << "," << toStr << " FROM ("
        << "SELECT id FROM registrar WHERE handle='" << registrarHandle
        << "') r " << "JOIN (SELECT id FROM zone WHERE fqdn='" << zone
        << "') z ON (1=1) " << "LEFT JOIN registrarinvoice ri ON "
        << "(ri.registrarid=r.id AND ri.zone=z.id) " << "WHERE ri.id ISNULL";
    if (!db->ExecSQL(sql.str().c_str()))
      throw SQL_ERROR();
  }
}; // class ManagerImpl

Manager *Manager::create(DB *db) {
  TRACE("[CALL] Register::Registrar::Manager::create()");
  return new ManagerImpl(db);
}

}
; // namespace Registrar
}
; // namespace Register
