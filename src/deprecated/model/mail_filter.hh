#ifndef MAIL_FILTER_HH_7D939F1475434A96B6D35F5698A244A5
#define MAIL_FILTER_HH_7D939F1475434A96B6D35F5698A244A5

#include "src/util/db/query/base_filters.hh"
#include "src/deprecated/model/file_filter.hh"

namespace Database {
namespace Filters {

class Mail : virtual public Compound {
public:
  virtual ~Mail() {
  }

  virtual Table& joinMailTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<int>& addType() = 0;
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
