#ifndef CONTACT_H_
#define CONTACT_H_

#include <string>
#include <vector>
#include "object.h"
#include "exceptions.h"
#include "db_settings.h"
#include "model/model_filters.h"
#include "old_utils/dbsql.h"

namespace Fred {
namespace Contact {

/// member identification (i.e. for sorting)
enum MemberType {
  MT_HANDLE, ///< contact identificator
  MT_NAME, ///< name
  MT_ORGANIZATION, ///< organization
  MT_CRDATE, ///< create date
  MT_ERDATE, ///< delete date
  MT_REGISTRAR_HANDLE, ///< registrar handle
};


class Contact : virtual public Fred::Object {
public:
  /// public destructor
  virtual ~Contact() {
  }
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
  /// return id of contact identification type
  virtual unsigned getSSNTypeId() const = 0;
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
  /// return disclose attribute for contact vat
  virtual bool getDiscloseVat() const = 0;
  /// return disclose attribute for contact identification token
  virtual bool getDiscloseIdent() const = 0;
  /// return disclose attribute for contact notify email
  virtual bool getDiscloseNotifyEmail() const = 0;
};


/// domain list
class List : virtual public ObjectList {
public:
  virtual ~List() {
  }
  /// get detail of loaded contact
  virtual Contact *getContact(unsigned idx) const = 0;
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
  /// testing new reload function
  virtual void reload(Database::Filters::Union &uf) = 0;
  /// reload list with current filter
  virtual void reload() = 0;
  /// clear filter data
  virtual void clearFilter() = 0;
  /// sort by column
  virtual void sort(MemberType _member, bool _asc) = 0;
};


/// main entry class
class Manager {
public:
  /// destructor 
  virtual ~Manager() {
  }
  /// return list of domains
  virtual List *createList() = 0;
  /// type for check 
  enum CheckAvailType {
    /// handle cannot be contact
    CA_INVALID_HANDLE,
    /// handle is already registred
    CA_REGISTRED,
    /// handle is in protected period
    CA_PROTECTED,
    /// handle is free for registration
    CA_FREE
  };
  /// check proper format of handle
  virtual bool checkHandleFormat(const std::string& handle) const = 0;
  /// check possibilities for registration
  virtual CheckAvailType checkAvail(
          const std::string& handle,
          NameIdPair& conflict,
          bool lock = false) const 
      throw (SQL_ERROR) = 0;
  /// check with id
  virtual CheckAvailType checkAvail(
          const unsigned long long &_id,
          NameIdPair& conflict,
          bool lock = false) const = 0;
  virtual unsigned long long findRegistrarId(
          const unsigned long long &_id) const = 0;
  /// factory method
  static Manager *create(DBSharedPtr db, bool restrictedHandle);
};

typedef std::auto_ptr<Manager> ManagerPtr;

} // namespace Contact
} // namespace Fred

#endif /* CONTACT_H_ */
