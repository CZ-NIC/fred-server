#include <memory> ///< auto_ptr<>
#include <ctype.h> ///< toupper()
#include "register.h"
#include "dbsql.h"
#include "zone.h"
#include "domain.h"

namespace Register
{
 class ManagerImpl : virtual public Manager
 {
   DB *db;
   std::auto_ptr<Zone::Manager> zm;
   std::auto_ptr<Domain::Manager> dm;
   std::auto_ptr<Registrar::Manager> rm;
   std::auto_ptr<Contact::Manager> cm;
   std::auto_ptr<NSSet::Manager> nm;
   std::vector<CountryDesc> countries;
   /// classify domain status as handle status
   CheckHandleClass classifyDomain(
     const std::string& fqdn, std::string& conflictFqdn) const
   {
     switch (dm->checkAvail(fqdn,conflictFqdn)) {
       case Domain::CA_INVALID_HANDLE:
       case Domain::CA_BAD_LENGHT:
       case Domain::CA_BAD_ZONE: return CH_UNREGISTRABLE;
       case Domain::CA_PROTECTED:
       case Domain::CA_BLACKLIST: return CH_PROTECTED;
       case Domain::CA_REGISTRED: return CH_REGISTRED;
       case Domain::CA_PARENT_REGISTRED: return CH_REGISTRED_PARENT;
       case Domain::CA_CHILD_REGISTRED: return CH_REGISTRED_CHILD;
       case Domain::CA_AVAILABLE: return CH_FREE;
     }
     return CH_UNREGISTRABLE;
   }
  public:
   ManagerImpl(DB *_db) : db(_db)
   {
     zm.reset(Zone::Manager::create(db));
     dm.reset(Domain::Manager::create(db,zm.get()));
     rm.reset(Registrar::Manager::create(db));
     cm.reset(Contact::Manager::create(db));
     nm.reset(NSSet::Manager::create(db));
     // TODO SQL load
     CountryDesc cd;
     cd.cc = "CZ";
     cd.name = "Czech Republic";
     countries.push_back(cd);
     cd.cc = "SK";
     cd.name = "Slovak Republic";
     countries.push_back(cd);
   } 
#define CHECK_DOM \

   /// interface method implementation
   void checkHandle(const std::string& handle, CheckHandleList& chl) const
   {
     CheckHandle chnew;
     chnew.type = HT_OTHER;
     chnew.handleClass= CH_FREE;
     chl.push_back(chnew);
     CheckHandle& ch(chl.back());
     try {
       ch.newHandle.clear();
       // trying convert string to enum domain
       ch.newHandle = dm->makeEnumDomain(handle);
       // succeeded!
       ch.type = HT_ENUM_NUMBER;
       ch.handleClass = classifyDomain(ch.newHandle,ch.conflictHandle);
       return;
     } 
     catch (...) {
       // no enum, parse as domain name
       Domain::DomainName dn;
       try {
         dm->parseDomainName(handle,dn);
         // succeed!
         if (dn.size() == 1) {
          throw Domain::INVALID_DOMAIN_NAME();
          /* NOT AVAILABLE NOW 
           // single name is appended by default domain
           ch.newHandle = dn[0];
           ch.newHandle += '.';
           ch.newHandle += zm->getDefaultDomainSuffix();
           ch.type = HT_DOMAIN;
           ch.handleClass = CH_UNREGISTRABLE;
           return;
          */
         }
         const Zone::Zone *z = zm->findZoneId(handle); 
         if (!z) {
           // unregistrable because of diffrenet zone
           // must determine if it is enum domain or nod  
           ch.handleClass = CH_UNREGISTRABLE;
           const std::string& esuf = zm->getDefaultEnumSuffix();
           if (handle.rfind(esuf) + esuf.size() == handle.size())
             ch.type = HT_ENUM_DOMAIN;
           else 
             ch.type = HT_DOMAIN;
         }
         else {
           // special test for enum domain (parts are single numbers)
           if (z->isEnumZone()) {
             if (!dm->checkEnumDomainName(dn))
               throw Domain::INVALID_DOMAIN_NAME();
             ch.type = HT_ENUM_DOMAIN;
             ch.handleClass = classifyDomain(handle,ch.conflictHandle);
             return;
           }
           // is normal domain
           ch.type = HT_DOMAIN;           
           // only second level domain is allowed           
           if (dn.size() != 2) ch.handleClass = CH_UNREGISTRABLE_LONG;
           else {
             ch.handleClass = classifyDomain(handle,ch.conflictHandle);
            /* NOT AVAILABLE NOW
             ch.handleClass = CH_DOMAIN_LONG;
             // long domain name is truncated to 2nd level
             ch.newHandle = dn[dn.size()-2];
             ch.newHandle += '.';
             ch.newHandle += dn[dn.size()-1];
            */
           } 
         }
       }
       catch (...) {
         // no domain name, could be nsset or contact
         // check contact
         Contact::Manager::CheckAvailType cca = 
           getContactManager()->checkAvail(handle);
         if (cca != Contact::Manager::CA_INVALID_HANDLE) {
           // is contact
           ch.type = HT_CONTACT;
           switch (cca) {
             case Contact::Manager::CA_PROTECTED : 
               ch.handleClass= CH_PROTECTED; 
               break;
             case Contact::Manager::CA_REGISTRED :
               ch.handleClass= CH_REGISTRED; 
               break;
             default:
               ch.handleClass = CH_FREE;
           };
           return;
         }
         // not contact, could be nsset   
         NSSet::Manager::CheckAvailType nca = 
           getNSSetManager()->checkAvail(handle);
         if (nca != NSSet::Manager::CA_INVALID_HANDLE) {
           // is nsset
           ch.type = HT_NSSET;
           switch (nca) {
             case NSSet::Manager::CA_PROTECTED : 
               ch.handleClass= CH_PROTECTED; 
               break;
             case NSSet::Manager::CA_REGISTRED :
               ch.handleClass= CH_REGISTRED; 
               break;
             default:
               ch.handleClass = CH_FREE;
           };
           return;
         }
         if (getRegistrarManager()->checkHandle(handle)) {
           ch.type = HT_REGISTRAR;
           ch.handleClass= CH_REGISTRED;             
         }
         // left default settings
         return;
       } 
     }
   }
   Domain::Manager *getDomainManager()
   {
     return dm.get();
   }   
   Registrar::Manager *getRegistrarManager() const
   {
     return rm.get();
   }   
   Contact::Manager *getContactManager() const
   {
     return cm.get();
   }   
   NSSet::Manager *getNSSetManager() const
   {
     return nm.get();
   }
   virtual unsigned getCountryDescSize() const
   {
     return countries.size();
   }
   virtual const CountryDesc& getCountryDescByIdx(unsigned idx) const
     throw (NOT_FOUND)
   {
     if (idx >= countries.size()) throw NOT_FOUND();
     return countries[idx];
   }   
 };
 Manager *Manager::create(DB *db)
 {
   return new ManagerImpl(db);
 }
}
