#include <memory>
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
   void checkHandle(const std::string& handle, CheckHandle& ch)
   {
     try {
       // trying convert string to enum domain
       ch.newHandle = dm->makeEnumDomain(handle);
       if (!zm->findZoneId(ch.newHandle)) ch.handleClass = CH_BAD_ENUM;
       else ch.handleClass = CH_ENUM;
     } 
     catch (...) {
       // no enum, parse as domain name
       Domain::DomainName dn;
       try {
         dm->parseDomainName(handle,dn);
         if (!zm->findZoneId(handle)) ch.handleClass = CH_BAD_ZONE;
         else {
          if (dn.size() > 2) {
          }
         }
       }
       catch (...) {
         // no domain name, could be nsset or contact
       } 
     }
   }
 };
 Manager *Manager::create(DB *db)
 {
   return new ManagerImpl(db);
 }
}
