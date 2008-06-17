#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include "db/data_types.h"

namespace Util {

std::string escape(std::string _input,
                   const std::string _what = "'\\",
                   const std::string _esc_char = "\\");

template<class TpIn, class TpOut> 
TpOut stream_cast(const TpIn& _value) {
  std::stringstream tmp;
  tmp << _value;
  TpOut ret;
  tmp >> ret;
  return ret;
}

template<class TpOut> 
TpOut stream_cast(const DBase::ID& _value) {
  std::stringstream tmp;
  tmp << _value;
  TpOut ret;
  tmp >> ret;
  return ret;
}

}

#endif /*UTIL_H_*/
