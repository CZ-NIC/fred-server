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

#include <sstream>
#include <memory>
#include <functional>
#include <algorithm>
#include "domain.h"
#include "blacklist.h"
#include "object_impl.h"
#include "sql.h"
#include "dbsql.h"
#include <boost/date_time/posix_time/time_parsers.hpp>

namespace Register
{
  namespace Domain
  {
    class DomainImpl : public ObjectImpl, public virtual Domain
    {
      struct AdminInfo 
      {
        TID id;
        std::string handle;
        AdminInfo(TID _id, const std::string& _handle)
          : id(_id), handle(_handle)
        {} 
      };
      typedef std::vector<AdminInfo> AdminInfoList;
      std::string fqdn;
      TID zone;
      TID nsset;
      std::string nssetHandle;
      TID registrant;
      std::string registrantHandle;
      std::string registrantName;
      AdminInfoList adminList;
      AdminInfoList tempList;
      date exDate;
      date valExDate;
      unsigned zoneStatus;
      ptime zoneStatusTime;
     public:
      DomainImpl(
        TID _id,
        const std::string& _fqdn,
        TID _zone,
        TID _nsset,
        const std::string& _nssetHandle,
        TID _registrant,
        const std::string& _registrantHandle,
        const std::string& _registrantName,
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
        date _exDate,
        date _valExDate,
        unsigned _zoneStatus,
        ptime _zoneStatusTime
      )
      : ObjectImpl(_id,_crDate,_trDate,_upDate,_registrar,_registrarHandle,
        _createRegistrar,_createRegistrarHandle,
        _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
        fqdn(_fqdn), zone(_zone), nsset(_nsset),
        nssetHandle(_nssetHandle), registrant(_registrant),
        registrantHandle(_registrantHandle), registrantName(_registrantName),
        exDate(_exDate), valExDate(_valExDate),
        zoneStatus(_zoneStatus), zoneStatusTime(_zoneStatusTime)
      {
      }
      virtual const std::string& getFQDN() const
      {
        return fqdn;
      }
      virtual TID getZoneId() const
      {
        return zone;
      }
      virtual const std::string& getNSSetHandle() const
      {
        return nssetHandle;
      }
      virtual TID getNSSetId() const
      {
        return nsset;
      }
      virtual void setNSSetId(TID nsset)
      {
        // check existance and set handle
        this->nsset = nsset;
        modified = true;
      }
      virtual const std::string& getRegistrantHandle() const
      {
        return registrantHandle;
      }
      virtual const std::string& getRegistrantName() const
      {
        return registrantName;
      }
      virtual TID getRegistrantId() const
      {
        return registrant;
      }
      virtual void setRegistrantId(TID registrant)
      {
        this->registrant = registrant;
      }
      virtual date getExpirationDate() const
      {
        return exDate; 
      }
      virtual date getValExDate() const
      {
        return valExDate; 
      }
      virtual unsigned getZoneStatus() const
      {
        return zoneStatus;
      }
      virtual ptime getZoneStatusTime() const
      {
        return zoneStatusTime;
      }
      virtual unsigned getAdminCount(unsigned role) const
      {
        return role == 1 ? adminList.size() : tempList.size();
      }
      virtual TID getAdminIdByIdx(unsigned idx, unsigned role) const
        throw (NOT_FOUND)
      {
        if (idx >= getAdminCount(role)) throw NOT_FOUND();
        return role == 1 ? adminList[idx].id : tempList[idx].id;
      }
      virtual const std::string& getAdminHandleByIdx(
	unsigned idx, unsigned role
      )  const throw (NOT_FOUND)
      {
        if (idx >= getAdminCount(role)) throw NOT_FOUND();
        return role == 1 ? adminList[idx].handle : tempList[idx].handle;
      }
      virtual void removeAdminId(TID id)
      {
        // find id in list and delete
      }
      virtual void insertAdminId(TID id)
      {
        // check existance of id
      }
      /// id lookup function
      bool hasId(TID id) const
      {
        return this->id == id;
      }
      /// add one admin handle - for domain intialization
      void addAdminHandle(TID id, const std::string& handle, unsigned role=1)
      {
        if (role == 1) adminList.push_back(AdminInfo(id,handle));
	    else tempList.push_back(AdminInfo(id,handle));
      } 
      /// add nsset handle - for domain intialization
      void addNSSetHandle(const std::string& handle)
      {
        nssetHandle = handle;
      }
      /// setting validation date - for domain initialization
      void setValExDate(date _valExDate)
      {
        valExDate = _valExDate;
      }
      virtual void insertStatus(TID id, ptime timeFrom, ptime timeTo)
      {
    	ObjectImpl::insertStatus(id, timeFrom, timeTo);
        // trigger setting ouzone status TODO: make define
    	if (id == 15) {
          zoneStatus = 0;
          zoneStatusTime = timeFrom;
    	}
      }
    };
    
    class CompareExdate
    {
      bool asc;
    public:
      CompareExdate(bool _asc) : asc(_asc) {}
      bool operator()(CommonObject *a, CommonObject *b) const
      {
        bool res =
          ((dynamic_cast<DomainImpl *>(a))->getExpirationDate() <=
          (dynamic_cast<DomainImpl *>(b))->getExpirationDate());
        return asc && res || !asc && !res; 
      }
    };

    class ListImpl : virtual public List, public ObjectListImpl
    {
      TID zoneFilter;
      TID registrantFilter;
      std::string registrantHandleFilter;
      TID nsset;
      std::string nssetHandle;
      TID admin;
      std::string adminHandle;
      TID temp;
      std::string tempHandle;
      TID contactFilter;
      std::string contactHandleFilter;
      std::string fqdn;
      time_period exDate;
      time_period valExDate;
      std::string techAdmin;
      std::string hostIP;
      unsigned zoneStatus;
     public:
      ListImpl(DB *_db) : ObjectListImpl(_db), 
        zoneFilter(0), registrantFilter(0),
        nsset(0), admin(0), temp(0), contactFilter(0),
        exDate(ptime(neg_infin),ptime(pos_infin)),
        valExDate(ptime(neg_infin),ptime(pos_infin)),
        zoneStatus(0)
      {
      }
      virtual Domain *getDomain(unsigned idx) const
      {
    	return dynamic_cast<DomainImpl *>(get(idx));
      }
      virtual void setZoneFilter(TID zoneId)
      {
        zoneFilter = zoneId;
      }
      virtual void setRegistrantFilter(TID registrantId)
      {
        registrantFilter = registrantId;
      }
      void setRegistrantHandleFilter(const std::string& _registrantHandle)
      {
        registrantHandleFilter = _registrantHandle;
      }
      virtual void setNSSetFilter(TID _nssetId)
      {
        nsset = _nssetId;
      }
      virtual void setNSSetHandleFilter(const std::string& _nssetHandle)
      {
        nssetHandle = _nssetHandle;
      }
      virtual void setAdminFilter(TID _adminId)
      {
        admin = _adminId;
      }
      virtual void setAdminHandleFilter(const std::string& _adminHandle)
      {
        adminHandle = _adminHandle;
      }
      virtual void setTempFilter(TID _tempId)
      {
        temp = _tempId;
      }
      virtual void setTempHandleFilter(const std::string& _tempHandle)
      {
        tempHandle = _tempHandle;
      }
      virtual void setContactFilter(TID contactId)
      {
        contactFilter = contactId;
      }
      virtual void setContactHandleFilter(const std::string& cHandle)
      {
        contactHandleFilter = cHandle;
      }
      virtual void setFQDNFilter(const std::string& _fqdn)
      {
        fqdn = _fqdn;
        boost::algorithm::to_lower(fqdn);
      }
      virtual void setExpirationDateFilter(time_period period)
      {
        exDate = period;
      }
      virtual void setValExDateFilter(time_period period)
      {
        valExDate = period;
      }
      virtual void setTechAdminHandleFilter(const std::string& handle)
      {
        techAdmin = handle;
      }
      virtual void setHostIPFilter(const std::string& ip)
      {
        hostIP = ip;
      }
      virtual void setZoneStatusFilter(unsigned status) 
      {
        zoneStatus = status;
      }
      void makeQuery(
        bool count, bool limit, std::stringstream& sql
      ) const
      {
        std::stringstream from, where;
        sql.str("");
        if (!count) 
          sql << "INSERT INTO " << getTempTableName() << " ";
        sql << "SELECT " << (count ? "COUNT(" : "")
            << "DISTINCT d.id" << (count ? ") " : " ");
        from << "FROM domain d ";
        where << "WHERE 1=1 ";
        SQL_ID_FILTER(where,"d.id",idFilter);
        SQL_DATE_FILTER(where,"d.exdate",exDate);
        SQL_ID_FILTER(where,"d.nsset",nsset);
        SQL_ID_FILTER(where,"d.registrant",registrantFilter);
        if (registrarFilter || !registrarHandleFilter.empty() ||
            updateRegistrarFilter || !updateRegistrarHandleFilter.empty() ||
            TIME_FILTER_SET(updateIntervalFilter) ||
            TIME_FILTER_SET(trDateIntervalFilter)
           ) {
          from << ",object o ";
          where << "AND d.id=o.id ";
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
            !fqdn.empty()) {
          from << ",object_registry obr ";
          where << "AND obr.id=d.id AND obr.type=3 ";       
          SQL_ID_FILTER(where,"obr.crid",createRegistrarFilter);
          SQL_DATE_FILTER(where,"obr.crdate",crDateIntervalFilter);
          SQL_HANDLE_WILDCHECK_FILTER(
            where,"obr.name",fqdn,wcheck,false
          );
          if (!createRegistrarHandleFilter.empty()) {
            from << ",registrar creg ";
            where << "AND obr.crid=creg.id ";          
            SQL_HANDLE_WILDCHECK_FILTER(
              where,"creg.handle",createRegistrarHandleFilter,wcheck,false
            );
          }
        }
        if (TIME_FILTER_SET(valExDate)) {
          from << ",enumval ev ";
          where << "AND d.id=ev.domainid ";
          SQL_DATE_FILTER(where,"ev.exdate",valExDate);
        }
        if (!registrantHandleFilter.empty()) {
          from << ",object_registry cor ";
          where << "AND d.registrant=cor.id AND cor.type=1 ";
          SQL_HANDLE_WILDCHECK_FILTER(
            where,"cor.name",registrantHandleFilter,wcheck,true
          );
        }
        if (admin || !adminHandle.empty()) {
          from << ",domain_contact_map dcm ";
          where << "AND d.id=dcm.domainid AND dcm.role=1 ";
          SQL_ID_FILTER(where,"dcm.contactid",admin);
          if (!adminHandle.empty()) {
            from << ",object_registry dcor ";
            where << "AND dcm.contactid=dcor.id AND dcor.type=1 ";
            SQL_HANDLE_WILDCHECK_FILTER(
              where,"dcor.name",adminHandle,wcheck,true
            );
          }
        }
        if (temp || !tempHandle.empty()) {
          from << ",domain_contact_map dcmt ";
          where << "AND d.id=dcmt.domainid AND dcmt.role=2 ";
          SQL_ID_FILTER(where,"dcmt.contactid",temp);
          if (!tempHandle.empty()) {
            from << ",object_registry dcort ";
            where << "AND dcmt.contactid=dcort.id AND dcort.type=1 ";
            SQL_HANDLE_WILDCHECK_FILTER(
              where,"dcort.name",tempHandle,wcheck,true
            );
          }
        }
        if (!nssetHandle.empty()) {
          from << ",object_registry nor ";
          where << "AND d.nsset=nor.id AND nor.type=2 ";         
          SQL_HANDLE_WILDCHECK_FILTER(
            where,"nor.name",nssetHandle,wcheck,true
          );
        }
        if (!techAdmin.empty()) {
          from << ",nsset_contact_map ncm, object_registry tcor ";
          where << "AND d.nsset=ncm.nssetid AND ncm.contactid=tcor.id "
                << "AND tcor.type=1 ";
          SQL_HANDLE_WILDCHECK_FILTER(
            where,"tcor.name",techAdmin,wcheck,true
          );
        }
        if (!hostIP.empty()) {
          from << ",host_ipaddr_map him ";
          where << "AND d.nsset=him.nssetid ";
          SQL_HANDLE_FILTER(where,"host(him.ipaddr)",hostIP);
        }
        if (add) where << "AND d.id NOT IN "
                       << "(SELECT id FROM " << getTempTableName() << ") ";
        if (!count) where << "ORDER BY d.id ASC ";
        if (limit) where << "LIMIT " << limitCount << " ";
        sql << from.rdbuf();
        sql << where.rdbuf();
      }
      virtual void reload() throw (SQL_ERROR)
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
        clear();
        fillTempTable(true);
        // load domain data
        sql.str("");
        sql << "SELECT "
            // domain, zone, nsset
            << "obr.id,obr.name,d.zone,d.nsset,'',"
            // registrant
            << "cor.id,cor.name,c.name,"
            // registrar
            << "o.clid,"
            // registration dates
            << "obr.crdate,o.trdate,o.update,"
            // creating and updating registrar
            << "obr.crid,o.upid,"
            // repository data
            << "o.authinfopw,obr.roid,"
            // expiration and validation dates (validation in seperate query)
            << "d.exdate,NULL "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "contact c, object_registry cor, "
            << "object_registry obr, "
            << "object o, "
            << "domain d "
            << "WHERE tmp.id=d.id AND d.id=o.id AND d.registrant=c.id "
            << "AND c.id=cor.id "
            << "AND obr.id=o.id "
            << "ORDER BY tmp.id ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          DomainImpl *d;
          d = new DomainImpl(
            STR_TO_ID(db->GetFieldValue(i,0)), // id
            db->GetFieldValue(i,1), // fqdn
            STR_TO_ID(db->GetFieldValue(i,2)), // zone
            STR_TO_ID(db->GetFieldValue(i,3)), // nsset id
            db->GetFieldValue(i,4), // nsset handle
            STR_TO_ID(db->GetFieldValue(i,5)), // registrant id
            db->GetFieldValue(i,6), // registrant handle
            db->GetFieldValue(i,7), // registrant name
            STR_TO_ID(db->GetFieldValue(i,8)), // registrar
            registrars[STR_TO_ID(db->GetFieldValue(i,8))], // registrar handle
            MAKE_TIME(i,9), // crdate
            MAKE_TIME(i,10), // trdate
            MAKE_TIME(i,11), // update
            STR_TO_ID(db->GetFieldValue(i,12)), // crid
            registrars[STR_TO_ID(db->GetFieldValue(i,12))], // crid handle
            STR_TO_ID(db->GetFieldValue(i,13)), // upid
            registrars[STR_TO_ID(db->GetFieldValue(i,13))], // upid handle
            db->GetFieldValue(i,14), // authinfo
            db->GetFieldValue(i,15), // roid
            MAKE_DATE(i,16), // exdate
            MAKE_DATE(i,17), // valexdate
            true, // zone status
            ptime() // zone status time 
          );
          olist.push_back(d);
        }
        db->FreeSelect();
        // no need to proceed when nothing was loaded
        if (!getCount()) return;
        // add admin contacts
        resetIDSequence();
        sql.str("");
        sql << "SELECT "
            << "tmp.id, obr.id, obr.name, dcm.role "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "domain_contact_map dcm, "
            << "object_registry obr "
            << "WHERE tmp.id=dcm.domainid and dcm.contactid=obr.id "
            << "ORDER BY tmp.id";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          DomainImpl *dom = dynamic_cast<DomainImpl *>(findIDSequence(
            STR_TO_ID(db->GetFieldValue(i,0))
          ));
          if (!dom) throw SQL_ERROR(); 
          dom->addAdminHandle(
            STR_TO_ID(db->GetFieldValue(i,1)),
            db->GetFieldValue(i,2),
            atoi(db->GetFieldValue(i,3))
          );
        }
        db->FreeSelect();
        // add nsset handles (instead of LEFT JOIN)
        resetIDSequence();
        sql.str("");
        sql << "SELECT "
            << "tmp.id, nor.name "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "domain d, object_registry nor "
            << "WHERE tmp.id=d.id AND d.nsset=nor.id "
            << "ORDER BY tmp.id";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          DomainImpl *dom = dynamic_cast<DomainImpl *>(findIDSequence(
            STR_TO_ID(db->GetFieldValue(i,0))
          ));
          if (!dom) throw SQL_ERROR(); 
          dom->addNSSetHandle(
            db->GetFieldValue(i,1)
          );
        }
        db->FreeSelect();
        // add validation (for enum domains)
        resetIDSequence();
        sql.str("");
        sql << "SELECT "
            << "tmp.id, ev.exdate "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "enumval ev "
            << "WHERE tmp.id=ev.domainid "
            << "ORDER BY tmp.id";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          DomainImpl *dom = dynamic_cast<DomainImpl *>(findIDSequence(
            STR_TO_ID(db->GetFieldValue(i,0))
          ));
          if (!dom) throw SQL_ERROR(); 
          dom->setValExDate(MAKE_DATE(i,1));
        }
        db->FreeSelect();
        ObjectListImpl::reload();
      }
      void clearFilter()
      {
        ObjectListImpl::clearFilter();
        registrantFilter = 0;
        registrantHandleFilter = "";
        nsset = 0;
        nssetHandle = "";
        admin = 0;
        adminHandle = "";
        fqdn = "";
        zoneFilter = 0;
        techAdmin = "";
        hostIP = "";
        zoneStatus = 0;
      }
      virtual void sort(MemberType member, bool asc)
      {
        switch (member) {
          case MT_EXDATE:
            stable_sort(olist.begin(),olist.end(),CompareExdate(asc));
            break;
        }
      }
      virtual const char *getTempTableName() const
      {
        return "tmp_domain_filter_result";
      }            
    };

    class ManagerImpl : virtual public Manager
    {
      DB *db; ///< connection do db
      Zone::Manager *zm; ///< zone management api
      std::auto_ptr<Blacklist> blacklist; ///< black list manager
     public:
      ManagerImpl(DB *_db, Zone::Manager *_zm) :
        db(_db), zm(_zm), blacklist(Blacklist::create(_db))
      {}
      CheckAvailType checkHandle(const std::string& fqdn) const 
      {
        Zone::DomainName domain; // parsed domain name
        try { zm->parseDomainName(fqdn,domain); }
        catch (Zone::INVALID_DOMAIN_NAME) { return CA_INVALID_HANDLE; }
        const Zone::Zone *z = zm->findZoneId(fqdn);
        // TLD domain allowed only if zone.fqdn='' is in zone list 
        if (!z && domain.size() == 1) return CA_INVALID_HANDLE;
        if (!z) return CA_BAD_ZONE;
        if (domain.size() > z->getMaxLevel()) return CA_BAD_LENGHT; 
        return CA_AVAILABLE;
      }
      /// interface method implementation  
      CheckAvailType checkAvail(
        const std::string& _fqdn,
        NameIdPair& conflictFqdn,
        bool lock
      ) const 
        throw (SQL_ERROR)
      {
        std::string fqdn = _fqdn;
        boost::algorithm::to_lower(fqdn);
        // clear output
        conflictFqdn.id = 0;
        conflictFqdn.name = "";
        CheckAvailType ret = checkHandle(fqdn);
        if (ret != CA_AVAILABLE) return ret;
        std::stringstream sql;
        // domain can be subdomain or parent domain of registred domain
        // there could be a lot of subdomains therefor LIMIT 1
        if (zm->findZoneId(fqdn)->isEnumZone())
          sql << "SELECT o.name, o.id FROM object_registry o "
              << "WHERE o.type=3 AND o.erdate ISNULL AND "
              << "(('" << fqdn << "' LIKE '%.'|| o.name) OR "
              << "(o.name LIKE '%.'||'" << fqdn << "') OR "
              << "o.name='" << fqdn << "') "
              << "LIMIT 1";
        else 
          sql << "SELECT o.name, o.id FROM object_registry o "
              << "WHERE o.type=3 AND o.erdate ISNULL AND "
              << "o.name='" << fqdn << "' "
              << "LIMIT 1";
        if (lock) sql << " FOR UPDATE ";        
        if (!db->ExecSelect(sql.str().c_str())) {
          db->FreeSelect();
          throw SQL_ERROR();
        }
        if (db->GetSelectRows() == 1) {
          conflictFqdn.name = db->GetFieldValue(0,0);
          conflictFqdn.id = STR_TO_ID(db->GetFieldValue(0,1));
          if (fqdn == conflictFqdn.name) ret = CA_REGISTRED;
          else if (fqdn.size() > conflictFqdn.name.size())
            ret = CA_PARENT_REGISTRED;
          else ret = CA_CHILD_REGISTRED;  
        }
        db->FreeSelect();
        if (ret != CA_AVAILABLE) return ret;
        if (blacklist->checkDomain(fqdn)) return CA_BLACKLIST;
        return ret;        
      }
      /// interface method implementation
      unsigned long getDomainCount(const std::string& zone) const
      {
        std::stringstream sql;
        unsigned long ret = 0;
        sql << "SELECT COUNT(*) "
            << "FROM object_registry o, domain d, zone z "
            << "WHERE d.id=o.id AND d.zone=z.id "
            << "AND z.fqdn='" << zone << "'";
        if (db->ExecSelect(sql.str().c_str()) && db->GetSelectRows() == 1)
          ret =  atol(db->GetFieldValue(0,0));
        db->FreeSelect();
        return ret;
      }
      /// interface method implementation
      unsigned long getEnumNumberCount() const
      {
        std::stringstream sql;
        unsigned long ret = 0;
        sql << "SELECT SUM(power(10,(33-char_length(name))/2)) "
            << "FROM object_registry o, domain d, zone z "
            << "WHERE d.id=o.id AND d.zone=z.id AND z.fqdn='0.2.4.e164.arpa'";
        if (db->ExecSelect(sql.str().c_str()) && db->GetSelectRows() == 1)
          ret =  atol(db->GetFieldValue(0,0));
        db->FreeSelect();
        return ret;
      }  
      virtual List *createList()
      {
        return new ListImpl(db);
      }
    };
    Manager *Manager::create(DB *db, Zone::Manager *zm)
    {
      return new ManagerImpl(db,zm);
    }
  };
};
