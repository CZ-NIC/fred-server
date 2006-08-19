#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <string>
#include <vector>
#include "zone.h"

/// forward declared parameter type 
class DB;

namespace Register
{
  namespace Domain 
  {
    /// exception thrown when string is not a phone number
    class NOT_A_NUMBER {}; 
    /// exception thrown when string is not a domain name
    class INVALID_DOMAIN_NAME {}; 
    /// tokenized domain name type
    typedef std::vector<std::string> DomainName;
    /// main entry class
    class Manager
    {
     public:
      /// destructor 
      virtual ~Manager() = 0;      
      /// translate phone number into domain name
      virtual std::string makeEnumDomain(const std::string& number)
        throw (NOT_A_NUMBER) = 0;
      /// tokenize domain name into sequence
      virtual void parseDomainName(const std::string& fqdn, DomainName& domain) 
        throw (INVALID_DOMAIN_NAME) = 0;
      /// create manager object
      static Manager *create(DB *db, Zone::Manager *zm);
    };
  } // namespace Domain
} // namespace Admin

#endif /*DOMAIN_H_*/
