#include "id.h"

namespace Database {

/*
 * output operator
 */

std::ostream& operator<<(std::ostream &_os, const ID& _v) {
  return _os << _v.value_;
}


std::istream& operator>>(std::istream &_is, ID& _v) {
  return _is >> _v.value_;
}


/*
 * comparison operators
 */

// bool operator<(const ID& _left, const ID& _right) {
//   return _left.value_ < _right.value_;
// }
// 
// 
// bool operator>(const ID& _left, const ID& _right) {
//   return _left.value_ > _right.value_;
// }
// 
// 
// bool operator<=(const ID& _left, const ID& _right) {
//   return !(_left > _right);
// }
// 
// 
// bool operator>=(const ID& _left, const ID& _right) {
//   return !(_left < _right);
// }
// 
// 
// bool operator==(const ID& _left, const ID& _right) {
//   return _left.value_ == _right.value_;
// }
// 
// 
// bool operator!=(const ID& _left, const ID& _right) {
//   return !(_left == _right);
// }

}

