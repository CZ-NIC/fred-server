#include "contact.h"
#include "dbsql.h"
#include "object_impl.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <vector>

namespace Register
{
  namespace Contact
  {
    class ContactImpl : public ObjectImpl, public virtual Contact
    {
      unsigned id;
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
     public:
      ContactImpl(
        unsigned _id,
        const std::string& _handle, 
        const std::string& _name,
        unsigned _registrar,
        const std::string& _registrarHandle,
        ptime _crDate,
        ptime _trDate,
        ptime _upDate,
        unsigned _createRegistrar,
        const std::string& _createRegistrarHandle,
        unsigned _updateRegistrar,      
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
        const std::string& _ssn         
      ) :
        ObjectImpl(_crDate,_trDate,_upDate,_registrar,_registrarHandle,
        _createRegistrar,_createRegistrarHandle,
        _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
        id(_id), handle(_handle), name(_name),
        organization(_organization), street1(_street1), street2(_street2),
        street3(_street3), province(_province), postalCode(_postalCode),
        city(_city), country(_country), telephone(_telephone), fax(_fax),
        email(_email), notifyEmail(_notifyEmail), ssn(_ssn)
      {
      }
      unsigned getId() const
      {
        return id;
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
    };
    class ListImpl : public virtual List
    {
      typedef std::vector<ContactImpl *> ContactList;
      ContactList clist;
      unsigned registrar;
      std::string registrarHandle;
      time_period crDateIntervalFilter;
      std::string handle;
      DB *db;
     public:
      ListImpl(DB *_db) : registrar(0),
      crDateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
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
      void setRegistrarFilter(unsigned registrarId)
      {
        registrar = registrarId;
      }
      void setRegistrarHandleFilter(const std::string& _registrarHandle)
      {
        registrarHandle = _registrarHandle;
      }
      void setCrDateIntervalFilter(time_period period)
      {
        crDateIntervalFilter = period;
      }    
      void setHandleFilter(const std::string& _handle)
      {
        handle = _handle;
      }
#define MAKE_TIME(ROW,COL)  \
 (ptime(db->IsNotNull(ROW,COL) ? \
 time_from_string(db->GetFieldValue(ROW,COL)) : not_a_date_time))     
      void reload() throw (SQL_ERROR)
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT c.id,c.handle,c.name,"
            << "r.id,r.handle,"
            << "c.crdate,c.trdate,c.update,"
            << "c.crid,creg.handle,c.upid,ureg.handle,c.authinfopw,c.roid,"
            << "c.organization,c.street1,c.street2,c.street3,"
            << "c.stateorprovince,"
            << "c.postalcode,c.city,c.country,c.telephone,c.fax,c.email,"
            << "c.notifyEmail,c.ssn "
            << "FROM registrar r, registrar creg, contact c "
            << "LEFT JOIN registrar ureg ON (c.upid=ureg.id) "
            << "WHERE c.clid=r.id AND c.crid=creg.id ";
        if (registrar)
          sql << "AND c.clid=" << registrar << " ";
        if (!registrarHandle.empty())
          sql << "AND r.handle='" << registrarHandle << "' ";     
        if (!handle.empty())
          sql << "AND c.handle='" << handle << "' ";     
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          clist.push_back(
            new ContactImpl(
              atoi(db->GetFieldValue(i,0)), // id
              db->GetFieldValue(i,1), // handle
              db->GetFieldValue(i,2), // name
              atoi(db->GetFieldValue(i,3)), // registrar id
              db->GetFieldValue(i,4), // registrar handle
              MAKE_TIME(i,5), // crdate
              MAKE_TIME(i,6), // trdate
              MAKE_TIME(i,7), // update
              atoi(db->GetFieldValue(i,8)), // crid
              db->GetFieldValue(i,9), // crid handle
              atoi(db->GetFieldValue(i,10)), // upid
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
              db->GetFieldValue(i,26) //ssn              
            )
          );
        }
        db->FreeSelect();
      }
      void clearFilter()
      {
        registrar = 0;
        registrarHandle = "";
        crDateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
        handle = "";
      }
    };
    class ManagerImpl : public virtual Manager
    {
      DB *db; ///< connection do db
      ListImpl clist;
     public:
      ManagerImpl(DB *_db) :
        db(_db), clist(_db)
      {}
      virtual List *getList()
      {
        return &clist;
      }      
    };
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }    
  }
}
