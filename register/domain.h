#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <string>
#include <vector>
#include "zone.h"
#include "object.h"
#include "exceptions.h"

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
      /// return fully qualified domain name
      virtual const std::string& getFQDN() const = 0;
      /// return id of zone
      virtual TID getZoneId() const = 0;
      /// return handle of nsset
      virtual const std::string& getNSSetHandle() const = 0;
      /// return id of nsset
      virtual TID getNSSetId() const = 0;
      /// set nsset
      virtual void setNSSetId(TID nsset) = 0;
      /// return handle of registrant
      virtual const std::string& getRegistrantHandle() const = 0;
      /// return name of registrant
      virtual const std::string& getRegistrantName() const = 0;
      /// return id of registrant
      virtual TID getRegistrantId() const = 0;
      /// set registrant 
      virtual void setRegistrantId(TID registrant) = 0;
      /// return count of admin contacts
      virtual unsigned getAdminCount() const = 0;
      /// return id of admin contact by index
      virtual TID getAdminIdByIdx(unsigned idx) const
        throw (NOT_FOUND) = 0;
      /// return handle of admin contact by index
      virtual const std::string& getAdminHandleByIdx(unsigned idx) const 
        throw (NOT_FOUND) = 0;
      /// remove contact from admin contact list
      virtual void removeAdminId(TID id) = 0;
      /// insert contact into admin contact list
      virtual void insertAdminId(TID id) = 0;
      /// return date of registration expiration
      virtual ptime getExpirationDate() const = 0;
      /// return date of validation expiration
      virtual ptime getValExDate() const = 0;
    };
    /// domain list
    class List : virtual public ObjectList
    {
     public:
      virtual ~List() {}
      /// get detail of loaded domain
      virtual Domain *get(unsigned idx) const = 0;
      /// set filter for domain zone
      virtual void setZoneFilter(TID zoneId) = 0;
      /// set filter for registrant
      virtual void setRegistrantFilter(TID registrantId) = 0;
      /// set filter for registrant handle
      virtual void setRegistrantHandleFilter(
        const std::string& registrantHandle
      ) = 0;
      /// set filter for nsset
      virtual void setNSSetFilter(TID nssetId) = 0;
      /// set filter for nsset handle
      virtual void setNSSetHandleFilter(const std::string& nssetHandle) = 0;
      /// set filter for admin
      virtual void setAdminFilter(TID adminId) = 0;
      /// set filter for admin handle
      virtual void setAdminHandleFilter(const std::string& adminHandle) = 0;
      /// set filter for domain name 
      virtual void setFQDNFilter(const std::string& fqdn) = 0;
      /// set filter for period of expiration date
      virtual void setExpirationDateFilter(time_period period) = 0;
      /// set filter for period of val. expiration date
      virtual void setValExDateFilter(time_period period) = 0;
      /// set filter for admin handle in associated nsset 
      virtual void setTechAdminHandleFilter(const std::string& handle) = 0;
      /// set filter for IP address in associated nsset 
      virtual void setHostIPFilter(const std::string& ip) = 0;
      /// reload list with current filter
      virtual void reload() = 0;
      /// clear filter data
      virtual void clearFilter() = 0;      
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
      /// return current count of enum domains
      virtual unsigned long getEnumDomainCount() const = 0;
      /// return current count of enum numbers
      virtual unsigned long getEnumNumberCount() const = 0;  
      /// Return list of domains
      virtual List *getList() = 0;
      /// factory method
      static Manager *create(DB *db, Zone::Manager *zm);
    };
  } // namespace Domain
} // namespace Register

#endif /*DOMAIN_H_*/
