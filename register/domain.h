#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <string>
#include <vector>
#include "zone.h"
#include "object.h"

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
    /// domain detail
    class Domain : virtual public Register::Object
    {
     public:
      /// public destructor
      virtual ~Domain() {}
      /// return id of domain
      virtual unsigned getId() const = 0;
      /// return fully qualified domain name
      virtual const std::string& getFQDN() const = 0;
      /// return id of zone
      virtual unsigned getZoneId() const = 0;
      /// return handle of nsset
      virtual const std::string& getNSSetHandle() const = 0;
      /// return id of nsset
      virtual unsigned getNSSetId() const = 0;
      /// set nsset
      virtual void setNSSetId(unsigned nsset) = 0;
      /// return handle of registrant
      virtual const std::string& getRegistrantHandle() const = 0;
      /// return id of registrant
      virtual unsigned getRegistrantId() const = 0;
      /// set registrant 
      virtual void setRegistrantId(unsigned registrant) = 0;
      /// return count of admin contacts
      virtual unsigned getAdminCount() const = 0;
      /// return id of admin contact by index
      virtual unsigned getAdminByIdx(unsigned idx) const = 0;
      /// remove contact from admin contact list
      virtual void removeAdminId(unsigned id) = 0;
      /// insert contact into admin contact list
      virtual void insertAdminId(unsigned id) = 0;
    };
    /// domain list
    class List
    {
     public:
      virtual ~List() {}
      /// return count of domains in list
      virtual unsigned getCount() const = 0;
      /// get detail of loaded domain
      virtual Domain *get(unsigned idx) const = 0;
      /// set filter for domain zone
      virtual void setZoneFilter(unsigned zoneId) = 0;
      /// set filter for registrar
      virtual void setRegistrarFilter(unsigned registrarId) = 0;
      /// set filter for registrar handle
      virtual void setRegistrarHandleFilter(
        const std::string& registrarHandle
      ) = 0;
      /// set filter for registrant
      virtual void setRegistrantFilter(unsigned registrantId) = 0;
      /// set filter for registrant handle
      virtual void setRegistrantHandleFilter(
        const std::string& registrantHandle
      ) = 0;
      /// reload list with current filter
      virtual void reload() = 0;
    };
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
      virtual void parseDomainName(
       const std::string& fqdn, DomainName& domain
      ) const throw (INVALID_DOMAIN_NAME) = 0;
      /// check availability of domain  
      virtual CheckAvailType checkAvail(const std::string& fqdn) const = 0;
      /// check validity of enum domain name (every part is one digit)
      virtual bool checkEnumDomainName(DomainName& domain) const = 0; 
      /// Return list of domains
      virtual List *getList() = 0;
      /// factory method
      static Manager *create(DB *db, Zone::Manager *zm);
    };
  } // namespace Domain
} // namespace Register

#endif /*DOMAIN_H_*/
