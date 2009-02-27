#ifndef MONEY_H_
#define MONEY_H_

#include <ostream>
#include <istream>
#include "config.h"

#ifdef HAVE_BOOST_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#endif

namespace Database {

/*
 * class representing currency type
 */

class Money {
public:
  typedef long long value_type;

  /* constructors */
  Money() : value_(0) {
  }

  Money(value_type _value) : value_(_value) {
  }

  Money(const int value): value_(value)
  { }
  Money(const unsigned int value): value_(value)
  { }
  Money(const long int value): value_(value)
  { }
  Money(const unsigned long int value): value_(value)
  { }
  Money(const char *value)
  {
      from_string(std::string(value));
  }
  Money(const std::string &value)
  {
      from_string(value);
  }

  Money(const Money &sec): value_(sec.value_)
  { }
  operator value_type() const {
    return value_;
  }

  /* string construct and getter */
  void from_string(const std::string& _value);
  const std::string to_string() const;
  const std::string format() const;
  void format(std::string);

  /* comparison operators */
  friend bool operator<(const Money& _left, const Money& _right);
  friend bool operator>(const Money& _left, const Money& _right);
  friend bool operator<=(const Money& _left, const Money& _right);
  friend bool operator>=(const Money& _left, const Money& _right);
  friend bool operator==(const Money& _left, const Money& _right);
  friend bool operator!=(const Money& _left, const Money& _right);
  friend Money operator+(const Money& _left, const Money& _right);
  friend Money operator-(const Money& _left, const Money& _right);
  Money operator=(const Money& sec);
  
  /* output operator */
  friend std::ostream& operator<<(std::ostream &_os, const Money& _v);
  friend std::istream& operator>>(std::istream &_is, Money& _v);

#ifdef HAVE_BOOST_SERIALIZATION
  /* boost serialization */
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value_);
  }
#endif

private:
  value_type value_;
};

}

#endif /*MONEY_H_*/

