#ifndef OBJECT_FILTER_H_
#define OBJECT_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "object_registry_filter.h"
#include "registrar_filter.h"

namespace DBase {
namespace Filters {

class Object : virtual public ObjectRegistry {
public:
  virtual ~Object() {
  }

  virtual Table& joinObjectTable() = 0;
  virtual Interval<DBase::DateTimeInterval>& addTransferTime() = 0;
  virtual Interval<DBase::DateTimeInterval>& addUpdateTime() = 0;
  virtual Value<std::string>& addAuthInfo() = 0;
  virtual Value<DBase::ID>& addRegistrarId() = 0;
  virtual Value<DBase::ID>& addUpdateRegistrarId() = 0;
  virtual Registrar& addRegistrar() = 0;
  virtual Registrar& addUpdateRegistrar() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ObjectRegistry);
  }
};

class ObjectImpl : public ObjectRegistryImpl, virtual public Object {
public:
  ObjectImpl();
  virtual ~ObjectImpl();

  virtual Table& joinObjectTable();
  virtual void _joinPolymorphicTables();
  
  virtual Interval<DBase::DateTimeInterval>& addTransferTime();
  virtual Interval<DBase::DateTimeInterval>& addUpdateTime();
  virtual Value<std::string>& addAuthInfo();
  virtual Value<DBase::ID>& addRegistrarId();
  virtual Value<DBase::ID>& addUpdateRegistrarId();
  virtual Registrar& addRegistrar();
  virtual Registrar& addUpdateRegistrar();
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
};

class ObjectHistoryImpl : public ObjectRegistryImpl, virtual public Object {
public:
  ObjectHistoryImpl();
  virtual ~ObjectHistoryImpl();

  virtual Table& joinObjectTable();
  virtual void _joinPolymorphicTables();
  
  virtual Interval<DBase::DateTimeInterval>& addTransferTime();
  virtual Interval<DBase::DateTimeInterval> & addUpdateTime();
  virtual Value<std::string>& addAuthInfo();
  virtual Value<DBase::ID>& addRegistrarId();
  virtual Value<DBase::ID>& addUpdateRegistrarId();
  virtual Registrar& addRegistrar();
  virtual Registrar& addUpdateRegistrar();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
};

}
}

#endif /*OBJECT_FILTER_H_*/
