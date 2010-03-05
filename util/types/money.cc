#include "money.h"
#include <iomanip>
#include <sstream>
#include "convert_sql_base.h"
#include "convert_sql_db_types.h"

namespace Database {

// standard abs takes only long, not long long
#define ABS(x) (((x) < 0) ? (-x) : (x))

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
    ss << ((value_ < 0) ? "-" : "") << ABS(value_) / 100 << "."
        << std::setfill('0') << std::setw(2) << ABS(value_) % 100;
    return ss.str();
}

void
Money::format(std::string str)
{

    
    Money conv(SqlConvert<Money>::from(str));

    *this = conv;   
}

/*
 * output operator
 */

std::ostream& operator<<(std::ostream &_os, const Money& _v) {
    return _os << _v.to_string();
}

std::istream& operator>>(std::istream &_is, Money& _v) {
    std::string tmp;
    _is >> tmp;
    _v.from_string(tmp);
    return _is;
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

