#ifndef ZONE_H_
#define ZONE_H_

#include <string>

/// forward declaration for database connection
class DB;

namespace Register
{
  namespace Zone
  {
    /// zone attributes and specific parameters 
    class Zone {
     protected:
      /// protected destruktor - object is managed by Manager object
      virtual ~Zone() {}
     public:
      /// suffix of domain name for this zone
      virtual const std::string& getFqdn() const = 0;
      ///< is zone for enum domains?
      virtual bool isEnumZone() const = 0;     
      ///< return maximal level of domains in this zone
      virtual unsigned getMaxLevel() const = 0;
    };
    /// holder for zones managed by register
    class Manager
    {
     public:
      /// destruktor
      virtual ~Manager() {}
      /// return default enum country prefix e.g '0.2.4'
      virtual const std::string& getDefaultEnumSuffix() const = 0;
      /// return default domain suffix e.g. 'cz'
      virtual const std::string& getDefaultDomainSuffix() const = 0;
      /// return enum zone string 'e164.arpa'
      virtual const std::string& getEnumZoneString() const = 0;
      /// find zone from domain fqdn
      virtual const Zone* findZoneId(const std::string& fqdn) const = 0;
      /// create manager object
      static Manager *create(DB *db);
    };
  };
};

#endif /*ZONE_H_*/

