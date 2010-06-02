#ifndef REGISTRAR_FILTER_H_
#define REGISTRAR_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "db/query/base_filters.h"
#include "zone_filter.h"

namespace Database {
namespace Filters {

class Registrar : virtual public Compound {
public:
  virtual ~Registrar() {
  }

  virtual Table& joinRegistrarTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<std::string>& addIco() = 0;
  virtual Value<std::string>& addDic() = 0;
  virtual Value<std::string>& addVarSymbol() = 0;
  virtual Value<bool>& addVat() = 0;
  virtual Value<std::string>& addHandle() = 0;
  virtual Value<std::string>& addName() = 0;
  virtual Value<std::string>& addOrganization() = 0;
  virtual Value<std::string>& addStreet() = 0;
  virtual Value<std::string>& addCity() = 0;
  virtual Value<std::string>& addStateOrProvince() = 0;
  virtual Value<std::string>& addPostalCode() = 0;
  virtual Value<std::string>& addCountryCode() = 0;
  virtual Value<std::string>& addTelephone() = 0;
  virtual Value<std::string>& addFax() = 0;
  virtual Value<std::string>& addEmail() = 0;
  virtual Value<std::string>& addUrl() = 0;
  virtual Value<std::string>& addZoneFqdn() = 0;
  virtual Value<Database::ID>& addGroupId() = 0;


  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class RegistrarImpl : virtual public Registrar {
public:
  RegistrarImpl(bool _set_active = false);
  virtual ~RegistrarImpl();

  virtual Table& joinRegistrarTable();
  virtual Value<Database::ID>& addId();
  virtual Value<std::string>& addIco();
  virtual Value<std::string>& addDic();
  virtual Value<std::string>& addVarSymbol();
  virtual Value<bool>& addVat();
  virtual Value<std::string>& addHandle();
  virtual Value<std::string>& addName();
  virtual Value<std::string>& addOrganization();
  virtual Value<std::string>& addStreet();
  virtual Value<std::string>& addCity();
  virtual Value<std::string>& addStateOrProvince();
  virtual Value<std::string>& addPostalCode();
  virtual Value<std::string>& addCountryCode();
  virtual Value<std::string>& addTelephone();
  virtual Value<std::string>& addFax();
  virtual Value<std::string>& addEmail();
  virtual Value<std::string>& addUrl();
  virtual Value<std::string>& addZoneFqdn();
  virtual Value<Database::ID>& addGroupId();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Registrar);
  }


};

}
}

#endif /*REGISTRAR_FILTER_H_*/

