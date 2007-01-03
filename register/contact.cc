#include "contact.h"
#include "dbsql.h"
#include "object_impl.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>
#include <vector>

#define SET_SSNTYPE(t) ((t == 1 ? "RC" : (t == 2 ? "ICO" : "PASS")))

namespace Register
{
  namespace Contact
  {
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
      std::string vat;
      bool discloseName;
      bool discloseOrganization;
      bool discloseAddr;
      bool discloseEmail;
      bool discloseTelephone;
      bool discloseFax;
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
        bool _discloseFax  
      ) :
        ObjectImpl(_id, _crDate,_trDate,_upDate,_registrar,_registrarHandle,
        _createRegistrar,_createRegistrarHandle,
        _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
        handle(_handle), name(_name),
        organization(_organization), street1(_street1), street2(_street2),
        street3(_street3), province(_province), postalCode(_postalCode),
        city(_city), country(_country), telephone(_telephone), fax(_fax),
        email(_email), notifyEmail(_notifyEmail), ssn(_ssn),
        ssnType(SET_SSNTYPE(_ssnType)), vat(_vat),
        discloseName(_discloseName),discloseOrganization(_discloseOrganization),
        discloseAddr(_discloseAddr),discloseEmail(_discloseEmail),
        discloseTelephone(_discloseTelephone),discloseFax(_discloseFax)
        
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
    };
    class ListImpl : public virtual List, public ObjectListImpl
    {
      typedef std::vector<ContactImpl *> ContactList;
      ContactList clist;
      std::string handle;
      std::string name;
      std::string ident;
      std::string email;
      std::string org;
      std::string vat;
      DB *db;
     public:
      ListImpl(DB *_db) : ObjectListImpl(),
      db(_db)
      {
      }
      ~ListImpl() 
      {
        clear();
      }
      void clear()
      {
        for (unsigned i=0; i<clist.size(); i++) delete clist[i];
        clist.clear();
      }
      unsigned getCount() const
      {
        return clist.size();
      }      
      Contact *get(unsigned idx) const
      {
        return idx >= getCount() ? NULL : clist[idx];
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
#define MAKE_TIME(ROW,COL)  \
 (ptime(db->IsNotNull(ROW,COL) ? \
 time_from_string(db->GetFieldValue(ROW,COL)) : not_a_date_time))     
      void reload() throw (SQL_ERROR)
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT obr.id,obr.name,c.name,"
            << "r.id,r.handle,"
            << "obr.crdate,o.trdate,o.update,"
            << "creg.id,creg.handle,ureg.id,ureg.handle,o.authinfopw,obr.roid,"
            << "c.organization,c.street1,c.street2,c.street3,"
            << "c.stateorprovince,"
            << "c.postalcode,c.city,c.country,c.telephone,c.fax,c.email,"
            << "c.notifyEmail,c.ssn,c.ssntype,c.vat,"
            << "c.disclosename,c.discloseorganization,c.discloseaddress,"
            << "c.discloseemail,c.disclosetelephone,c.disclosefax "
            << "FROM registrar r, registrar creg, "
            << "contact c, object_registry obr, object o "
            << "LEFT JOIN registrar ureg ON (o.upid=ureg.id) "
            << "WHERE c.id=o.id AND o.clid=r.id "
            << "AND obr.crid=creg.id AND o.id=obr.id ";
        SQL_ID_FILTER(sql,"obr.id",idFilter);
        SQL_ID_FILTER(sql,"r.id",registrarFilter);
        SQL_HANDLE_FILTER(sql,"r.handle",registrarHandleFilter);
        SQL_ID_FILTER(sql,"creg.id",createRegistrarFilter);
        SQL_HANDLE_FILTER(sql,"creg.handle",createRegistrarHandleFilter);
        SQL_ID_FILTER(sql,"ureg.id",updateRegistrarFilter);
        SQL_HANDLE_FILTER(sql,"ureg.handle",updateRegistrarHandleFilter);        
        SQL_DATE_FILTER(sql,"obr.crDate",crDateIntervalFilter);
        SQL_DATE_FILTER(sql,"o.upDate",updateIntervalFilter);
        SQL_DATE_FILTER(sql,"o.trDate",trDateIntervalFilter);
        SQL_HANDLE_FILTER(sql,"obr.name",handle);
        SQL_HANDLE_FILTER(sql,"c.name",name);
        SQL_HANDLE_FILTER(sql,"c.ssn",ident);
        SQL_HANDLE_FILTER(sql,"c.email",email);
        SQL_HANDLE_FILTER(sql,"c.organization",org);
        SQL_HANDLE_FILTER(sql,"c.vat",vat);
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          clist.push_back(
            new ContactImpl(
              STR_TO_ID(db->GetFieldValue(i,0)), // id
              db->GetFieldValue(i,1), // handle
              db->GetFieldValue(i,2), // name
              STR_TO_ID(db->GetFieldValue(i,3)), // registrar id
              db->GetFieldValue(i,4), // registrar handle
              MAKE_TIME(i,5), // crdate
              MAKE_TIME(i,6), // trdate
              MAKE_TIME(i,7), // update
              STR_TO_ID(db->GetFieldValue(i,8)), // crid
              db->GetFieldValue(i,9), // crid handle
              STR_TO_ID(db->GetFieldValue(i,10)), // upid
              db->GetFieldValue(i,11), // upid handle
              db->GetFieldValue(i,12), // authinfo
              db->GetFieldValue(i,13), // roid
              db->GetFieldValue(i,14), //organization
              db->GetFieldValue(i,15), //street1
              db->GetFieldValue(i,16), //street2
              db->GetFieldValue(i,17), //street3
              db->GetFieldValue(i,18), //province
              db->GetFieldValue(i,19), //postalcode
              db->GetFieldValue(i,20), //city
              db->GetFieldValue(i,21), //country
              db->GetFieldValue(i,22), //telephone
              db->GetFieldValue(i,23), //fax
              db->GetFieldValue(i,24), //email
              db->GetFieldValue(i,25), //notifyEmail
              db->GetFieldValue(i,26), //ssn
              atoi(db->GetFieldValue(i,27)), //ssntype
              db->GetFieldValue(i,28), //vat
              (*db->GetFieldValue(i,29) == 't'), //discloseName
              (*db->GetFieldValue(i,30) == 't'), //discloseOrganization
              (*db->GetFieldValue(i,31) == 't'), //discloseAddr
              (*db->GetFieldValue(i,32) == 't'), //discloseEmail
              (*db->GetFieldValue(i,33) == 't'), //discloseTelephone
              (*db->GetFieldValue(i,34) == 't') //discloseFax
            )
          );
        }
        db->FreeSelect();
      }
      void clearFilter()
      {
        ObjectListImpl::clear();
        handle = "";
        name = "";
        ident = "";
        email = "";
        org = "";
        vat = "";
      }
    };
    class ManagerImpl : public virtual Manager
    {
      DB *db; ///< connection do db
      ListImpl clist;
      /// check if handle is in valid format
      /** Valid format is regexp 'CID:[[:alnum:]_.:]{1,36}' */
      bool checkHandleFormat(const std::string& handle) const
      {
        return boost::regex_match(handle,boost::regex(
           "[cC][iI][dD]:[a-zA-Z0-9_.:]{1,36}"
        ));
      }
      /// check if object is in database
      bool checkHandleRegistration(const std::string& handle) const
        throw (SQL_ERROR)
      {
        std::ostringstream sql;
        sql << "SELECT COUNT(*) FROM object_registry o, contact c "
            << "WHERE o.id=c.id AND o.name=upper('" << handle << "')";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        bool result = (atoi(db->GetFieldValue(0,0)));
        db->FreeSelect();
        return result;
      }
     public:
      ManagerImpl(DB *_db) :
        db(_db), clist(_db)
      {}
      virtual List *getList()
      {
        return &clist;
      }
      virtual CheckAvailType checkAvail(const std::string& handle) const
        throw (SQL_ERROR)
      {
        if (!checkHandleFormat(handle)) return CA_INVALID_HANDLE;
        if (!checkHandleRegistration(handle)) return CA_REGISTRED;
        return CA_FREE;
      }
    };
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }    
  }
}
