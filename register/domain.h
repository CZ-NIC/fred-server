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
    /// return type for checkAvail method 
    enum CheckAvailType {
      CA_INVALID_HANDLE, ///< bad formed handle
      CA_BAD_ZONE, ///< domain outside of register
      CA_BAD_LENGHT, ///< domain longer then acceptable
      CA_BLACKLIST, ///< registration blocked in blacklist
      CA_REGISTRED, ///< domain registred
      CA_PARENT_REGISTRED, ///< parent already registred
      CA_AVAILABLE ///< domain is available
    };
    /// tokenized domain name type
    typedef std::vector<std::string> DomainName;
    /// main entry class
    class Manager
    {
     public:
      /// destructor 
      virtual ~Manager() {}      
      /// translate phone number into domain name
      virtual std::string makeEnumDomain(const std::string& number)
        const throw (NOT_A_NUMBER) = 0;
      /// tokenize domain name into sequence
      virtual void parseDomainName(const std::string& fqdn, DomainName& domain) 
        const throw (INVALID_DOMAIN_NAME) = 0;
      /// check availability of domain  
      virtual CheckAvailType checkAvail(const std::string& fqdn) const = 0; 
      /// factory method
      static Manager *create(DB *db, Zone::Manager *zm);
    };
  } // namespace Domain
} // namespace Admin

#endif /*DOMAIN_H_*/
