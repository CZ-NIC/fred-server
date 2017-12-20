/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/util/types/date.hh"

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
 * date output operator
 */

std::ostream& operator<<(std::ostream &_os, const Date& _v) {
  return _os << _v.value; 
}

}

