#include "nsset.h"
#include "dbsql.h"
#include "util.h"
#include "object_impl.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <vector>

namespace Register
{
  namespace NSSet
  {
    class HostImpl : public virtual Host
    {
      typedef std::vector<std::string> AddrListType;
      AddrListType addrList;
      std::string name;
     public:
      HostImpl(const std::string& _name) : name(_name)
      {}
      virtual const std::string getName() const
      {
        return name;
      }
      virtual unsigned getAddrCount() const
      {
        return addrList.size();
      }
      virtual std::string getAddrByIdx(unsigned idx) const
      {
        if (idx >= getAddrCount()) return "";
        else return addrList[idx];
      }
      void addAddr(const std::string& addr)
      {
        addrList.push_back(addr);
      } 
    };
    class NSSetImpl : public ObjectImpl, public virtual NSSet
    {
      unsigned id;
      std::string handle;
      typedef std::vector<std::string> ContactListType;
      typedef std::vector<HostImpl> HostListType;
      ContactListType admins;
      HostListType hosts;
     public:
      NSSetImpl(
        unsigned _id, 
        const std::string& _handle, 
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
      unsigned getAdminCount() const
      {
        return admins.size(); 
      }
      std::string getAdminByIdx(unsigned idx) const
      {
        if (idx>=admins.size()) return "";
        else return admins[idx];
      }
      unsigned getHostCount() const
      {
        return hosts.size();
      }
      const Host *getHostByIdx(unsigned idx) const
      {
        if (idx>=hosts.size()) return NULL;
        else return &hosts[idx];
      }
      void addAdminHandle(const std::string& admin)
      {
        admins.push_back(admin);
      }
      HostImpl *addHost(const std::string& name)
      {
        hosts.push_back(HostImpl(name));
        return &hosts.back();
      }
      bool hasId(unsigned _id)
      {
        return id == _id;
      }
    };
    class ListImpl : public virtual List
    {
      typedef std::vector<NSSetImpl *> NSSetList;
      NSSetList nlist;
      unsigned registrar;
      std::string registrarHandle;
      std::string handle;
      time_period crDateIntervalFilter;      
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
        sql << "SELECT n.id,n.handle,"
            << "r.id,r.handle,n.crdate "
            << "FROM nsset n, registrar r "
            << "WHERE n.clid=r.id ";
        if (registrar)
          sql << "AND n.clid=" << registrar << " ";
        if (!registrarHandle.empty())        
          sql << "AND r.handle='" << registrarHandle << "' ";
        if (!handle.empty())
          sql << "AND n.handle='" << handle << "' ";          
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          nlist.push_back(
            new NSSetImpl(
              atoi(db->GetFieldValue(i,0)),
              db->GetFieldValue(i,1),
              atoi(db->GetFieldValue(i,2)),
              db->GetFieldValue(i,3),
              ptime(time_from_string(db->GetFieldValue(i,4))),
              ptime(time_from_string(db->GetFieldValue(i,4))),
              ptime(time_from_string(db->GetFieldValue(i,4))),
              1,
              "",
              1,
              "",
              "auth",
              "roid"                                      
            )
          ); 
        }
        db->FreeSelect();
        sql.str("");
        sql << "SELECT n.nssetid, c.handle "
            << "FROM nsset_contact_map n, contact c "
            << "WHERE n.contactid = c.id";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          unsigned id = atoi(db->GetFieldValue(i,0));
          NSSetList::iterator n = find_if(
            nlist.begin(),nlist.end(),
            std::bind2nd(std::mem_fun(&NSSetImpl::hasId),id)
          );
          if (n != nlist.end())
            (*n)->addAdminHandle(db->GetFieldValue(i,1));
        }
        db->FreeSelect();
        sql.str("");
        sql << "SELECT h.nssetid, h.fqdn, h.ipaddr "
            << "FROM host h ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          unsigned id = atoi(db->GetFieldValue(i,0));
          NSSetList::iterator n = find_if(
            nlist.begin(),nlist.end(),
            std::bind2nd(std::mem_fun(&NSSetImpl::hasId),id)
          );
          if (n != nlist.end()) {
            HostImpl* h = (*n)->addHost(db->GetFieldValue(i,1));
            char buffer[100];
            for (unsigned i=0; 
                 i<(unsigned)get_array_length(db->GetFieldValue(i,2)); i++) {
              get_array_value(db->GetFieldValue(i,2),buffer,i);
              h->addAddr(buffer);
            }
          }
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
