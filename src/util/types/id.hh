#ifndef ID_H_
#define ID_H_

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
 * class represents id type
 */

class ID {
public:
  typedef unsigned long long value_type;
  /* constructors */
  ID() : value_(0) {
  }

  ID(value_type _value) : value_(_value) {
  }


  /* comparison operators */
//  friend bool operator<(const ID& _left, const ID& _right);
//  friend bool operator>(const ID& _left, const ID& _right);
//  friend bool operator<=(const ID& _left, const ID& _right);
//  friend bool operator>=(const ID& _left, const ID& _right);
//  friend bool operator==(const ID& _left, const ID& _right);
//  friend bool operator!=(const ID& _left, const ID& _right);

  /* output operator */
  friend std::ostream& operator<<(std::ostream &_os, const ID& _v);
  friend std::istream& operator>>(std::istream &_is, ID& _v);


  operator value_type() const {
    return value_;
  }

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

#endif /*ID_H_*/

