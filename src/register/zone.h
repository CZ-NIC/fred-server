#ifndef ZONE_H_
#define ZONE_H_

#include <string>
#include "types/data_types.h"
#include "types.h" 
#include "exceptions.h"

/// forward declaration for database connection
class DB;

namespace Register
{
  namespace Zone
  {
      enum Operation {
          CREATE, 
          RENEW
      };

    /// exception thrown when string is not a domain name
    class INVALID_DOMAIN_NAME {};
    /// exception thrown when string is not a phone number
    class NOT_A_NUMBER {};     
    /// tokenized domain name type
    typedef std::vector<std::string> DomainName;
    /// zone attributes and specific parameters 
    class Zone {
     protected:
      /// protected destruktor - object is managed by Manager object
      virtual ~Zone() {}
     public:
      /// id of domain
      virtual const TID getId() const = 0;
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
      /// encode UTF8 domain name into IDN ascii string
      virtual std::string encodeIDN(const std::string& fqdn) const = 0;
      /// decode IDN ascii domain name into UTF8 string
      virtual std::string decodeIDN(const std::string& fqdn) const = 0;      
      /// tokenize domain name into sequence
      virtual void parseDomainName(
        const std::string& fqdn, DomainName& domain, bool allowIDN
      ) const throw (INVALID_DOMAIN_NAME) = 0;
      /// check validity of enum domain name (every part is one digit)
      virtual bool checkEnumDomainName(const DomainName& domain) const = 0;
      /// check if domain is under global zone e164.arpa
      virtual bool checkEnumDomainSuffix(const std::string& fqdn) const = 0;
      /// translate phone number into domain name
      virtual std::string makeEnumDomain(const std::string& number)
        const throw (NOT_A_NUMBER) = 0;      
      /// return default enum country prefix e.g '0.2.4'
      virtual const std::string& getDefaultEnumSuffix() const = 0;
      /// return default domain suffix e.g. 'cz'
      virtual const std::string& getDefaultDomainSuffix() const = 0;
      /// return enum zone string 'e164.arpa'
      virtual const std::string& getEnumZoneString() const = 0;
      /// find zone from domain fqdn
      virtual const Zone* findZoneId(const std::string& fqdn) const = 0;
      /// check fqdn agains list of toplevel domain (true=found) 
      virtual bool checkTLD(const DomainName& domain) const = 0;
      /// add zone
      virtual void addZone(
              const std::string& fqdn,
              int ex_period_min=12,
              int ex_period_max=120,
              int ttl=18000,
              const std::string &hostmaster="hostmaster@localhost",
              int refresh=10600,
              int update_retr=3600,
              int expiry=1209600,
              int minimum=7200,
              const std::string &ns_fqdn="localhost")
        throw (SQL_ERROR, ALREADY_EXISTS) = 0;
      virtual void addZoneNs(
              const std::string &zone,
              const std::string &fqdn="localhost",
              const std::string &addr="")
          throw (SQL_ERROR) = 0;
      virtual void addPrice(
              const std::string &zone,
              Operation operation,
              const Database::DateTime &validFrom,
              const Database::DateTime &validTo,
              const Database::Money &price,
              int period)
          throw (SQL_ERROR) = 0;
      /// create manager object
      static Manager *create(DB *db);
    };
  };
};

#endif /*ZONE_H_*/

