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

#include "nsset.h"
#include "object_impl.h"
#include "sql.h"
#include "dbsql.h"
#include "util.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>

#include <vector>

#define NSSET_REGEX_RESTRICTED "[nN][sS][sS][iI][dD]:[a-zA-Z0-9_:.-]{1,57}"
#define NSSET_REGEX "[a-zA-Z0-9_:.-]{1,63}"

namespace Register
{
  namespace NSSet
  {
    static boost::regex format(NSSET_REGEX);
    static boost::regex formatRestricted(NSSET_REGEX_RESTRICTED);

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
        if (!addr.empty()) addrList.push_back(addr);
      } 
    };
    class NSSetImpl : public ObjectImpl, public virtual NSSet
    {
      std::string handle;
      typedef std::vector<std::string> ContactListType;
      typedef std::vector<HostImpl> HostListType;
      ContactListType admins;
      HostListType hosts;
      unsigned checkLevel;
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
        const std::string& _roid,
        unsigned _checkLevel
      ) :
        ObjectImpl(
          _id, _crDate,_trDate,_upDate,_registrar,_registrarHandle,
          _createRegistrar,_createRegistrarHandle,
          _updateRegistrar,_updateRegistrarHandle,_authPw,_roid
        ), handle(_handle), checkLevel(_checkLevel)
      {
      }
      const std::string& getHandle() const
      {
        return handle;
      }
      virtual const unsigned  getCheckLevel() const
      {
        return checkLevel;
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
      std::string handle;
      std::string hostname;
      std::string ip;
      std::string admin;
     public:
      ListImpl(DB *_db) : ObjectListImpl(_db)  
      {}
      NSSet *getNSSet(unsigned idx) const
      {
        return dynamic_cast<NSSet *>(get(idx));
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
      void makeQuery(bool count, bool limit, std::stringstream& sql) const
      {
        std::stringstream from, where;
        sql.str("");
        if (!count) 
          sql << "INSERT INTO " << getTempTableName() << " ";
        sql << "SELECT " << (count ? "COUNT(" : "")
            << "DISTINCT n.id" << (count ? ") " : " ");
        from << "FROM nsset n ";
        where << "WHERE 1=1 ";
        SQL_ID_FILTER(where,"n.id",idFilter);
        if (registrarFilter || !registrarHandleFilter.empty() ||
            updateRegistrarFilter || !updateRegistrarHandleFilter.empty() ||
            TIME_FILTER_SET(updateIntervalFilter) ||
            TIME_FILTER_SET(trDateIntervalFilter)
           ) {
          from << ",object o ";
          where << "AND n.id=o.id ";
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
          where << "AND obr.id=n.id AND obr.type=2 ";       
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
        if (!admin.empty()) {
          from << ",nsset_contact_map ncm ";
          where << "AND n.id=ncm.nssetid ";
          // preprared for addition of admin ID filter
          if (!admin.empty()) {
            from << ",object_registry ncor ";
            where << "AND ncm.contactid=ncor.id AND ncor.type=1 ";            
            SQL_HANDLE_WILDCHECK_FILTER(
              where,"ncor.name",admin,wcheck,true
            );
          }
        }
        if (!hostname.empty()) {
          from << ",host h ";
          where << "AND n.id=h.nssetid ";
          SQL_HANDLE_FILTER(where,"h.fqdn",hostname);
        }
        if (!ip.empty()) {
          from << ",host_ipaddr_map him ";
          where << "AND n.id=him.nssetid ";
          SQL_HANDLE_FILTER(where,"host(him.ipaddr)",ip);
        }
        if (!count) where << "ORDER BY n.id ASC ";
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
        sql << "SELECT "
            << "obr.id,obr.name,"
            << "o.clid,"
            << "obr.crdate,o.trdate,o.update,"
            << "obr.crid,o.upid,o.authinfopw,obr.roid,n.checklevel "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "nsset n, object_registry obr, object o "
            << "WHERE tmp.id=n.id AND n.id=o.id AND obr.id=o.id "
            << "ORDER BY tmp.id ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          olist.push_back(
            new NSSetImpl(
              STR_TO_ID(db->GetFieldValue(i,0)), // nsset id
              db->GetFieldValue(i,1), // nsset handle
              STR_TO_ID(db->GetFieldValue(i,2)), // registrar id
              registrars[STR_TO_ID(db->GetFieldValue(i,2))], // reg. handle
              MAKE_TIME(i,3), // registrar crdate
              MAKE_TIME(i,4), // registrar trdate
              MAKE_TIME(i,5), // registrar update
              STR_TO_ID(db->GetFieldValue(i,6)), // crid 
              registrars[STR_TO_ID(db->GetFieldValue(i,6))], // crid handle
              STR_TO_ID(db->GetFieldValue(i,7)), // upid
              registrars[STR_TO_ID(db->GetFieldValue(i,7))], // upid handle
              db->GetFieldValue(i,8), // authinfo
              db->GetFieldValue(i,9), // roid
              atoi(db->GetFieldValue(i,10)) // checklevel
            )
          ); 
        }
        db->FreeSelect();
        // no need to proceed when nothing was loaded
        if (!getCount()) return;
        resetIDSequence();
        sql.str("");
        sql << "SELECT n.nssetid, cor.name "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "nsset_contact_map n, object_registry cor "
            << "WHERE tmp.id=n.nssetid AND n.contactid = cor.id "
            << "ORDER BY tmp.id, cor.id ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          NSSetImpl *ns = dynamic_cast<NSSetImpl *>(findIDSequence(
            STR_TO_ID(db->GetFieldValue(i,0))
          ));
          if (!ns) throw SQL_ERROR(); 
          ns->addAdminHandle(db->GetFieldValue(i,1));
        }
        db->FreeSelect();
        resetIDSequence();
        sql.str("");
        sql << "SELECT h.nssetid, h.fqdn, him.ipaddr "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "host h LEFT JOIN host_ipaddr_map him ON (h.id=him.hostid) "
            << "WHERE tmp.id=h.nssetid "
            << "ORDER BY tmp.id, h.id, him.id ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          NSSetImpl *ns = dynamic_cast<NSSetImpl *>(findIDSequence(
            STR_TO_ID(db->GetFieldValue(i,0))
          ));
          if (!ns) throw SQL_ERROR(); 
          HostImpl* h = ns->addHost(db->GetFieldValue(i,1));
          h->addAddr(db->GetFieldValue(i,2));
        }
        db->FreeSelect();
        ObjectListImpl::reload();
      }
      void clearFilter()
      {
        ObjectListImpl::clearFilter();
        handle = "";
        admin = "";
        ip = "";
        hostname = "";
      }
      virtual const char *getTempTableName() const
      {
        return "tmp_nsset_filter_result";
      }                  
    };
    class ManagerImpl : public virtual Manager
    {
      DB *db; ///< connection do db
      Zone::Manager *zm; ///< needed for hostname checking
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
            << "WHERE type=2 AND erDate ISNULL AND "
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
      ManagerImpl(DB *_db, Zone::Manager *_zm, bool _restrictedHandle) :
        db(_db), zm(_zm), restrictedHandle(_restrictedHandle)
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
        if (checkProtection(handle,2,"2 month")) 
          return CA_PROTECTED; 
        return CA_FREE;
      }            
      virtual unsigned checkHostname(
        const std::string& hostname, bool glue
      ) const
      {
        try {
          // test according to database limit
          if (hostname.length() > 255) return 1;
          // parse hostname (will throw exception on invalid)
          Zone::DomainName name;
          zm->parseDomainName(hostname,name);
          // if glue is specified, hostname must be under one of managed zones 
          if (glue && !zm->findZoneId(hostname)) return 1;
          // if glue is not specified, hostname must be under any valid zone
          if (!glue && !zm->checkTLD(name)) return 1;
          return 0;
        }
        catch (...) { return 1; } // INVALID_DOMAIN_NAME
      }
    };
    Manager *Manager::create(DB *db, Zone::Manager *zm, bool restrictedHandle)
    {
      return new ManagerImpl(db, zm, restrictedHandle);
    }    
  }
}
