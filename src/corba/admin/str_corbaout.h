#ifndef STR_CORBAOUT_H_
#define STR_CORBAOUT_H_

#include "common.h"
#include "types/conversions.h"


inline const char* str_corbaout(const char *_value) {
  return _value;
}


template<class T>
inline const char* str_corbaout(const T &_value) {
  return Database::Conversion<T>::to_string(_value).c_str();
}


template<>
inline const char* str_corbaout(const std::string &_value) {
  return _value.c_str();
}


template<>
inline const char* str_corbaout(const ptime &_value) {
  return formatTime(_value, true).c_str();
}


template<>
inline const char* str_corbaout(const date &_value) {
  return formatDate(_value).c_str();
}


#endif /*STR_CORBAOUT_H_*/

