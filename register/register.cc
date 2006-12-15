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
       switch (dm->checkAvail(ch.newHandle)) {
         case Domain::CA_INVALID_HANDLE: // TODO: log problem,  
         case Domain::CA_BAD_LENGHT:
         case Domain::CA_BAD_ZONE: ch.handleClass = CH_UNREGISTRABLE; break;
         case Domain::CA_PROTECTED:
         case Domain::CA_BLACKLIST: ch.handleClass = CH_PROTECTED; break;
         case Domain::CA_REGISTRED: ch.handleClass = CH_REGISTRED; break;
         case Domain::CA_PARENT_REGISTRED: ch.handleClass = CH_REGISTRED_PARENT; break;
         case Domain::CA_CHILD_REGISTRED: ch.handleClass = CH_REGISTRED_CHILD; break;
         case Domain::CA_AVAILABLE: ch.handleClass = CH_FREE; break;       
       }
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
           ch.handleClass = CH_UNREGISTRABLE;
           const std::string& esuf = zm->getDefaultEnumSuffix();
           if (handle.rfind(esuf) + esuf.size() == handle.size())
             ch.type = HT_DOMAIN;
           else 
             ch.type = HT_ENUM_DOMAIN;
         }
         else {
           // special test for enum domain (parts are single numbers)
           if (z->isEnumZone()) {
             if (!dm->checkEnumDomainName(dn))
               throw Domain::INVALID_DOMAIN_NAME();
             ch.type = HT_ENUM_DOMAIN;
             ch.handleClass = CH_FREE; // TODO: CHECK!!!!! 
             return;
           }
           // is normal domain
           ch.type = HT_DOMAIN;           
           // only second level domain is allowed           
           if (dn.size() != 2) ch.handleClass = CH_UNREGISTRABLE_LONG;
           else {
             ch.handleClass = CH_FREE;
            /*
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
         std::string upper;
         for (unsigned i=0; i<handle.size(); i++) 
           upper+=toupper(handle[i]);
         if (!upper.compare(0,6,"NSSID:")) {
           ch.type = HT_NSSET;
           ch.handleClass = CH_FREE;
         }
         else if (!upper.compare(0,4,"CID:")) {
           ch.type = HT_CONTACT;
           ch.handleClass = CH_FREE;
         }
         else return; // default
       } 
     }
   }
   Domain::Manager *getDomainManager()
   {
     return dm.get();
   }   
   Registrar::Manager *getRegistrarManager()
   {
     return rm.get();
   }   
   Contact::Manager *getContactManager()
   {
     return cm.get();
   }   
   NSSet::Manager *getNSSetManager()
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
