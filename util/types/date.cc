#include "date.h"

namespace Database {

/*
 * constructors
 */
Date::Date() : value(not_a_date_time) {
}

Date::Date(unsigned _y, unsigned _m, unsigned _d) : value(_y, _m, _d) {
}

Date::Date(const std::string& _v) {
  from_string(_v);
}

Date::Date(DateTimeSpecial _s) {
  switch (_s) {
    case NOW:
      value = day_clock::local_day();
      break;
    case POS_INF:
      value = date(pos_infin);
      break;
    case NEG_INF:
      value = date(neg_infin);
      break;
    case NOW_UTC:
      value = day_clock::universal_day();
      break;
    default:
      value = date(not_a_date_time);
  }
}


/* 
 * boost type manipulation compatibility 
 */

Date::Date(const date& _v) : value(_v) {
}

const date& Date::get() const {
  return value;
}


/*
 * string construct and getters 
 */

void Date::from_string(const std::string& _v) {
  try {
    value = boost::gregorian::from_string(_v);
  }
  catch (...) {
    value = date(not_a_date_time);
  }
}


const std::string Date::to_string() const {
  return iso_str();
}


const std::string Date::iso_str() const {
  return to_iso_extended_string(value);
}


const std::string Date::str() const {
  return to_simple_string(value);
}


bool Date::is_special() const {
  return value.is_special();
}

int
Date::year() const
{
    return value.year();
}

int
Date::month() const
{
    return value.month();
}

int
Date::day() const
{
    return value.day();
}


/* 
 * date comparison operators 
 */

bool operator<(const Date &_left, const Date &_right) {
  return _left.value < _right.value;
}


bool operator>(const Date &_left, const Date &_right) {
  return _left.value > _right.value;
}


bool operator<=(const Date &_left, const Date &_right) {
  return !(_left > _right);
}


bool operator>=(const Date &_left, const Date &_right) {
  return !(_left < _right);
}


bool operator==(const Date &_left, const Date &_right) {
  return _left.value == _right.value;
}


bool operator!=(const Date &_left, const Date &_right) {
  return !(_left == _right);
}


/*
 * date math operator 
 */

Date operator+(const Date& _d, Days _days) {
  return Date(_d.value + days((Days::value_type)_days));
}


Date operator+(const Date& _d, Months _months) {
  return Date(_d.value + months((Months::value_type)_months));
}


Date operator+(const Date& _d, Years _years) {
  return Date(_d.value + years((Years::value_type)_years));
}


Date operator-(const Date& _d, Days _days) {
  return Date(_d.value - days((Days::value_type)_days));
}


Date operator-(const Date& _d, Months _months) {
  return Date(_d.value - months((Months::value_type)_months));
}


Date operator-(const Date& _d, Years _years) {
  return Date(_d.value - years((Years::value_type)_years));
}


/*
 * date output operator
 */

std::ostream& operator<<(std::ostream &_os, const Date& _v) {
  return _os << _v.value; 
}

}

