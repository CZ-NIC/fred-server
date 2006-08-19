#ifndef ZONE_H_
#define ZONE_H_

#include <string>

/// forward declaration for database connection
class DB;

namespace Register
{
  namespace Zone
  {
    /// holder for zones managed by register
    class Manager
    {
     public:
      /// return default enum country prefix e.g '0.2.4'
      virtual const std::string& getDefaultEnumSuffix() const = 0;
      /// return enum zone string 'e164.arpa'
      virtual const std::string& getEnumZoneString() const = 0;
      /// find zone from domain fqdn
      virtual unsigned findZoneId(const std::string& fqdn) const = 0;
      /// create manager object
      static Manager *create(DB *db);
    };
  };
};

#endif /*ZONE_H_*/

