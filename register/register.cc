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
   bool restrictedHandles;
  public:
   ManagerImpl(DB *_db, bool _restrictedHandles) : 
     db(_db), restrictedHandles(_restrictedHandles)
   {
     zm.reset(Zone::Manager::create(db));
     dm.reset(Domain::Manager::create(db,zm.get()));
     rm.reset(Registrar::Manager::create(db));
     cm.reset(Contact::Manager::create(db,restrictedHandles));
     nm.reset(NSSet::Manager::create(db,restrictedHandles));
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
     CheckHandle ch;
     bool isEnum = false;
     try {
       // trying convert string to enum domain
       ch.newHandle = dm->makeEnumDomain(handle);
       ch.type = HT_ENUM_NUMBER;
       isEnum = true;
     } catch (...) {}
     bool isDomain = true;
     std::string fqdn = isEnum ? ch.newHandle : handle;
     NameIdPair conflictFQDN; 
     switch (dm->checkAvail(fqdn, conflictFQDN)) {
       case Domain::CA_INVALID_HANDLE:
        isDomain = false; break;
       case Domain::CA_BAD_LENGHT: 
         ch.handleClass = CH_UNREGISTRABLE_LONG; break;
       case Domain::CA_BAD_ZONE:
         ch.handleClass = CH_UNREGISTRABLE; break;
       case Domain::CA_BLACKLIST: 
         ch.handleClass = CH_PROTECTED; break;
       case Domain::CA_REGISTRED:
         ch.handleClass = CH_REGISTRED; break;
       case Domain::CA_PARENT_REGISTRED: 
         ch.handleClass = CH_REGISTRED_PARENT; break;
       case Domain::CA_CHILD_REGISTRED: 
         ch.handleClass = CH_REGISTRED_CHILD; break;
       case Domain::CA_AVAILABLE:
         ch.handleClass = CH_FREE; break;
     }
     ch.conflictHandle = conflictFQDN.name;
     if (isDomain) {
       if (!isEnum)
         ch.type = dm->checkEnumDomainSuffix(fqdn) ? HT_ENUM_DOMAIN : HT_DOMAIN;
       chl.push_back(ch);
     }
     // check if handle is registred contact
     NameIdPair conflictContact;
     if (getContactManager()->checkAvail(handle,conflictContact) == 
         Contact::Manager::CA_REGISTRED) {
       CheckHandle chCon;
       chCon.type = HT_CONTACT;
       chCon.handleClass= CH_REGISTRED; 
       chl.push_back(chCon);
     }
     // check if handle is registred nsset   
     NameIdPair conflictNSSet;
     if (getNSSetManager()->checkAvail(handle,conflictNSSet) == 
         NSSet::Manager::CA_REGISTRED) {
       CheckHandle chNss;
       chNss.type = HT_NSSET;
       chNss.handleClass= CH_REGISTRED; 
       chl.push_back(chNss);
     }
     // check if handle is registrant   
     if (getRegistrarManager()->checkHandle(handle)) {
       CheckHandle chReg;
       chReg.type = HT_REGISTRAR;
       chReg.handleClass= CH_REGISTRED;
       chl.push_back(chReg);       
     }
     // if empty return OTHER   
     if (!chl.size()) {
       CheckHandle chOth;
       chOth.type = HT_OTHER;
       chOth.handleClass= CH_FREE;
       chl.push_back(chOth);
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
 Manager *Manager::create(DB *db, bool restrictedHandles)
 {
   return new ManagerImpl(db, restrictedHandles);
 }
}
