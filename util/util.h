#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include "types/data_types.h"

namespace Util {

template<class T>
std::string container2comma_list(const T &_cont)
{
    if (_cont.empty()) {
        return "";
    }

    std::stringstream tmp;
    typename T::const_iterator it = _cont.begin();
    tmp << *it;
    for (++it; it != _cont.end(); ++it) {
        tmp << ", " << *it;
    }
    return tmp.str();
}

inline std::string escape(std::string _input,
                   const std::string _what = "'\\",
                   const std::string _esc_char = "\\") {

  size_t i = 0;
  while ((i = _input.find_first_of(_what, i)) != _input.npos) {
    _input.replace(i, 1, _esc_char + _input[i]);
    i += _esc_char.length() + 1;
  }
  return _input;
}


inline std::string escape2(std::string _input) {
  const std::string _what = "'\\";
  const std::string _esc_char = "\\";
  size_t i = 0;
  while ((i = _input.find_first_of(_what, i)) != _input.npos) {
    _input.replace(i, 1, _esc_char + _input[i]);
    i += _esc_char.length() + 1;
  }
  return _input;
}

std::string make_svtrid(unsigned long long request_id);

}

#endif /*UTIL_H_*/
