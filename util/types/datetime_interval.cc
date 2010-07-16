#include "datetime_interval.h"

namespace Database {

/*
 * constructors 
 */

DateTimeInterval::DateTimeInterval() : value(ptime(neg_infin), ptime(pos_infin)), 
                                       special(NONE), offset() {
}


DateTimeInterval::DateTimeInterval(const DateTime& _start, const DateTime& _stop) : value(_start.get(), _stop.get()),
                                                                                    special(INTERVAL), offset() {
}


DateTimeInterval::DateTimeInterval(DateTimeIntervalSpecial _s,
                                   int _offset,
                                   const DateTime& _start,
                                   const DateTime& _stop) : value(_start.get(), _stop.get()),
      	                                                    special(_s),
                                                            offset(_offset) {

  if (special == DAY) {
    value = time_period(ptime(_start.get().date()), ptime(_start.get().date()) + days(1));
  }
}


/*
 * boost type manipulation compatibility 
 */

DateTimeInterval::DateTimeInterval(const time_period& _v) : value(_v), 
                                                            special(INTERVAL), offset() {
}


const time_period& DateTimeInterval::get() const {
  return value;
}


/*
 * string construct and getters 
 */

const std::string DateTimeInterval::to_string() const {
  return str();
}


const std::string DateTimeInterval::str() const {
  return to_simple_string(value);
}


/*
 * interval bounds getters 
 */

const DateTime DateTimeInterval::begin() const {
  return DateTime(value.begin());
}


const DateTime DateTimeInterval::last() const {
  return DateTime(value.last());
}


const DateTime DateTimeInterval::end() const {
  return DateTime(value.end());
}


/*
 * special value manipulation 
 */

bool DateTimeInterval::isSpecial() const {
  return (special != NONE);
}


DateTimeIntervalSpecial DateTimeInterval::getSpecial() const {
  return special;
}


int DateTimeInterval::getSpecialOffset() const {
  return offset;
}


/*
 * datetime interval output operator 
 */

std::ostream& operator<<(std::ostream &_os, const DateTimeInterval& _v) {
  return _os << _v.value;
}

}

