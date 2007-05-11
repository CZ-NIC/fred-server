#include <vector>
#include <algorithm>
#include <functional>
#include "zone.h"
#include "dbsql.h"
#include <iostream>
#include "types.h"
#include <ctype.h>

#define RANGE(x) x.begin(),x.end()

namespace Register
{
  namespace Zone
  {
    struct ZoneImpl : public virtual Zone {
      ZoneImpl(
        TID _id, const std::string& _fqdn, bool _isEnum, 
        unsigned _maxLevel
      ) :
        id(_id), fqdn(_fqdn), isEnum(_isEnum), maxLevel(_maxLevel)
      {}
      ~ZoneImpl() {}
      TID id;
      std::string fqdn;
      bool isEnum;
      unsigned maxLevel; ///< Maximal domain level in this zone
      /// compare if domain belongs to this zone (according to suffix)
      bool operator==(const std::string& domain) const
      {
        std::string copy = domain;
        for (unsigned i=0; i<copy.length(); i++)
          copy[i] = tolower(copy[i]);
        unsigned l = fqdn.length();
        if (copy.length() < l) return false;
        return copy.compare(copy.length()-l,l,fqdn) == 0;  
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
      /// interface implementation
      virtual unsigned getMaxLevel() const
      {
        return maxLevel;
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
        if (!db->ExecSelect(
          "SELECT id, fqdn, enum_zone, "
          "ARRAY_UPPER(STRING_TO_ARRAY(fqdn,'.'),1) + dots_max "
          "FROM zone"
        )) return;
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          zoneList.push_back(ZoneImpl(
           STR_TO_ID(db->GetFieldValue(i,0)),
           db->GetFieldValue(i,1),
           *db->GetFieldValue(i,2) == 't' ? true : false,
           atoi(db->GetFieldValue(i,3))
           ));
        }
        db->FreeSelect();
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

