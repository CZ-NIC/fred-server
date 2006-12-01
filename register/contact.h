#ifndef CONTACT_H_
#define CONTACT_H_

#include <string>
#include <vector>
#include "object.h"
#include "exceptions.h"

/// forward declared parameter type 
class DB;

namespace Register
{
  namespace Contact 
  {
    class Contact : virtual public Register::Object
    {
     public:
      /// public destructor
      virtual ~Contact() {}
      /// return contact handle
      virtual const std::string& getHandle() const = 0;
      /// return contact name
      virtual const std::string& getName() const = 0;
      /// return contact organization
      virtual const std::string& getOrganization() const = 0;
      /// return contact street addres part 1
      virtual const std::string& getStreet1() const = 0;
      /// return contact street addres part 2
      virtual const std::string& getStreet2() const = 0;
      /// return contact street addres part 3
      virtual const std::string& getStreet3() const = 0;
      /// return contact state or province 
      virtual const std::string& getProvince() const = 0;
      /// return contact postl code      
      virtual const std::string& getPostalCode() const = 0;
      /// return contact city
      virtual const std::string& getCity() const = 0;
      /// return contact contry code      
      virtual const std::string& getCountry() const = 0;
      /// return contact phone number
      virtual const std::string& getTelephone() const = 0;
      /// return contact fax number
      virtual const std::string& getFax() const = 0;
      /// return contact email
      virtual const std::string& getEmail() const = 0;
      /// return contact notify email
      virtual const std::string& getNotifyEmail() const = 0;
      /// return contact identification string
      virtual const std::string& getSSN() const = 0;
      /// return contact identification type
      virtual const std::string& getSSNType() const = 0;
      /// return value added tax identification
      virtual const std::string& getVAT() const = 0;
      /// return disclose attribute for contact name
      virtual bool getDiscloseName() const = 0;
      /// return disclose attribute for contact name
      virtual bool getDiscloseOrganization() const = 0;
      /// return disclose attribute for contact name
      virtual bool getDiscloseAddr() const = 0;
      /// return disclose attribute for contact name
      virtual bool getDiscloseEmail() const = 0;
      /// return disclose attribute for contact name
      virtual bool getDiscloseTelephone() const = 0;
      /// return disclose attribute for contact name
      virtual bool getDiscloseFax() const = 0;
    };
    /// domain list
    class List : virtual public ObjectList
    {
     public:
      virtual ~List() {}
      /// get detail of loaded contact
      virtual Contact *get(unsigned idx) const = 0;
      /// set filter for handle
      virtual void setHandleFilter(const std::string& handle) = 0;
      /// set filter for name
      virtual void setNameFilter(const std::string& name) = 0;
      /// set filter for identity id
      virtual void setIdentFilter(const std::string& ident) = 0;
      /// set filter for email
      virtual void setEmailFilter(const std::string& email) = 0;
      /// set filter for organization
      virtual void setOrganizationFilter(const std::string& org) = 0;
      /// set filter for VAT number
      virtual void setVATFilter(const std::string& vat) = 0;
      /// reload list with current filter
      virtual void reload() throw (SQL_ERROR) = 0;
      /// clear filter data
      virtual void clearFilter() = 0;      
    };
    /// main entry class
    class Manager
    {
     public:
      /// destructor 
      virtual ~Manager() {}
      /// Return list of domains
      virtual List *getList() = 0;
      /// factory method
      static Manager *create(DB *db);
    };
  } // namespace Contact
} // namespace Register

#endif /* CONTACT_H_ */
