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
  public:
   ManagerImpl(DB *_db) : db(_db)
   {
     zm.reset(Zone::Manager::create(db));
     dm.reset(Domain::Manager::create(db,zm.get()));
   } 
   /// interface method implementation
   void checkHandle(const std::string& handle, CheckHandle& ch) const
   {
     try {
       ch.newHandle.clear();
       // trying convert string to enum domain
       ch.newHandle = dm->makeEnumDomain(handle);
       if (!zm->findZoneId(ch.newHandle)) ch.handleClass = CH_ENUM_BAD_ZONE;
       else ch.handleClass = CH_ENUM;
       return;
     } 
     catch (...) {
       // no enum, parse as domain name
       Domain::DomainName dn;
       try {
         dm->parseDomainName(handle,dn);
         if (dn.size() == 1) {
           // single name is appended by default domain
           ch.newHandle = dn[0];
           ch.newHandle += '.';
           ch.newHandle += zm->getDefaultDomainSuffix();
           ch.handleClass = CH_DOMAIN_PART;
           return;
         }
         const Zone::Zone *z = zm->findZoneId(handle); 
         if (!z) ch.handleClass = CH_DOMAIN_BAD_ZONE;
         else {
           // special test for enum domain (parts are single numbers)
           if (z->isEnumZone()) {
             if (dm->checkEnumDomainName(dn)) ch.handleClass = CH_ENUM;
             else ch.handleClass = CH_INVALID;
             return;
           }
           // only second level domain is allowed
           if (dn.size() == 2) ch.handleClass = CH_DOMAIN;
           else {
             ch.handleClass = CH_DOMAIN_LONG;
             // long domain name is truncated to 2nd level
             ch.newHandle = dn[dn.size()-2];
             ch.newHandle += '.';
             ch.newHandle += dn[dn.size()-1];
           } 
         }
       }
       catch (...) {
         // no domain name, could be nsset or contact
         std::string upper;
         for (unsigned i=0; i<handle.size(); i++) 
           upper+=toupper(handle[i]);
         if (!upper.compare(0,6,"NSSID:")) ch.handleClass = CH_NSSET;
         else if (!upper.compare(0,4,"CID:")) ch.handleClass = CH_CONTACT;
         else ch.handleClass = CH_INVALID;
       } 
     }
   }
   Domain::Manager *getDomainManager()
   {
     return dm.get();
   }   
 };
 Manager *Manager::create(DB *db)
 {
   return new ManagerImpl(db);
 }
}
