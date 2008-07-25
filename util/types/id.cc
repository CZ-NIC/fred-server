#include "id.h"
#include "conversions.h"

namespace Database {

/*
 * string construct and getter
 */

void ID::from_string(const std::string& _value) {
  value_ = Conversion<value_type>::from_string(_value);
}


const std::string ID::to_string() const {
  return Conversion<value_type>::to_string(value_);
}


/*
 * output operator
 */

std::ostream& operator<<(std::ostream &_os, const ID& _v) {
  return _os << _v.value_;
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

