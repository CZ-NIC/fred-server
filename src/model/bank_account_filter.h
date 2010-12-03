#ifndef _BANK_ACCOUNT_FILTER_H_
#define _BANK_ACCOUNT_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "db/query/base_filters.h"
#include "contact_filter.h"


namespace Database {
namespace Filters {

class BankAccount : virtual public Compound {
public:
  virtual ~BankAccount() {}
  virtual Table& joinBankAccountTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<std::string>& addAccountNumber() = 0;
  virtual Value<std::string>& addAccountName() = 0;
  virtual Value<std::string>& addBankCode() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class BankAccountImpl : virtual public BankAccount {
public:
    BankAccountImpl(bool _set_active = false);
  virtual ~BankAccountImpl();

  virtual Table& joinBankAccountTable();
  virtual Value<Database::ID>& addId();
  virtual Value<std::string>& addAccountNumber();
  virtual Value<std::string>& addAccountName();
  virtual Value<std::string>& addBankCode();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BankAccount);
  }


};

}
}

#endif // _BANK_ACCOUNT_FILTER_H_
