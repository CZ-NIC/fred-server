#ifndef REGISTRAR_FILTER_H_
#define REGISTRAR_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "db/base_filters.h"

namespace Database {
namespace Filters {

class Registrar : virtual public Compound {
public:
  virtual ~Registrar() {
  }

  virtual Table& joinRegistrarTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<std::string>& addHandle() = 0;
  virtual Value<std::string>& addName() = 0;
  virtual Value<std::string>& addOrganization() = 0;
  virtual Value<std::string>& addCity() = 0;
  virtual Value<std::string>& addEmail() = 0;
  virtual Value<std::string>& addCountry() = 0;

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
  virtual Value<std::string>& addHandle();
  virtual Value<std::string>& addName();
  virtual Value<std::string>& addOrganization();
  virtual Value<std::string>& addCity();
  virtual Value<std::string>& addEmail();
  virtual Value<std::string>& addCountry();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Registrar);
  }

};

}
}

#endif /*REGISTRAR_FILTER_H_*/

