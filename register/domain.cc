#include <sstream>
#include <memory>
#include <functional>
#include "domain.h"
#include "blacklist.h"
#include "dbsql.h"
#include "object_impl.h"
#include <boost/date_time/posix_time/time_parsers.hpp>

#define IS_NUMBER(x) (x >= '0' && x <= '9')
#define IS_LETTER(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define NUMLEN 9

namespace Register
{
  namespace Domain
  {
    class DomainImpl : ObjectImpl, public virtual Domain
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
      ptime exDate;
      ptime valExDate;
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
        ptime _exDate,
        ptime _valExDate
      )
      : ObjectImpl(_id,_crDate,_trDate,_upDate,_registrar,_registrarHandle,
        _createRegistrar,_createRegistrarHandle,
        _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
        fqdn(_fqdn), zone(_zone), nsset(_nsset),
        nssetHandle(_nssetHandle), registrant(_registrant),
        registrantHandle(_registrantHandle), registrantName(_registrantName),
        exDate(_exDate), valExDate(_valExDate)
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
      virtual ptime getExpirationDate() const
      {
        return exDate; 
      }
      virtual ptime getValExDate() const
      {
        return valExDate; 
      }
      virtual unsigned getAdminCount() const
      {
        return adminList.size();
      }
      virtual TID getAdminIdByIdx(unsigned idx) const
        throw (NOT_FOUND)
      {
        if (idx >= getAdminCount()) throw NOT_FOUND();
        return adminList[idx].id;
      }
      virtual const std::string& getAdminHandleByIdx(unsigned idx)  const 
        throw (NOT_FOUND)
      {
        if (idx >= getAdminCount()) throw NOT_FOUND();
        return adminList[idx].handle;
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
      void addAdminHandle(TID id, const std::string& handle)
      {
        adminList.push_back(AdminInfo(id,handle));
      } 
    };

    class ListImpl : virtual public List, public ObjectListImpl
    {
      typedef std::vector<DomainImpl *> DomainListType;
      DomainListType dlist;
      TID zoneFilter;
      TID registrantFilter;
      std::string registrantHandleFilter;
      TID nsset;
      std::string nssetHandle;
      TID admin;
      std::string adminHandle;
      std::string fqdn;
      time_period exDate;
      time_period valExDate;
      std::string techAdmin;
      std::string hostIP;
      DB *db;
     public:
      ListImpl(DB *_db) : ObjectListImpl(), 
        zoneFilter(0), registrantFilter(0),
        nsset(0), admin(0),
        exDate(ptime(neg_infin),ptime(pos_infin)),
        valExDate(ptime(neg_infin),ptime(pos_infin)),
        db(_db)
      {
      }
      virtual ~ListImpl()
      {
        clear();
      }
      void clear()
      {
        for (unsigned i=0; i<dlist.size(); i++) delete dlist[i];
        dlist.clear();
      }
      virtual unsigned getCount() const
      {
        return dlist.size();
      }
      virtual Domain *get(unsigned idx) const
      {
        if (idx >= dlist.size()) return NULL;
        return dlist[idx];
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
#define MAKE_TIME(ROW,COL)  \
 (ptime(db->IsNotNull(ROW,COL) ? \
 time_from_string(db->GetFieldValue(ROW,COL)) : not_a_date_time))
      virtual void reload()
      {
        clear();
        std::ostringstream sql;
        /// loading admin contact handles together with domains
        sql << "SELECT DISTINCT o.id,o.name,d.zone,no.id,no.name,"
            << "co.id,co.name,c.name,"
            << "r.id,r.handle,"
            << "o.crdate,o.trdate,o.update,"
            << "o.crid,creg.handle,o.upid,ureg.handle,o.authinfopw,o.roid,"
            << "d.exdate,ev.exdate,ac.handle "
            << "FROM contact c, object co, registrar r, registrar creg, "
            << "object o, domain d LEFT JOIN object no ON (d.nsset=no.id) "
            << "LEFT JOIN domain_contact_map adcm ON (d.id=adcm.domainid) "
            << "LEFT JOIN object aco ON (adcm.contactid=aco.id) "
            << "LEFT JOIN registrar ureg ON (o.upid=ureg.id) "
            << "LEFT JOIN enumval ev ON (d.id=ev.domainid) "
            << "LEFT JOIN nsset_contact_map ncm ON (ncm.nssetid=no.id) "
            << "LEFT JOIN object tco ON (ncm.contactid=tco.id) "
            << "LEFT JOIN host h ON (no.id=h.nssetid) "
            << "WHERE d.id=o.id AND d.registrant=c.id AND c.id=co.id "
            << "AND o.crid=creg.id AND o.clid=r.id ";
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
        SQL_ID_FILTER(sql,"co.id",registrantFilter);
        SQL_HANDLE_FILTER(sql,"co.name",registrantHandleFilter);
        SQL_DATE_FILTER(sql,"d.exdate",exDate);
        SQL_DATE_FILTER(sql,"ev.exdate",valExDate);
        SQL_ID_FILTER(sql,"no.id",nsset);
        SQL_HANDLE_FILTER(sql,"no.handle",nssetHandle);
        SQL_ID_FILTER(sql,"aco.id",admin);
        SQL_HANDLE_FILTER(sql,"aco.handle",adminHandle);
        SQL_HANDLE_FILTER(sql,"o.name",fqdn);
        SQL_HANDLE_FILTER(sql,"tco.name",techAdmin);
        if (!hostIP.empty())
          sql << "AND STRPOS(ARRAY_TO_STRING(h.ipaddr,' '),'"
              << hostIP << "')!=0 ";        
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          TID id = STR_TO_ID(db->GetFieldValue(i,0));
          // sql result has multiple lines for one domain
          // if domain has been already initialized only admin contacts
          // are filled
          DomainListType::const_iterator dom = find_if(
            dlist.begin(),dlist.end(),
            std::bind2nd(std::mem_fun(&DomainImpl::hasId),id)
          ); 
          DomainImpl *d;
          if (dom != dlist.end()) 
            d = *dom;
          else {
            d = new DomainImpl(
              id, // id
              db->GetFieldValue(i,1), // fqdn
              STR_TO_ID(db->GetFieldValue(i,2)), // zone
              STR_TO_ID(db->GetFieldValue(i,3)), // nsset id
              db->GetFieldValue(i,4), // nsset handle
              STR_TO_ID(db->GetFieldValue(i,5)), // registrant id
              db->GetFieldValue(i,6), // registrant handle
              db->GetFieldValue(i,7), // registrant name
              STR_TO_ID(db->GetFieldValue(i,8)), // registrar
              db->GetFieldValue(i,9), // registrar handle
              MAKE_TIME(i,10), // crdate
              MAKE_TIME(i,11), // trdate
              MAKE_TIME(i,12), // update
              STR_TO_ID(db->GetFieldValue(i,13)), // crid
              db->GetFieldValue(i,14), // crid handle
              STR_TO_ID(db->GetFieldValue(i,15)), // upid
              db->GetFieldValue(i,16), // upid handle
              db->GetFieldValue(i,17), // authinfo
              db->GetFieldValue(i,18), // roid
              MAKE_TIME(i,19), // exdate
              MAKE_TIME(i,20) // valexdate
            );
            dlist.push_back(d);
          }
          // add admin contact (temporary ignore id)
          if (db->IsNotNull(i,21)) 
            d->addAdminHandle(0,db->GetFieldValue(i,21));
        }
        db->FreeSelect();
      }
      void clearFilter()
      {
        ObjectListImpl::clear();
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
      }      
    };

    class ManagerImpl : virtual public Manager
    {
      DB *db; ///< connection do db
      Zone::Manager *zm; ///< zone management api
      std::auto_ptr<Blacklist> blacklist; ///< black list manager
      ListImpl dlist;
     public:
      ManagerImpl(DB *_db, Zone::Manager *_zm) :
        db(_db), zm(_zm), blacklist(Blacklist::create(_db)), dlist(_db)
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
        if (l-last <= NUMLEN) {
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
        std::string part;
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
      }
      /// interface method implementation  
      CheckAvailType checkAvail(const std::string& fqdn) const 
        throw (SQL_ERROR)
      {
        DomainName domain;
        try { parseDomainName(fqdn,domain); }
        catch (INVALID_DOMAIN_NAME) { return CA_INVALID_HANDLE; }
        if (!zm->findZoneId(fqdn)) return CA_BAD_ZONE;
        if (blacklist->checkDomain(fqdn)) return CA_BLACKLIST;
        std::stringstream sql;
        CheckAvailType ret = CA_AVAILABLE;
        // domain cannot be subdomain or parent domain of registred domain
        sql << "SELECT fqdn FROM domain WHERE "
            << "('" << fqdn << "' LIKE '%'||fqdn) OR "
            << "(fqdn LIKE '%'||'" << fqdn << "')";
        if (!db->ExecSelect(sql.str().c_str())) {
          db->FreeSelect();
          throw SQL_ERROR();
        }
        if (db->GetSelectRows() == 1) {
          std::string fqdnLoaded = db->GetFieldValue(0,0);
          if (fqdn == fqdnLoaded) ret = CA_REGISTRED;
          else ret = CA_PARENT_REGISTRED;  
        }
        db->FreeSelect();
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
      unsigned long getEnumDomainCount() const
      {
        std::stringstream sql;
        unsigned long ret = 0;
        sql << "SELECT COUNT(*) FROM domain WHERE fqdn LIKE '%e164.arpa'";
        if (db->ExecSelect(sql.str().c_str()) && db->GetSelectRows() == 1)
          ret =  atol(db->GetFieldValue(0,0));
        db->FreeSelect();
        return ret;
      }
      unsigned long getEnumNumberCount() const
      {
        std::stringstream sql;
        unsigned long ret = 0;
        sql << "SELECT SUM(power(10,(33-char_length(fqdn))/2)) "
            << "FROM domain WHERE fqdn LIKE '%e164.arpa'";
        if (db->ExecSelect(sql.str().c_str()) && db->GetSelectRows() == 1)
          ret =  atol(db->GetFieldValue(0,0));
        db->FreeSelect();
        return ret;
      }  
      virtual List *getList()
      {
        return &dlist;
      }
    };
    Manager *Manager::create(DB *db, Zone::Manager *zm)
    {
      return new ManagerImpl(db,zm);
    }
  };
};
