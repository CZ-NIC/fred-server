#ifndef MAIL_FILTER_H_
#define MAIL_FILTER_H_

#include "db/query/base_filters.h"
#include "file_filter.h"

namespace Database {
namespace Filters {

class Mail : virtual public Compound {
public:
  virtual ~Mail() {
  }

  virtual Table& joinMailTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<int>& addType() = 0;
  virtual Value<std::string>& addHandle() = 0;
  virtual Interval<Database::DateTimeInterval>& addCreateTime() = 0;
  virtual Interval<Database::DateTimeInterval>& addModifyTime() = 0;
  virtual Value<int>& addStatus() = 0;
  virtual Value<int>& addAttempt() = 0;
  virtual Value<std::string>& addMessage() = 0;
  virtual File& addAttachment() = 0;
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar, const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class MailImpl : virtual public Mail {
public:
  MailImpl();
  virtual ~MailImpl();

  virtual Table& joinMailTable();
  virtual Value<Database::ID>& addId();
  virtual Value<int>& addType();
  virtual Value<std::string>& addHandle();
  virtual Interval<Database::DateTimeInterval>& addCreateTime();
  virtual Interval<Database::DateTimeInterval>& addModifyTime();
  virtual Value<int>& addStatus();
  virtual Value<int>& addAttempt();
  virtual Value<std::string>& addMessage();
  virtual File& addAttachment();
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Mail);
  }

};



}
}

#endif /*MAIL_FILTER_H_*/
