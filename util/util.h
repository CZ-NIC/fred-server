#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include "types/data_types.h"

namespace Util {

inline std::string escape(std::string _input,
                   const std::string _what = "'\\",
                   const std::string _esc_char = "\\") {

  size_t i = 0;
  size_t c = 0;
  while ((i = _input.find_first_of(_what, i)) != _input.npos) {
    c = _what.find(_input[i], 0);
    _input.replace(i, 1, _esc_char, c, 1);
    _input.replace(i+1, 0, _input, i, 1);
    i += 2;
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
