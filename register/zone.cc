#include <vector>
#include <algorithm>
#include <functional>
#include "zone.h"
#include "dbsql.h"
#include <iostream>
#include "types.h"

#define RANGE(x) x.begin(),x.end()

namespace Register
{
  namespace Zone
  {
    struct ZoneImpl : public virtual Zone {
      ZoneImpl(TID _id, const std::string& _fqdn, bool _isEnum) :
        id(_id), fqdn(_fqdn), isEnum(_isEnum)
      {}
      ~ZoneImpl() {}
      TID id;
      std::string fqdn;
      bool isEnum;
      /// compare if domain belongs to this zone (according to suffix)
      bool operator==(const std::string& domain) const
      {
        unsigned l = fqdn.length();
        if (domain.length() < l) return false;
        return domain.compare(domain.length()-l,l,fqdn) == 0;  
      }
      /// interface implementation
      const std::string& getFqdn() const 
      {
        return fqdn; 
      }
      /// interface implementation
      bool isEnumZone() const 
      {
        return isEnum; 
      }           
    };
    typedef std::vector<ZoneImpl> ZoneList;
    class ManagerImpl : virtual public Manager
    {
      ZoneList zoneList;
      DB *db;
      std::string defaultEnumSuffix;
      std::string enumZoneString;
      std::string defaultDomainSuffix;
     public:
      ManagerImpl(DB *_db) :
       db(_db), 
       defaultEnumSuffix("0.2.4"),
       enumZoneString("e164.arpa"),
       defaultDomainSuffix("cz") 
      {
        if (!db->ExecSelect("SELECT id,fqdn,enum_zone FROM zone")) return;
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          zoneList.push_back(ZoneImpl(
           STR_TO_ID(db->GetFieldValue(i,0)),
           db->GetFieldValue(i,1),
           *db->GetFieldValue(i,2) == 't' ? true : false
           ));
        }
      }
      const std::string& getDefaultEnumSuffix() const
      {
        return defaultEnumSuffix;
      }
      const std::string& getDefaultDomainSuffix() const
      {
        return defaultDomainSuffix;
      }
      const std::string& getEnumZoneString() const
      {
        return enumZoneString; 
      }
      const Zone* findZoneId(const std::string& fqdn) const
      {
        ZoneList::const_iterator i = find(RANGE(zoneList),fqdn);
        if (i!=zoneList.end()) return &(*i);
        else return NULL;
      }
    };
    Manager* Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }
  };
};

