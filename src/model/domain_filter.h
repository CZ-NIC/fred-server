#ifndef DOMAIN_FILTER_H_
#define DOMAIN_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "object_filter.h"
#include "contact_filter.h"
#include "nsset_filter.h"

namespace DBase {
namespace Filters {

class Domain : virtual public Object {
public:
  virtual ~Domain() {
  }

  virtual Table& joinDomainTable() = 0;
  virtual Value<DBase::ID>& addId() = 0;
  virtual Value<DBase::ID>& addNSSetId() = 0;
  virtual Value<DBase::ID>& addZoneId() = 0;
  virtual Value<DBase::ID>& addRegistrantId() = 0;
  virtual Interval<DBase::DateInterval>& addExpirationDate() = 0;
  virtual Interval<DBase::DateInterval>& addOutZoneDate() = 0;
  virtual Interval<DBase::DateInterval>& addCancelDate() = 0;
  virtual Contact& addRegistrant() = 0;
  virtual Contact& addAdminContact() = 0;
  virtual Contact& addTempContact() = 0;
  virtual NSSet& addNSSet() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
};

class DomainImpl : public ObjectImpl, virtual public Domain {
public:
  DomainImpl();
  virtual ~DomainImpl();

  virtual ObjectType getType() {
    return TDOMAIN;
  }

  Table& joinDomainTable();
  virtual void _joinPolymorphicTables();

  virtual Value<DBase::ID>& addId();
  virtual Value<DBase::ID>& addNSSetId();
  virtual Value<DBase::ID>& addZoneId();
  virtual Value<DBase::ID>& addRegistrantId();
  virtual Interval<DBase::DateInterval>& addExpirationDate();
  virtual Interval<DBase::DateInterval>& addOutZoneDate();
  virtual Interval<DBase::DateInterval>& addCancelDate();
  virtual Contact& addRegistrant();
  virtual Contact& addAdminContact();
  virtual Contact& addTempContact();
  virtual NSSet& addNSSet();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Domain);
  }

private:
  Contact& _addDCMFilter(unsigned _role);
};

class DomainHistoryImpl : public ObjectHistoryImpl, virtual public Domain {
public:
  DomainHistoryImpl();
  virtual ~DomainHistoryImpl();

  virtual ObjectType getType() {
    return TDOMAIN;
  }

  Table& joinDomainTable();
  virtual void _joinPolymorphicTables();

  virtual Value<DBase::ID>& addId();
  virtual Value<DBase::ID>& addNSSetId();
  virtual Value<DBase::ID>& addZoneId();
  virtual Value<DBase::ID>& addRegistrantId();
  virtual Interval<DBase::DateInterval>& addExpirationDate();
  virtual Interval<DBase::DateInterval>& addOutZoneDate();
  virtual Interval<DBase::DateInterval>& addCancelDate();
  virtual Contact& addRegistrant();
  virtual Contact& addAdminContact();
  virtual Contact& addTempContact();
  virtual NSSet& addNSSet();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Domain);
  }

private:
  Contact& _addDCMFilter(unsigned _role);
};

}
}

#endif /*DOMAIN_FILTER_H_*/
