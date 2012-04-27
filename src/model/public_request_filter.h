#ifndef PUBLIC_REQUEST_FILTER_H_
#define PUBLIC_REQUEST_FILTER_H_

#include "db/query/base_filters.h"
#include "object_filter.h"

namespace Database {
namespace Filters {

class PublicRequest : virtual public Compound {
public:
  virtual ~PublicRequest() {
  }

  virtual Table& joinRequestTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
//  virtual Value<Fred::Request::Type>& addType() = 0;
//  virtual Value<Fred::Request::Status>& addStatus() = 0;
  virtual Value<int>& addType() = 0;
  virtual Value<int>& addStatus() = 0;
  virtual Interval<DateTimeInterval>& addCreateTime() = 0;
  virtual Interval<DateTimeInterval>& addResolveTime() = 0;
  virtual Value<std::string>& addReason() = 0;
  virtual Value<std::string>& addEmailToAnswer() = 0;
  virtual Value<Database::ID>& addAnswerEmailId() = 0;
  virtual Value<Database::ID>& addRegistrarId() = 0;
  virtual Object& addObject() = 0;

  virtual Registrar& addRegistrar() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static PublicRequest* create();
};


class PublicRequestImpl : virtual public PublicRequest {
public:
  PublicRequestImpl();
  virtual ~PublicRequestImpl();

  virtual Table& joinRequestTable();
  virtual Value<Database::ID>& addId();
  virtual Value<int>& addType();
  virtual Value<int>& addStatus();
  virtual Interval<DateTimeInterval>& addCreateTime();
  virtual Interval<DateTimeInterval>& addResolveTime();
  virtual Value<std::string>& addReason();
  virtual Value<std::string>& addEmailToAnswer();
  virtual Value<Database::ID>& addAnswerEmailId();
  virtual Value<Database::ID>& addRegistrarId();
  virtual Object& addObject();
  virtual Registrar& addRegistrar();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PublicRequest);
  }

};

}
}

#endif /*PUBLIC_REQUEST_FILTER_H_*/
