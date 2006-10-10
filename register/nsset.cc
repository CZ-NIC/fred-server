#include "nsset.h"
#include "dbsql.h"
#include "object_impl.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <vector>

namespace Register
{
  namespace NSSet
  {
    class NSSetImpl : public ObjectImpl, public virtual NSSet
    {
      unsigned id;
      std::string handle;
     public:
      NSSetImpl(
        unsigned _id, 
        const std::string& _handle, 
        unsigned registrarId,
        const std::string& registrarHandle,
        ptime crDate      
      ) : ObjectImpl(crDate,registrarId,registrarHandle),
          id(_id), handle(_handle)
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
    };
    class ListImpl : public virtual List
    {
      typedef std::vector<NSSetImpl *> NSSetList;
      NSSetList nlist;
      unsigned registrar;
      std::string registrarHandle;
      DB *db;
     public:
      ListImpl(DB *_db) : registrar(0), db(_db)
      {
      }
      ~ListImpl() 
      {
        clear();
      }
      void clear()
      {
        for (unsigned i=0; i<nlist.size(); i++) delete nlist[i];
        nlist.clear();
      }
      unsigned getCount() const
      {
        return nlist.size();
      }      
      NSSet *get(unsigned idx) const
      {
        return idx >= getCount() ? NULL : nlist[idx];
      }      
      void setRegistrarFilter(unsigned registrarId)
      {
        registrar = registrarId;
      }
      void setRegistrarHandleFilter(const std::string& _registrarHandle)
      {
        registrarHandle = _registrarHandle;
      }
      void reload() throw (SQL_ERROR)
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT n.id,n.handle,"
            << "r.id,r.handle,n.crdate "
            << "FROM nsset n, registrar r "
            << "WHERE n.clid=r.id ";
        if (registrar)
          sql << "AND n.clid=" << registrar << " ";
        if (!registrarHandle.empty())
          sql << "AND r.handle='" << registrarHandle << "' ";
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          nlist.push_back(
            new NSSetImpl(
              atoi(db->GetFieldValue(i,0)),
              db->GetFieldValue(i,1),
              atoi(db->GetFieldValue(i,2)),
              db->GetFieldValue(i,3),
              ptime(time_from_string(db->GetFieldValue(i,4)))
            )
          ); 
        }
      }
    };
    class ManagerImpl : public virtual Manager
    {
      DB *db; ///< connection do db
      ListImpl nlist;
     public:
      ManagerImpl(DB *_db) :
        db(_db), nlist(_db)
      {}
      virtual List *getList()
      {
        return &nlist;
      }      
    };
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }    
  }
}
