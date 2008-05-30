#ifndef CONTACT_FILTER_H_
#define CONTACT_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "object_filter.h"

namespace DBase {
namespace Filters {

class Contact : virtual public Object {
public:
  virtual ~Contact() {
  }
  
  virtual Table& joinContactTable() = 0;
  virtual Value<DBase::ID>& addId() = 0;
  virtual Value<std::string>& addName() = 0;
  virtual Value<std::string>& addOrganization() = 0;
  virtual Value<std::string>& addCity() = 0;
  virtual Value<std::string>& addEmail() = 0;
  virtual Value<std::string>& addNotifyEmail() = 0;
  virtual Value<std::string>& addVat() = 0;
  virtual Value<std::string>& addSsn() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
};

class ContactImpl : public ObjectImpl, virtual public Contact {
public:
  ContactImpl();
  virtual ~ContactImpl();

  virtual ObjectType getType() const {
    return TCONTACT;
  }

  virtual Table& joinContactTable();
  virtual void _joinPolymorphicTables();

  virtual Value<DBase::ID>& addId();
  virtual Value<std::string>& addName();
  virtual Value<std::string>& addOrganization();
  virtual Value<std::string>& addCity();
  virtual Value<std::string>& addEmail();
  virtual Value<std::string>& addNotifyEmail();
  virtual Value<std::string>& addVat();
  virtual Value<std::string>& addSsn();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Contact);
  }
};

class ContactHistoryImpl : public ObjectHistoryImpl, virtual public Contact {
public:
  ContactHistoryImpl();
  ~ContactHistoryImpl();

  virtual ObjectType getType() const {
    return TCONTACT;
  }

  virtual Table& joinContactTable();
  virtual void _joinPolymorphicTables();

  virtual Value<DBase::ID>& addId();
  virtual Value<std::string>& addName();
  virtual Value<std::string>& addOrganization();
  virtual Value<std::string>& addCity();
  virtual Value<std::string>& addEmail();
  virtual Value<std::string>& addNotifyEmail();
  virtual Value<std::string>& addVat();
  virtual Value<std::string>& addSsn();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Contact);
  }
};

}
}

#endif /*CONTACT_FILTER_H_*/
