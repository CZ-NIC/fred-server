#ifndef NSSET_FILTER_H_
#define NSSET_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "object_filter.h"
#include "contact_filter.h"

namespace Database {
namespace Filters {

class NSSet : virtual public Object {
public:
  virtual ~NSSet() {
  }

  virtual Table& joinNSSetTable() = 0;
  virtual Value<ID>& addId() = 0;
  virtual Value<std::string>& addHostFQDN() = 0;
  virtual Value<std::string>& addHostIP() = 0;
  virtual Contact& addTechContact() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
  
  static NSSet* create();
};

class NSSetImpl : public ObjectImpl, virtual public NSSet {
public:
  NSSetImpl();
  virtual ~NSSetImpl();

  virtual ObjectType getType() const {
    return TNSSET;
  }

  virtual Table& joinNSSetTable();
  virtual void _joinPolymorphicTables();

  virtual Value<ID>& addId();
  virtual Value<std::string>& addHandle();
  virtual Value<std::string>& addHostFQDN();
  virtual Value<std::string>& addHostIP();
  virtual Contact& addTechContact();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NSSet);
  }
};

class NSSetHistoryImpl : public ObjectHistoryImpl, virtual public NSSet {
public:
  NSSetHistoryImpl();
  virtual ~NSSetHistoryImpl();

  virtual ObjectType getType() const {
    return TNSSET;
  }

  virtual Table& joinNSSetTable();
  virtual void _joinPolymorphicTables();

  virtual Value<ID>& addId();
  virtual Value<std::string> &addHandle();
  virtual Value<std::string>& addHostFQDN();
  virtual Value<std::string>& addHostIP();
  virtual Contact& addTechContact();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NSSet);
  }
};

}
}

#endif /*NSSET_FILTER_H_*/
