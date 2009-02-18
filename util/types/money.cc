#include "money.h"
#include "conversions.h"
#include <sstream>

namespace Database {

/*
 * string construct and getter
 */

void Money::from_string(const std::string& _value) {
    format(_value);
}


const std::string Money::to_string() const {
    return format();
}

const std::string
Money::format() const
{
    std::stringstream ss;
    ss << value_ / 100 << "." << std::setfill('0') << std::setw(2) << value_ % 100;
    return ss.str();
}

void
Money::format(std::string str)
{
    if (str.find('.') != std::string::npos) {
        value_ = atoll(str.substr(0, str.find('.')).c_str()) * 100 + 
            atoll(str.substr(str.find('.') + 1, std::string::npos).c_str());
    } else {
        value_ = atoll(str.c_str()) * 100;
    }
}

/*
 * output operator
 */

std::ostream& operator<<(std::ostream &_os, const Money& _v) {
    return _os << _v.to_string();
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

Money
Money::operator=(const Money &sec)
{
    value_ = sec.value_;
    return sec;
}

}

