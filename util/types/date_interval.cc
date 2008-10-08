#include "date_interval.h"

namespace Database {

DateInterval::DateInterval() : value(date(neg_infin), date(pos_infin)),
                               special(NONE),
      	                       offset(0) {
}


DateInterval::DateInterval(const Date& _start, const Date& _stop) : value(_start.value, _stop.value),
                                                                    special(INTERVAL),
                                                                    offset(0) {
}


DateInterval::DateInterval(DateTimeIntervalSpecial _s,
                           int _offset,
                           const Date& _start,
                           const Date& _stop) : value(_start.value, _stop.value),
                                                special(_s),
                                                offset(_offset) {

  if (special == DAY) {
    value = date_period(_start.get(), _start.get() + days(1));
  }
}


/* 
 * boost type manipulation compability 
 */ 

DateInterval::DateInterval(const date_period& _v) : value(_v),
                                      special(INTERVAL),
      				offset(0) {
}


const date_period& DateInterval::get() const {
  return value;
}


/*
 * string construct and getters 
 */

const std::string DateInterval::to_string() const {
  return str();
}


const std::string DateInterval::str() const {
  return to_simple_string(value);
}


/* 
 * interval bounds getters 
 */

const Date DateInterval::begin() const {
  return Date(value.begin());
}


const Date DateInterval::last() const {
  return Date(value.last());
}


const Date DateInterval::end() const {
  return Date(value.end());
}


/*
 * special value manipulation 
 */

bool DateInterval::isSpecial() const {
  return (special != NONE);
}


DateTimeIntervalSpecial DateInterval::getSpecial() const {
  return special;
}


int DateInterval::getSpecialOffset() const {
  return offset;
}


/*
 * date interval output operator 
 */

std::ostream& operator<<(std::ostream &_os, const DateInterval& _v) {
  return _os << _v.value;
}

}

