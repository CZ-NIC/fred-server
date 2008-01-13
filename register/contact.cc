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
#include "dbsql.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>
#include <vector>

#include "log.h"

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

namespace Register
{
  namespace Contact
  {
    static boost::regex format(CONTACT_REGEX);
    static boost::regex formatRestricted(CONTACT_REGEX_RESTRICTED);

    class ContactImpl : public ObjectImpl, public virtual Contact
    {
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
      ContactImpl(
        TID _id,
        const std::string& _handle, 
        const std::string& _name,
        TID _registrar,
        const std::string& _registrarHandle,
        ptime _crDate,
        ptime _trDate,
        ptime _upDate,
        TID _createRegistrar,
        const std::string& _createRegistrarHandle,
        TID _updateRegistrar,      
        const std::string& _updateRegistrarHandle,
        const std::string& _authPw,
        const std::string& _roid,
        const std::string& _organization,
        const std::string& _street1,
        const std::string& _street2,
        const std::string& _street3,
        const std::string& _province,
        const std::string& _postalCode,
        const std::string& _city,
        const std::string& _country,
        const std::string& _telephone,
        const std::string& _fax,
        const std::string& _email,
        const std::string& _notifyEmail,
        const std::string& _ssn,
        unsigned _ssnType,   
        const std::string& _vat,
        bool _discloseName,
        bool _discloseOrganization,
        bool _discloseAddr,
        bool _discloseEmail,
        bool _discloseTelephone,
        bool _discloseFax,
        bool _discloseVat,
        bool _discloseIdent,
        bool _discloseNotifyEmail
      ) :
        ObjectImpl(_id, _crDate,_trDate,_upDate,_registrar,_registrarHandle,
        _createRegistrar,_createRegistrarHandle,
        _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
        handle(_handle), name(_name),
        organization(_organization), street1(_street1), street2(_street2),
        street3(_street3), province(_province), postalCode(_postalCode),
        city(_city), country(_country), telephone(_telephone), fax(_fax),
        email(_email), notifyEmail(_notifyEmail), ssn(_ssn),
        ssnType(SET_SSNTYPE(_ssnType)), ssnTypeId(_ssnType), vat(_vat),
        discloseName(_discloseName),
        discloseOrganization(_discloseOrganization),
        discloseAddr(_discloseAddr),discloseEmail(_discloseEmail),
        discloseTelephone(_discloseTelephone),discloseFax(_discloseFax),
        discloseVat(_discloseVat),discloseIdent(_discloseIdent),
        discloseNotifyEmail(_discloseNotifyEmail)
      {
      }
      const std::string& getHandle() const
      {
        return handle;
      }
      const std::string& getName() const
      {
        return name;
      }
      const std::string& getOrganization() const
      {
        return organization;
      }
      const std::string& getStreet1() const
      {
        return street1;
      }
      const std::string& getStreet2() const
      {
        return street2;
      }
      const std::string& getStreet3() const
      {
        return street3;
      }
      const std::string& getProvince() const
      {
        return province;
      }
      const std::string& getPostalCode() const
      {
        return postalCode;
      }
      const std::string& getCity() const
      {
        return city;
      }
      const std::string& getCountry() const
      {
        return country;
      }     
      const std::string& getTelephone() const
      {
        return telephone;
      }
      const std::string& getFax() const
      {
        return fax;
      }
      const std::string& getEmail() const
      {
        return email;
      }
      const std::string& getNotifyEmail() const
      {
        return notifyEmail;
      }
      const std::string& getSSN() const
      {
        return ssn;
      }
      virtual const std::string& getSSNType() const
      {
        return ssnType;
      }
      virtual unsigned getSSNTypeId() const
      {
        return ssnTypeId;
      }
      virtual const std::string& getVAT() const
      {
        return vat;
      }
      virtual bool getDiscloseName() const
      {
        return discloseName;
      }
      virtual bool getDiscloseOrganization() const
      {
        return discloseOrganization;
      }
      virtual bool getDiscloseAddr() const
      {
        return discloseAddr;
      }
      virtual bool getDiscloseEmail() const
      {
        return discloseEmail;
      }
      virtual bool getDiscloseTelephone() const
      {
        return discloseTelephone;
      }
      virtual bool getDiscloseFax() const
      {
        return discloseFax;
      }
      virtual bool getDiscloseVat() const
      {
        return discloseVat;
      }
      virtual bool getDiscloseIdent() const
      {
        return discloseIdent;
      }
      virtual bool getDiscloseNotifyEmail() const
      {
        return discloseNotifyEmail;
      }
    };
    class ListImpl : public virtual List, public ObjectListImpl
    {
      std::string handle;
      std::string name;
      std::string ident;
      std::string email;
      std::string org;
      std::string vat;
     public:
      ListImpl(DB *_db) : ObjectListImpl(_db)
      {}
      Contact *getContact(unsigned idx) const
      {
        return dynamic_cast<ContactImpl *>(get(idx));
      }      
      void setHandleFilter(const std::string& _handle)
      {
        handle = _handle;
      }
      void setNameFilter(const std::string& _name)
      {
        name = _name;
      }
      void setIdentFilter(const std::string& _ident)
      {
        ident = _ident;
      }
      void setEmailFilter(const std::string& _email)
      {
        email = _email;
      }
      void setOrganizationFilter(const std::string& _org)
      {
        org = _org;
      }
      void setVATFilter(const std::string& _vat)
      {
        vat = _vat;
      }
      void makeQuery(bool count, bool limit, std::stringstream& sql) const
      {
        std::stringstream from, where;
        sql.str("");
        if (!count) 
          sql << "INSERT INTO " << getTempTableName() << " ";
        sql << "SELECT " << (count ? "COUNT(" : "")
            << "DISTINCT c.id" << (count ? ") " : " ");
        from << "FROM contact c ";
        where << "WHERE 1=1 ";
        SQL_ID_FILTER(where,"c.id",idFilter);
        SQL_HANDLE_FILTER(where,"c.name",name);
        SQL_HANDLE_FILTER(where,"c.ssn",ident);
        SQL_HANDLE_FILTER(where,"c.email",email);
        SQL_HANDLE_FILTER(where,"c.organization",org);
        SQL_HANDLE_FILTER(where,"c.vat",vat);
        if (registrarFilter || !registrarHandleFilter.empty() ||
            updateRegistrarFilter || !updateRegistrarHandleFilter.empty() ||
            TIME_FILTER_SET(updateIntervalFilter) ||
            TIME_FILTER_SET(trDateIntervalFilter)
           ) {
          from << ",object o ";
          where << "AND c.id=o.id ";
          SQL_ID_FILTER(where,"o.clid",registrarFilter);
          SQL_ID_FILTER(where,"o.upid",updateRegistrarFilter);
          SQL_DATE_FILTER(where,"o.upDate",updateIntervalFilter);
          SQL_DATE_FILTER(where,"o.trDate",trDateIntervalFilter);
          if (!registrarHandleFilter.empty()) {
            from << ",registrar reg ";
            where << "AND o.clid=reg.id ";
            SQL_HANDLE_WILDCHECK_FILTER(
              where,"reg.handle",registrarHandleFilter,wcheck,false
            );
          }
          if (!updateRegistrarHandleFilter.empty()) {
            from << ",registrar ureg ";
            where << "AND o.upid=ureg.id ";          
            SQL_HANDLE_WILDCHECK_FILTER(
              where,"ureg.handle",updateRegistrarHandleFilter,wcheck,false
            );
          }
        }
        if (createRegistrarFilter || !createRegistrarHandleFilter.empty() ||
            TIME_FILTER_SET(crDateIntervalFilter) ||
            !handle.empty()) {
          from << ",object_registry obr ";
          where << "AND obr.id=c.id AND obr.type=1 ";       
          SQL_ID_FILTER(where,"obr.crid",createRegistrarFilter);
          SQL_DATE_FILTER(where,"obr.crdate",crDateIntervalFilter);
          SQL_HANDLE_WILDCHECK_FILTER(
            where,"obr.name",handle,wcheck,true
          );
          if (!createRegistrarHandleFilter.empty()) {
            from << ",registrar creg ";
            where << "AND obr.crid=creg.id ";          
            SQL_HANDLE_WILDCHECK_FILTER(
              where,"creg.handle",createRegistrarHandleFilter,wcheck,false
            );
          }
        }
        if (!count) where << "ORDER BY c.id ASC ";
        if (limit) where << "LIMIT " << limitCount << " ";
        sql << from.rdbuf();
        sql << where.rdbuf();
      }
      void reload() throw (SQL_ERROR)
      {
        std::map<TID,std::string> registrars;
        std::ostringstream sql;
        sql << "SELECT id, handle FROM registrar";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          registrars[STR_TO_ID(db->GetFieldValue(i,0))] = 
            db->GetFieldValue(i,1);
        }
        db->FreeSelect();
        sql.str("");
        clear();
        fillTempTable(true);
        sql << "SELECT obr.id,obr.name,c.name,"
            << "o.clid,"
            << "obr.crdate,o.trdate,o.update,"
            << "obr.crid,o.upid,o.authinfopw,obr.roid,"
            << "c.organization,c.street1,c.street2,c.street3,"
            << "c.stateorprovince,"
            << "c.postalcode,c.city,c.country,c.telephone,c.fax,c.email,"
            << "c.notifyEmail,c.ssn,c.ssntype,c.vat,"
            << "c.disclosename,c.discloseorganization,c.discloseaddress,"
            << "c.discloseemail,c.disclosetelephone,c.disclosefax, "
            << "c.disclosevat,c.discloseident,c.disclosenotifyemail "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "contact c, object_registry obr, object o "
            << "WHERE tmp.id=c.id AND c.id=o.id AND o.id=obr.id "
            << "ORDER BY tmp.id ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          olist.push_back(
            new ContactImpl(
              STR_TO_ID(db->GetFieldValue(i,0)), // id
              db->GetFieldValue(i,1), // handle
              db->GetFieldValue(i,2), // name
              STR_TO_ID(db->GetFieldValue(i,3)), // registrar id
              registrars[STR_TO_ID(db->GetFieldValue(i,3))], // reg. handle
              MAKE_TIME(i,4), // crdate
              MAKE_TIME(i,5), // trdate
              MAKE_TIME(i,6), // update
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
            )
          );
        }
        db->FreeSelect();
        ObjectListImpl::reload();
      }
      void clearFilter()
      {
        ObjectListImpl::clearFilter();
        handle = "";
        name = "";
        ident = "";
        email = "";
        org = "";
        vat = "";
      }
      virtual const char *getTempTableName() const
      {
        return "tmp_contact_filter_result";
      }                  
    };
    class ManagerImpl : public virtual Manager
    {
      DB *db; ///< connection do db
      bool restrictedHandle; ///< format of handle is more restrictive
      /// check if handle is in valid format
      bool checkHandleFormat(const std::string& handle) const
      {
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
      bool checkHandleRegistration(
        const std::string& handle,
        NameIdPair& conflict,
        bool lock
      ) const throw (SQL_ERROR)
      {
        std::ostringstream sql;
        sql << "SELECT id,name FROM object_registry "
            << "WHERE type=1 AND erDate ISNULL AND "
            << "UPPER(name)=UPPER('" << handle << "')";
        if (lock) sql << " FOR UPDATE ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        bool result = db->GetSelectRows() >= 1;
        conflict.id = result ? STR_TO_ID(db->GetFieldValue(0,0)) : 0;
        conflict.name = result ? db->GetFieldValue(0,1) : "";
        db->FreeSelect();
        return result;
      }
      /// check if object handle is in protection period (true=protected)
      bool checkProtection(
        const std::string& name, unsigned type,
        const std::string& monthPeriodSQL
      ) const throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "SELECT COALESCE("
            << "MAX(erdate) + INTERVAL '" << monthPeriodSQL << "'"
            << " > CURRENT_DATE, false) "
            << "FROM object_registry "
            << "WHERE NOT(erdate ISNULL) " 
            << "AND type=" << type << " " 
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
        db(_db), restrictedHandle(_restrictedHandle)
      {}
      virtual List *createList()
      {
        return new ListImpl(db);
      }
      virtual CheckAvailType checkAvail(
        const std::string& handle, NameIdPair& conflict, bool lock
      ) const throw (SQL_ERROR)
      {
        conflict.id = 0;
        conflict.name = "";
        if (!checkHandleFormat(handle)) return CA_INVALID_HANDLE;
        if (checkHandleRegistration(handle, conflict, lock)) 
          return CA_REGISTRED;
        if (checkProtection(handle,1,"2 month")) 
          return CA_PROTECTED;
        return CA_FREE;
      }
    };
    Manager *Manager::create(DB *db, bool restrictedHandle)
    {
      return new ManagerImpl(db, restrictedHandle);
    }    
  }
}
