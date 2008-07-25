#ifndef MONEY_H_
#define MONEY_H_

#include <ostream>
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

  operator value_type() const {
    return value_;
  }

  /* string construct and getter */
  void from_string(const std::string& _value);
  const std::string to_string() const;

  /* comparison operators */
  friend bool operator<(const Money& _left, const Money& _right);
  friend bool operator>(const Money& _left, const Money& _right);
  friend bool operator<=(const Money& _left, const Money& _right);
  friend bool operator>=(const Money& _left, const Money& _right);
  friend bool operator==(const Money& _left, const Money& _right);
  friend bool operator!=(const Money& _left, const Money& _right);
  friend Money operator+(const Money& _left, const Money& _right);
  friend Money operator-(const Money& _left, const Money& _right);
  
  /* output operator */
  friend std::ostream& operator<<(std::ostream &_os, const Money& _v);

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

