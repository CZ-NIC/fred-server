#ifndef DB_VALUE_H_
#define DB_VALUE_H_

#include <string>
#include "data_types.h"
#include "util.h"

namespace DBase {

class Value {
public:
  Value(const std::string& _value, bool _null) :
    value(_value), null(_null), quoted(false) {
    _init();
  }
  
  Value(const char* _value, bool _null) :
    value(std::string(_value)), null(_null), quoted(false) {
    _init();
  }
  
  Value(const std::string& _value) :
    value(_value), null(false), quoted(true) {
  }
  
  Value(const DBase::ID& _id) : value(Util::stream_cast<std::string>(_id)),
                                null(false),
                                quoted(false) {
      if (_id.value <= 0)
      null = true;
  }
  
  Value(const int _value) : value(Util::stream_cast<std::string>(_value)),
                            null(false),
                            quoted(false) {
  }
  
  Value(const DBase::Date& _value) : value(_value.str()),
                                     null(false),
                                     quoted(true) {
  }
  
  Value(const DBase::DateTime& _value) : value(_value.str()),
                                         null(false),
                                         quoted(true) {
  }

  bool isNull() const {
    return null;
  }
  
  std::string str() const {
    return value;
  }

  operator ID() const {
    return ID(atoll(value.c_str()));
  }
  
  operator Money() const {
    return Money(atol(value.c_str()));
  }
  
  operator Date() const {
    return Date(value);
  }
  
  operator DateTime() const {
    return DateTime(value);
  }
  
  operator ptime() const {
    return ptime(time_from_string(value));
  }

  template<class Tp> operator Null<Tp>() const {
    return Null<Tp>();
  }
  
  operator int() const {
    return atoi(value.c_str());
  }
  
  operator short() const {
    return (short)atoi(value.c_str());
  }
  
  operator long() const {
    return atol(value.c_str());
  }
  
  operator unsigned() const {
    return atol(value.c_str());
  }
  
  operator unsigned long() const {
    return atol(value.c_str());
  }
  
  operator long long() const {
    return atol(value.c_str());
  }
  
  operator unsigned long long() const {
    return atoll(value.c_str());
  }
  
  operator bool() const {
    return value == "t";
  }
  
  operator std::string() const {
    return value;
  }


  friend std::ostream& operator<<(std::ostream &_os, const Value& _v);
  friend bool operator==(const std::string& _str, const Value& _v);

private:
  void _init() {
  }

  std::string value;
  bool null;
  bool quoted;
};

}

#endif /*DB_VALUE_H_*/
