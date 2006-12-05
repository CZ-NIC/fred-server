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
      /// must not be reference because of find_if algo (ref. to ref. problem)
      bool hasName(std::string _name) const
      {
        return name == _name;
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
      std::string handle;
      typedef std::vector<std::string> ContactListType;
      typedef std::vector<HostImpl> HostListType;
      ContactListType admins;
      HostListType hosts;
     public:
      NSSetImpl(
        TID _id, 
        const std::string& _handle, 
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
        const std::string& _roid         
      ) :
          ObjectImpl(_id, _crDate,_trDate,_upDate,_registrar,_registrarHandle,
         _createRegistrar,_createRegistrarHandle,
         _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
         handle(_handle)
      {
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
        HostListType::iterator i = find_if(
          hosts.begin(),hosts.end(),
          std::bind2nd(std::mem_fun_ref(&HostImpl::hasName),name)
        );
        if (i == hosts.end()) hosts.push_back(HostImpl(name));
        else return &(*i);
        return &hosts.back();
      }
      bool hasId(TID _id)
      {
        return id == _id;
      }
    };
    class ListImpl : public virtual List, public ObjectListImpl
    {
      typedef std::vector<NSSetImpl *> NSSetList;
      NSSetList nlist;
      std::string handle;
      std::string hostname;
      std::string ip;
      std::string admin;
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
      void setHandleFilter(const std::string& _handle)
      {
        handle = _handle;
      }
      void setHostNameFilter(const std::string& name)
      {
        hostname = name;
      }
      void setHostIPFilter(const std::string& _ip)
      {
        ip = _ip;
      }
      void setAdminFilter(const std::string& handle)
      {
        admin = handle;
      }
#define MAKE_TIME(ROW,COL)  \
 (ptime(db->IsNotNull(ROW,COL) ? \
 time_from_string(db->GetFieldValue(ROW,COL)) : not_a_date_time))   
      void reload() throw (SQL_ERROR)
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT DISTINCT "
            << "o.id,o.name,"
            << "r.id,r.handle, "
            << "o.crdate,o.trdate,o.update,"
            << "o.crid,creg.handle,o.upid,ureg.handle,o.authinfopw,o.roid "
            << "FROM registrar r, registrar creg, nsset n, object o "
            << "LEFT JOIN registrar ureg ON (n.upid=ureg.id) "
            << "LEFT JOIN nsset_contact_map ncm ON (ncm.nssetid=n.id) "
            << "LEFT JOIN object tco ON (ncm.contactid=tco.id) "
            << "LEFT JOIN host h ON (n.id=h.nssetid) "
            << "WHERE n.id=o.id AND o.clid=r.id AND o.crid=creg.id ";
        SQL_ID_FILTER(sql,"o.id",idFilter);
        SQL_ID_FILTER(sql,"r.id",registrarFilter);
        SQL_HANDLE_FILTER(sql,"r.handle",registrarHandleFilter);
        SQL_ID_FILTER(sql,"creg.id",createRegistrarFilter);
        SQL_HANDLE_FILTER(sql,"creg.handle",createRegistrarHandleFilter);
        SQL_ID_FILTER(sql,"ureg.id",updateRegistrarFilter);
        SQL_HANDLE_FILTER(sql,"ureg.handle",updateRegistrarHandleFilter);        
        SQL_DATE_FILTER(sql,"o.crDate",crDateIntervalFilter);
        SQL_DATE_FILTER(sql,"o.upDate",updateIntervalFilter);
        SQL_DATE_FILTER(sql,"o.trDate",trDateIntervalFilter);
        SQL_HANDLE_FILTER(sql,"o.name",handle);
        SQL_HANDLE_FILTER(sql,"tco.name",admin);        
        SQL_HANDLE_FILTER(sql,"h.fqdn",hostname);
        if (!ip.empty())
          sql << "AND STRPOS(ARRAY_TO_STRING(h.ipaddr,' '),'"
              << ip << "')!=0 ";          
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          nlist.push_back(
            new NSSetImpl(
              STR_TO_ID(db->GetFieldValue(i,0)), // nsset id
              db->GetFieldValue(i,1), // nsset handle
              STR_TO_ID(db->GetFieldValue(i,2)), // registrar id
              db->GetFieldValue(i,3), // registrar handle
              MAKE_TIME(i,4), // registrar crdate
              MAKE_TIME(i,5), // registrar trdate
              MAKE_TIME(i,6), // registrar update
              STR_TO_ID(db->GetFieldValue(i,7)), // crid 
              db->GetFieldValue(i,8), // crid handle
              STR_TO_ID(db->GetFieldValue(i,9)), // upid
              db->GetFieldValue(i,10), // upid handle
              db->GetFieldValue(i,11), // authinfo
              db->GetFieldValue(i,12) // roid
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
          TID id = STR_TO_ID(db->GetFieldValue(i,0));
          NSSetList::iterator n = find_if(
            nlist.begin(),nlist.end(),
            std::bind2nd(std::mem_fun(&NSSetImpl::hasId),id)
          );
          if (n != nlist.end())
            (*n)->addAdminHandle(db->GetFieldValue(i,1));
        }
        db->FreeSelect();
        sql.str("");
        sql << "SELECT h.nssetid, h.fqdn, him.ipaddr "
            << "FROM host h, host_ipaddr_map him "
            << "WHERE h.id=him.hostid";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          TID id = STR_TO_ID(db->GetFieldValue(i,0));
          NSSetList::iterator n = find_if(
            nlist.begin(),nlist.end(),
            std::bind2nd(std::mem_fun(&NSSetImpl::hasId),id)
          );
          if (n != nlist.end()) {
            HostImpl* h = (*n)->addHost(db->GetFieldValue(i,1));
            h->addAddr(db->GetFieldValue(i,2));
          }
        }
        db->FreeSelect();
      }
      void clearFilter()
      {
        ObjectListImpl::clear();
        handle = "";
        admin = "";
        ip = "";
        hostname = "";
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
