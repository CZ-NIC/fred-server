#ifndef CONVERSIONS_H_
#define CONVERSIONS_H_

#include <string>
#include <cstdlib>

#include "datetime.h"
#include "date.h"
#include "id.h"
#include "money.h"

namespace Database {

/*
 * conversion structures for basic data types (POD)
 * to ability construct them from string and get them as a string
 */

template<class _Type>
inline std::string signed2string(const _Type& _value) {
  std::string buffer;
  const char *out = "0123456789";
  _Type copy = _value;

  do {
    buffer = out[std::abs(copy % 10)] + buffer;
    copy /= 10;
  }
  while (copy);

  if (_value < 0)
    buffer = "-" + buffer;

  return buffer;
}


template<class _Type>
inline std::string unsigned2string(const _Type& _value) {
  std::string buffer;
  const char *out = "0123456789";
  _Type copy = _value;

  do {
    buffer = out[copy % 10] + buffer;
    copy /= 10;
  }
  while (copy);

  return buffer;
}


template<class _Type> struct Conversion;

#define CONVERSION_DECLARATION(_type)                   \
template<>                                              \
struct Conversion<_type> {                              \
  static _type from_string(const std::string& _value);  \
  static std::string to_string(const _type& _value);    \
};

#define CONVERSION_DEFINITION(_type)                              \
_type Conversion<_type>::from_string(const std::string& _value) { \
  _type tmp;                                                      \
  tmp.from_string(_value);                                        \
  return tmp;                                                     \
}                                                                 \
                                                                  \
                                                                  \
std::string Conversion<_type>::to_string(const _type& _value) {   \
  return _value.to_string();                                      \
}

/*
 * conversion declarations for plain old data types
 */

CONVERSION_DECLARATION(short)
CONVERSION_DECLARATION(int)
CONVERSION_DECLARATION(long)
CONVERSION_DECLARATION(long long int)
CONVERSION_DECLARATION(unsigned int)
CONVERSION_DECLARATION(unsigned long)
CONVERSION_DECLARATION(unsigned long long)
CONVERSION_DECLARATION(bool)


/*
 * conversion declaration for user defined types
 * this type is required to have from_string() and to_string() methods
 */

CONVERSION_DECLARATION(DateTime)
CONVERSION_DECLARATION(Date)
CONVERSION_DECLARATION(ID)
CONVERSION_DECLARATION(Money)

/*
 * helper declaration for string from standard library
 */

CONVERSION_DECLARATION(std::string)


}

#endif /*CONVERSIIONS_H_*/

