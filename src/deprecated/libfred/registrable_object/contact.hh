/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CONTACT_HH_716547AF05904A3181F01E84CDEE42E0
#define CONTACT_HH_716547AF05904A3181F01E84CDEE42E0

#include <string>
#include <vector>
#include "src/deprecated/libfred/object.hh"
#include "src/deprecated/libfred/exceptions.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"
#include "src/deprecated/util/dbsql.hh"

namespace LibFred {
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

class Address
{
public:
  typedef struct _Type
  {
    static const std::string MAILING; // LibFred::Contact::Address::Type::MAILING  = "MAILING"
    static const std::string SHIPPING;// LibFred::Contact::Address::Type::SHIPPING = "SHIPPING"
    static const std::string BILLING; // LibFred::Contact::Address::Type::BILLING  = "BILLING"
  } Type;
  virtual const std::string& getType() const = 0;
  /// return contact company name
  virtual const std::string& getCompanyName() const = 0;
  /// return contact street address part 1
  virtual const std::string& getStreet1() const = 0;
  /// return contact street address part 2
  virtual const std::string& getStreet2() const = 0;
  /// return contact street address part 3
  virtual const std::string& getStreet3() const = 0;
  /// return contact state or province
  virtual const std::string& getProvince() const = 0;
  /// return contact postal code
  virtual const std::string& getPostalCode() const = 0;
  /// return contact city
  virtual const std::string& getCity() const = 0;
  /// return contact contry code
  virtual const std::string& getCountry() const = 0;

  virtual bool operator==(const Address &_other) const = 0;
  virtual bool operator!=(const Address &_other) const = 0;
};

class Contact : virtual public LibFred::Object {
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

  virtual unsigned int getAddressCount() const = 0;
  virtual const Address* getAddressByIdx(const unsigned int &_idx) const = 0;
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
          bool lock = false) const = 0;
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

typedef std::unique_ptr<Manager> ManagerPtr;

} // namespace Contact
} // namespace LibFred

#endif /* CONTACT_H_ */
