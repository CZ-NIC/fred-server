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
        unsigned id;
        std::string handle;
        AdminInfo(unsigned _id, const std::string& _handle)
          : id(_id), handle(_handle)
        {} 
      };
      typedef std::vector<AdminInfo> AdminInfoList;
      unsigned id;
      std::string fqdn;
      unsigned zone;
      unsigned nsset;
      std::string nssetHandle;
      unsigned registrant;
      std::string registrantHandle;
      std::string registrantName;
      AdminInfoList adminList;
      ptime exDate;
      ptime valExDate;
     public:
      DomainImpl(
        unsigned _id,
        const std::string& _fqdn,
        unsigned _zone,
        unsigned _nsset,
        const std::string& _nssetHandle,
        unsigned _registrant,
        const std::string& _registrantHandle,
        const std::string& _registrantName,
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
        ptime _exDate,
        ptime _valExDate
      )
      : ObjectImpl(_crDate,_trDate,_upDate,_registrar,_registrarHandle,
        _createRegistrar,_createRegistrarHandle,
        _updateRegistrar,_updateRegistrarHandle,_authPw,_roid),
        id(_id), fqdn(_fqdn), zone(_zone), nsset(_nsset),
        nssetHandle(_nssetHandle), registrant(_registrant),
        registrantHandle(_registrantHandle), registrantName(_registrantName),
        exDate(_exDate), valExDate(_valExDate)
      {
      }
      virtual unsigned getId() const
      {
        return id;
      }
      virtual const std::string& getFQDN() const
      {
        return fqdn;
      }
      virtual unsigned getZoneId() const
      {
        return zone;
      }
      virtual const std::string& getNSSetHandle() const
      {
        return nssetHandle;
      }
      virtual unsigned getNSSetId() const
      {
        return nsset;
      }
      virtual void setNSSetId(unsigned nsset)
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
      virtual unsigned getRegistrantId() const
      {
        return registrant;
      }
      virtual void setRegistrantId(unsigned registrant)
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
      virtual unsigned getAdminIdByIdx(unsigned idx) const
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
      virtual void removeAdminId(unsigned id)
      {
        // find id in list and delete
      }
      virtual void insertAdminId(unsigned id)
      {
        // check existance of id
      }
      /// id lookup function
      bool hasId(unsigned id) const
      {
        return this->id == id;
      }
      /// add one admin handle - for domain intialization
      void addAdminHandle(unsigned id, const std::string& handle)
      {
        adminList.push_back(AdminInfo(id,handle));
      } 
    };

    class ListImpl : public virtual List
    {
      typedef std::vector<DomainImpl *> DomainListType;
      DomainListType dlist;
      unsigned zoneFilter;
      unsigned registrarFilter;
      std::string registrarHandleFilter;
      unsigned registrantFilter;
      std::string registrantHandleFilter;
      time_period crDateIntervalFilter;
      unsigned nsset;
      std::string nssetHandle;
      unsigned admin;
      std::string adminHandle;
      std::string fqdn;
      DB *db;
     public:
      ListImpl(DB *_db) : 
        zoneFilter(0), registrarFilter(0), registrantFilter(0),
        crDateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
        nsset(0), admin(0), db(_db)
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
      virtual void setZoneFilter(unsigned zoneId)
      {
        zoneFilter = zoneId;
      }
      virtual void setRegistrarFilter(unsigned registrarId)
      {
        registrarFilter = registrarId;
      }
      void setRegistrarHandleFilter(const std::string& _registrarHandle)
      {
        registrarHandleFilter = _registrarHandle;
      }      
      virtual void setRegistrantFilter(unsigned registrantId)
      {
        registrantFilter = registrantId;
      }
      void setRegistrantHandleFilter(const std::string& _registrantHandle)
      {
        registrantHandleFilter = _registrantHandle;
      }
      void setCrDateIntervalFilter(time_period period)
      {
        crDateIntervalFilter = period;
      }
      virtual void setNSSetFilter(unsigned _nssetId)
      {
        nsset = _nssetId;
      }
      virtual void setNSSetHandleFilter(const std::string& _nssetHandle)
      {
        nssetHandle = _nssetHandle;
      }
      virtual void setAdminFilter(unsigned _adminId)
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
#define MAKE_TIME(ROW,COL)  \
 (ptime(db->IsNotNull(ROW,COL) ? \
 time_from_string(db->GetFieldValue(ROW,COL)) : not_a_date_time))
      virtual void reload()
      {
        clear();
        std::ostringstream sql;
        /// loading admin contact handles together with domains
        sql << "SELECT DISTINCT d.id,d.fqdn,d.zone,n.id,n.handle,"
            << "c.id,c.handle,c.name,"
            << "r.id,r.handle,"
            << "d.crdate,d.trdate,d.update,"
            << "d.crid,creg.handle,d.upid,ureg.handle,d.authinfopw,d.roid,"
            << "d.exdate,ev.exdate,ac.handle "
            << "FROM contact c, registrar r, registrar creg, "
            << "domain d LEFT JOIN nsset n ON (d.nsset=n.id) "
            << "LEFT JOIN domain_contact_map adcm ON (d.id=adcm.domainid) "
            << "LEFT JOIN contact ac ON (adcm.contactid=ac.id) "
            << "LEFT JOIN registrar ureg ON (d.upid=ureg.id) "
            << "LEFT JOIN enumval ev ON (d.id=ev.domainid) "
            << "WHERE d.registrant=c.id AND d.crid=creg.id "
            << "AND d.clid=r.id ";
        if (registrarFilter)
          sql << "AND r.id=" << registrarFilter << " ";
        if (!registrarHandleFilter.empty())
          sql << "AND r.handle='" << registrarHandleFilter << "' ";
        if (registrantFilter)
          sql << "AND c.id=" << registrantFilter << " ";
        if (!registrantHandleFilter.empty())
          sql << "AND c.handle='" << registrantHandleFilter << "' ";
        if (!crDateIntervalFilter.begin().is_special())
          sql << "AND d.crDate>='" 
              <<  to_iso_extended_string(crDateIntervalFilter.begin().date()) 
              << "' ";
        if (!crDateIntervalFilter.end().is_special())
          sql << "AND d.crDate<='" 
              <<  to_iso_extended_string(crDateIntervalFilter.end().date()) 
              << "' ";
        if (nsset)
          sql << "AND n.id=" << nsset << " ";
        if (!nssetHandle.empty())
          sql << "AND n.handle='" << nssetHandle << "' ";          
        if (admin)
          sql << "AND ac.id=" << admin << " ";
        if (!adminHandle.empty())
          sql << "AND ac.handle='" << adminHandle << "' ";          
        if (!fqdn.empty())
          sql << "AND d.fqdn LIKE '%" << fqdn << "%' ";          
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          unsigned id = atoi(db->GetFieldValue(i,0));
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
              atoi(db->GetFieldValue(i,2)), // zone
              atoi(db->GetFieldValue(i,3)), // nsset id
              db->GetFieldValue(i,4), // nsset handle
              atoi(db->GetFieldValue(i,5)), // registrant id
              db->GetFieldValue(i,6), // registrant handle
              db->GetFieldValue(i,7), // registrant name
              atoi(db->GetFieldValue(i,8)), // registrar
              db->GetFieldValue(i,9), // registrar handle
              MAKE_TIME(i,10), // crdate
              MAKE_TIME(i,11), // trdate
              MAKE_TIME(i,12), // update
              atoi(db->GetFieldValue(i,13)), // crid
              db->GetFieldValue(i,14), // crid handle
              atoi(db->GetFieldValue(i,15)), // upid
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
        registrarFilter = 0;
        registrarHandleFilter = "";
        crDateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
        registrantFilter = 0;
        registrantHandleFilter = "";
        nsset = 0;
        nssetHandle = "";
        admin = 0;
        adminHandle = "";
        fqdn = "";
        zoneFilter = 0;
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
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
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
