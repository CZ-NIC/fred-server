#include <sstream>
#include <memory>
#include <functional>
#include "domain.h"
#include "blacklist.h"
#include "object_impl.h"
#include "sql.h"
#include "dbsql.h"
#include <boost/date_time/posix_time/time_parsers.hpp>

#define IS_NUMBER(x) (x >= '0' && x <= '9')
#define IS_LETTER(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))

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
      std::string fqdn;
      time_period exDate;
      time_period valExDate;
      std::string techAdmin;
      std::string hostIP;
      unsigned zoneStatus;
     public:
      ListImpl(DB *_db) : ObjectListImpl(_db), 
        zoneFilter(0), registrantFilter(0),
        nsset(0), admin(0), temp(0),
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
      virtual void setFQDNFilter(const std::string& _fqdn)
      {
        fqdn = _fqdn;
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
      void makeQuery(bool count, bool limit, std::stringstream& sql) const
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
            // expiration and validation dates
            << "d.exdate,ev.exdate, "
            // zone generation status and status change time
            << "gdh.status, gdh.chdate "
            << "FROM "
            << getTempTableName() << " tmp, "
            << "contact c, object_registry cor, "
            << "object_registry obr, "
            << "object o, "
            << "domain d "
            << "LEFT JOIN enumval ev ON (d.id=ev.domainid) "
            << "LEFT JOIN genzone_domain_history gdh "
            << "ON (d.id=gdh.domain_id and gdh.last='t') "
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
            atoi(db->GetFieldValue(i,18)), // zone status
            MAKE_TIME(i,19) // zone status time 
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
      /// interface method implementation  
      std::string makeEnumDomain(const std::string& number)
        const throw (NOT_A_NUMBER)
      {
        std::string result;
        unsigned l = number.size();
        if (!l) throw NOT_A_NUMBER();
        // where to stop in backward processing
        unsigned last = 0;
        if (!number.compare(0,2,"00")) last = 2;
        else if (number[0] == '+') last = 1;
        // process string
        for (unsigned i=l-1; i>last; i--) {
          if (!IS_NUMBER(number[i])) throw NOT_A_NUMBER();
          result += number[i];
          result += '.';
        }
        if (!IS_NUMBER(number[last])) throw NOT_A_NUMBER();
        result += number[last];
        // append default country code if short
        if (!last) {
          result += '.';
          result += zm->getDefaultEnumSuffix();
        }
        // append enum domain zone
        result += '.';
        result += zm->getEnumZoneString(); 
        return result;
      }
      /// interface method implementation  
      void parseDomainName(const std::string& fqdn, DomainName& domain) 
       const throw (INVALID_DOMAIN_NAME)
      {
        std::string part; // one part(label) of fqdn  
        for (unsigned i=0; i<fqdn.size(); i++) {
          if (part.empty()) {
            // first character of every label has to be letter or digit
            if (!IS_NUMBER(fqdn[i]) && !IS_LETTER(fqdn[i]))
              throw INVALID_DOMAIN_NAME();
          }
          else {
            // dot '.' is a separator of labels, store and clear part
            if (fqdn[i] == '.') {
              domain.push_back(part);
              part.clear();
              continue;
            } 
            else {
              if (fqdn[i] == '-') {
                // dash '-' is acceptable only if last character wasn't dash
                if (part[part.length()-1] == '-') throw INVALID_DOMAIN_NAME();
              } 
              else {
                // other character could be only number or letter
                if (!IS_NUMBER(fqdn[i]) && !IS_LETTER(fqdn[i]))
                  throw INVALID_DOMAIN_NAME();
              }
            }
          }
          // add character into part
          part += fqdn[i];
        }
        // last part cannot be empty
        if (part.empty()) throw INVALID_DOMAIN_NAME();
        // append last part
        domain.push_back(part);
        // enum domains has special rules
        if (checkEnumDomainSuffix(fqdn) && !checkEnumDomainName(domain))
          throw INVALID_DOMAIN_NAME();
      }
      /// interface method implementation  
      CheckAvailType checkAvail(
        const std::string& fqdn,
        NameIdPair& conflictFqdn
      ) const 
        throw (SQL_ERROR)
      {
        // clear output
        conflictFqdn.id = 0;
        conflictFqdn.name = "";
        DomainName domain; // parsed domain name
        try { parseDomainName(fqdn,domain); }
        catch (INVALID_DOMAIN_NAME) { return CA_INVALID_HANDLE; }
        const Zone::Zone *z = zm->findZoneId(fqdn);
        // TLD domain allowed only if zone.fqdn='' is in zone list 
        if (!z && domain.size() == 1) return CA_INVALID_HANDLE;
        if (!z) return CA_BAD_ZONE;
        if (domain.size() > z->getMaxLevel()) return CA_BAD_LENGHT; 
        std::stringstream sql;
        CheckAvailType ret = CA_AVAILABLE;
        // domain can be subdomain or parent domain of registred domain
        // there could be a lot of subdomains therefor LIMIT 1
        if (z->isEnumZone())
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
      bool checkEnumDomainName(DomainName& domain) const
      {
        // must have suffix e164.org
        if (domain.size()<=2) return false;
        // every part of domain name except of suffix must be one digit
        for (unsigned i=0; i<domain.size()-2; i++)
          if (domain[i].length() != 1 || !IS_NUMBER(domain[i][0]))
            return false;
        return true;
      }
      /// check if suffix is e164.arpa
      bool checkEnumDomainSuffix(const std::string& fqdn) const
      {
        const std::string& esuf = zm->getEnumZoneString();
        // check if substring 'esuf' found from right
        // is last substring in fqdn
        std::string::size_type i = fqdn.rfind(esuf);
        return  i != std::string::npos && i + esuf.size() == fqdn.size();
      }
      /// interface method implementation
      unsigned long getEnumDomainCount() const
      {
        std::stringstream sql;
        unsigned long ret = 0;
        sql << "SELECT COUNT(*) FROM object_registry "
            << "WHERE type=3 AND erdate ISNULL AND name LIKE '%e164.arpa'";
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
            << "FROM object_registry "
            << "WHERE type=3 AND erdate ISNULL AND name LIKE '%e164.arpa'";
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
