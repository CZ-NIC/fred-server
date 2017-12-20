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

#include "src/util/types/datetime.hh"

namespace Database {

/*
 * constructors
 */
DateTime::DateTime() : value(not_a_date_time) {
}


DateTime::DateTime(const Date& _v) :
  value(ptime(_v.get())) {
}


DateTime::DateTime(DateTimeSpecial _s) {
  switch (_s) {
    case NOW:
      value = microsec_clock::local_time();
      break;
    case POS_INF:
      value = ptime(pos_infin);
      break;
    case NEG_INF:
      value = ptime(neg_infin);
      break;
    case NOW_UTC:
      value = microsec_clock::universal_time();
      break;
    default:
      value = ptime(not_a_date_time);
  }
}


DateTime::DateTime(const std::string& _v) {
  from_string(_v);
}


/*
 * boost type manipulation compatibility 
 */

DateTime::DateTime(const ptime& _v) :
  value(_v) {
}


const ptime& DateTime::get() const {
  return value;
}


void DateTime::from_string(const std::string& _v) {
  try {
    value = time_from_string(_v);
  }
  catch (...) {
    value = ptime(not_a_date_time);
  }
}


const std::string DateTime::to_string() const {
  return iso_str(); 
}


const std::string DateTime::iso_str() const {
  return to_iso_extended_string(value);
}


const std::string DateTime::str() const {
  return to_simple_string(value);
}

Date
DateTime::date() const
{
    return Date(value.date());
}


bool DateTime::is_special() const {
  return value.is_special();
}


/*
 * datetime comparison operators 
 */

bool operator<(const DateTime &_left, const DateTime &_right) {
  return _left.value < _right.value;
}


bool operator>(const DateTime &_left, const DateTime &_right) {
  return _left.value > _right.value;
}


bool operator<=(const DateTime &_left, const DateTime &_right) {
  return !(_left > _right);
}


bool operator>=(const DateTime &_left, const DateTime &_right) {
  return !(_left < _right);
}


bool operator==(const DateTime &_left, const DateTime &_right) {
  return _left.value == _right.value;
}


bool operator!=(const DateTime &_left, const DateTime &_right) {
  return !(_left == _right);
}

DateTime
DateTime::operator=(const DateTime &sec)
{
    value = sec.value;
    return sec;
}


/*
 * datetime output operator
 */

std::ostream& operator<<(std::ostream &_os, const DateTime& _v) {
	return _os << _v.str();
}

}

