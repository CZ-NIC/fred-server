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
         const std::string& _roid
       ) :
         ObjectImpl(_crDate,_trDate,_upDate,_registrar,_registrarHandle,
         _createRegistrar,_createRegistrarHandle,
         _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
         id(_id), handle(_handle), name(_name)
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
      void reload() throw (SQL_ERROR)
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT c.id,c.handle,c.name,"
            << "r.id,r.handle,c.crdate "
            << "FROM contact c, registrar r "
            << "WHERE c.clid=r.id ";
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
              atoi(db->GetFieldValue(i,0)),
              db->GetFieldValue(i,1),
              db->GetFieldValue(i,2),
              atoi(db->GetFieldValue(i,3)),
              db->GetFieldValue(i,4),
              ptime(time_from_string(db->GetFieldValue(i,5))),
              ptime(time_from_string(db->GetFieldValue(i,5))),
              ptime(time_from_string(db->GetFieldValue(i,5))),
              1,
              "",
              1,
              "",
              "auth",
              "roid"              
            )
          ); 
        }
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
