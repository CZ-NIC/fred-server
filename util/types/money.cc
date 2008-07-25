#include "money.h"
#include "conversions.h"

namespace Database {

/*
 * string construct and getter
 */

void Money::from_string(const std::string& _value) {
  value_ = Conversion<value_type>::from_string(_value);
}


const std::string Money::to_string() const {
  return Conversion<value_type>::to_string(value_);
}


/*
 * output operator
 */

std::ostream& operator<<(std::ostream &_os, const Money& _v) {
  return _os << _v.value_;
}


/*
 * comparison operators
 */

bool operator<(const Money& _left, const Money& _right) {
  return _left.value_ < _right.value_;
}


bool operator>(const Money& _left, const Money& _right) {
  return _left.value_ > _right.value_;
}


bool operator<=(const Money& _left, const Money& _right) {
  return !(_left > _right);
}

bool operator>=(const Money& _left, const Money& _right) {
  return !(_left < _right);
}


bool operator==(const Money& _left, const Money& _right) {
  return _left.value_ == _right.value_;
}


bool operator!=(const Money& _left, const Money& _right) {
  return !(_left == _right);
}


Money operator+(const Money& _left, const Money& _right) {
  return Money(_left.value_ + _right.value_);
}


Money operator-(const Money& _left, const Money& _right) {
  return Money(_left.value_ - _right.value_);
}

}

