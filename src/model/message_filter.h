#ifndef _MESSAGE_FILTER_H_
#define _MESSAGE_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "db/query/base_filters.h"
#include "contact_filter.h"


namespace Database {
namespace Filters {

class Message : virtual public Compound {
public:
  virtual ~Message() {}
  virtual Table& joinMessageArchiveTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Interval<Database::DateTimeInterval>& addCrDate() = 0;
  virtual Interval<Database::DateTimeInterval>& addModDate() = 0;
  virtual Value<int>& addAttempt() = 0;
  virtual Value<int>& addStatus() = 0;
  virtual Value<int>& addCommType() = 0;
  virtual Value<int>& addMessageType() = 0;
  virtual Value<std::string>& addSmsPhoneNumber() = 0;
  virtual Value<std::string>& addLetterAddrName() = 0;
  virtual Contact& addMessageContact() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class MessageImpl : virtual public Message {
public:
    MessageImpl(bool _set_active = false);
  virtual ~MessageImpl();

  virtual Table& joinMessageArchiveTable();
  virtual Value<Database::ID>& addId();
  virtual Interval<Database::DateTimeInterval>& addCrDate();
  virtual Interval<Database::DateTimeInterval>& addModDate();
  virtual Value<int>& addAttempt();
  virtual Value<int>& addStatus();
  virtual Value<int>& addCommType();
  virtual Value<int>& addMessageType();
  virtual Value<std::string>& addSmsPhoneNumber();
  virtual Value<std::string>& addLetterAddrName();
  virtual Contact& addMessageContact();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
  }


};

}
}

#endif // _MESSAGE_FILTER_H_
