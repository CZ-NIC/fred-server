#ifndef OBJECT_REGISTRY_FILTER_H_
#define OBJECT_REGISTRY_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "db/base_filters.h"
#include "registrar_filter.h"
#include "object_state_filter.h"

#include "types/conversions.h"

namespace Database {
namespace Filters {

enum ObjectType {
  TUNKNOWN = 0,
  TCONTACT = 1,
  TNSSET = 2,
  TDOMAIN = 3
};


class ObjectRegistry : public Compound {
public:
  virtual ~ObjectRegistry() {
  }

  virtual ObjectType getType() const = 0;
  
  virtual Table& joinObjectRegistryTable() = 0;
  virtual Value<ObjectType>& addType() = 0;
  virtual Value<std::string>& addHandle() = 0;
  virtual Interval<Database::DateTimeInterval>& addCreateTime() = 0;
  virtual Interval<Database::DateTimeInterval>& addDeleteTime() = 0;
  virtual Registrar& addCreateRegistrar() = 0;
  virtual ObjectState& addState() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
                                         const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class ObjectRegistryImpl : virtual public ObjectRegistry {
public:
  ObjectRegistryImpl();
  virtual ~ObjectRegistryImpl();

  virtual ObjectType getType() const {
    return TUNKNOWN;
  }
  
  virtual Table& joinObjectRegistryTable();
  virtual Value<ObjectType>& addType();
  virtual Value<std::string>& addHandle();
  virtual Interval<Database::DateTimeInterval>& addCreateTime();
  virtual Interval<Database::DateTimeInterval>& addDeleteTime();
  virtual Registrar& addCreateRegistrar();
  virtual ObjectState& addState();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
                                         const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ObjectRegistry);
  }

};

}


CONVERSION_DECLARATION(Filters::ObjectType)

inline Filters::ObjectType Conversion<Filters::ObjectType>::from_string(const std::string& _value) {
  return (Filters::ObjectType)atoi(_value.c_str());
}

inline std::string Conversion<Filters::ObjectType>::to_string(const Filters::ObjectType& _value) {
  return signed2string((int)_value);
}

}

#endif /*OBJECT_REGISTRY_FILTER_H_*/
