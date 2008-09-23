#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include "types/data_types.h"

namespace Util {

inline std::string escape(std::string _input,
                   const std::string _what = "'\\",
                   const std::string _esc_char = "'\\") {

  size_t i = 0;
  while ((i = _input.find_first_of(_what, i)) != _input.npos) {
    _input.replace(i, 1, _esc_char + _input[i]);
    i += _esc_char.length() + 1;
  }
  return _input;
}


template<class TpIn, class TpOut> 
TpOut stream_cast(const TpIn& _value) {
  std::stringstream tmp;
  tmp << _value;
  TpOut ret;
  tmp >> ret;
  return ret;
}

// template<class TpOut> 
// TpOut stream_cast(const ID& _value) {
//   std::stringstream tmp;
//   tmp << _value;
//   TpOut ret;
//   tmp >> ret;
//   return ret;
// }

}

#endif /*UTIL_H_*/
