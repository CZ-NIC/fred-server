#ifndef STR_CORBAOUT_H_
#define STR_CORBAOUT_H_

#include "common.h"
#include "types/convert_str_pod.h"
#include "types/stringify.h"


inline std::string str_corbaout(const char *_value) {
  return _value;
}


template<class T>
inline std::string str_corbaout(const T &_value) {
  return stringify(_value);
}


template<>
inline std::string str_corbaout(const std::string &_value) {
  return _value;
}


template<>
inline std::string str_corbaout(const ptime &_value) {
  return formatTime(_value, true);
}


template<>
inline std::string str_corbaout(const date &_value) {
  return formatDate(_value);
}


#endif /*STR_CORBAOUT_H_*/

