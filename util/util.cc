#include "util.h"

namespace Util {

std::string escape(std::string _input,
                   const std::string _what,
                   const std::string _esc_char) {
  std::string::size_type i = 0;
  while ((i = _input.find_first_of(_what, i)) != _input.npos) {
    _input.replace(i, 1, _esc_char + _input[i]);
    i += _esc_char.length() + 1;
  }
  return _input;
}


}
