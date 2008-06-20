#ifndef DATE_TYPES_H_
#define DATE_TYPES_H_

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace DBase {

enum DateSpecial {
  NOW,
  NEGINF,
  POSINF};

/*enum DateIntervalSpecial {
 TILL_NOW,
 SINCE_NOW,
 LAST_WEEK,
 THIS_WEEK,
 LAST_MONTH,
 THIS_MONTH,
 LAST_YEAR,
 THIS_YEAR};*/

enum DateTimeIntervalSpecial {
  NONE,
  DAY,
  INTERVAL,
  LAST_HOUR,
  LAST_DAY,
  LAST_WEEK,
  LAST_MONTH,
  LAST_YEAR,
  PAST_HOUR,
  PAST_DAY,
  PAST_WEEK,
  PAST_MONTH,
  PAST_YEAR};

template<class Tp> Tp& SQLNULL(Tp& _obj) {
  _obj.null();
  return _obj;
}

class NullStrRep {
public:
  static std::ostream& toStr(std::ostream& _os) {
    return _os << "NULL";
  }
};

template<class Tp, class Rep =NullStrRep> class Null {
public:
  Null(const Tp& _value, bool _null = false) :
    is_null(_null), t_value(_value) {
  }
  Null() :
    is_null(true) {
  }
  Null(const Null<Tp>& _src) {
    t_value = _src.t_value;
    is_null = _src.is_null;
  }
  virtual ~Null() {
  }
  friend std::ostream& operator<<(std::ostream& _os, const Null<Tp>& _v) {
    if (_v.is_null) {
      return Rep::toStr(_os);
    } else {
      return _os << _v.t_value;
    }
  }
  virtual bool isNull() const {
    return is_null;
  }
  virtual void null() {
    is_null = true;
  }
  virtual void setNull(bool _n) {
    is_null = _n;
  }
  virtual void setValue(const Tp& _v) {
    t_value = _v;
  }
  virtual const Tp& getValue() const {
    return t_value;
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(is_null);
    _ar & BOOST_SERIALIZATION_NVP(t_value);
  }

protected:
  bool is_null;
  Tp t_value;
};

template<> class Null<void, NullStrRep> {
public:
  Null(bool _null = true) :
    is_null(_null) {
  }
  virtual ~Null() {
  }
  friend std::ostream& operator<<(std::ostream& _os, const Null<void>& _v) {
    return NullStrRep::toStr(_os);
  }
  virtual bool isNull() {
    return is_null;
  }
  virtual void null() {
    is_null = true;
  }

protected:
  bool is_null;
};

struct Days {
  Days(unsigned _v) :
    value(_v) {
  }
  unsigned value;
};

struct Months {
  Months(unsigned _v) :
    value(_v) {
  }
  unsigned value;
};

struct Years {
  Years(unsigned _v) :
    value(_v) {
  }
  unsigned value;
};

struct Hours {
  Hours(unsigned _v) :
    value(_v) {
  }
  unsigned value;
};

struct Minutes {
  Minutes(unsigned _v) :
    value(_v) {
  }
  unsigned value;
};

struct Seconds {
  Seconds(unsigned _v) :
    value(_v) {
  }
  unsigned value;
};

class Date {
public:
  Date() {
  }
  Date(unsigned _y, unsigned _m, unsigned _d) :
    value(_y, _m, _d) {
  }
  Date(const std::string& _v) {
    try {
      value = from_string(_v);
    }
    catch (...) {
      value = date(not_a_date_time);
    }
  }
  Date(const date& _v) :
    value(_v) {
  }
  Date(DateSpecial _s) {
    if (_s == NOW) {
      value = day_clock::local_day();
    }
    if (_s == POSINF) {
      value = date(pos_infin);
    }
    if (_s == NEGINF) {
      value = date(neg_infin);
    }
  }

  const date& get() const {
    return value;
  }

  const std::string iso_str() const {
    return to_iso_extended_string(value);
  }
  const std::string str() const {
    return to_simple_string(value);
  }
  bool is_special() const {
    return value.is_special();
  }
  operator date() const {
    return value;
  }

  friend std::ostream& operator<<(std::ostream &_os, const Date& _v);
  friend Date operator+(const Date& _d, Days _days);
  friend Date operator+(const Date& _d, Months _months);
  friend Date operator+(const Date& _d, Years _years);
  friend Date operator-(const Date& _d, Days _days);
  friend Date operator-(const Date& _d, Months _months);
  friend Date operator-(const Date& _d, Years _years);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value);
  }

private:
  date value;
};

class Time {
public:
  Time() {
  }
  Time(unsigned _h, unsigned _m, unsigned _s = 0) :
    value(_h, _m, _s) {
  }
  Time(const time_duration& _td) :
    value(_td) {
  }

  const time_duration& get() const {
    return value;
  }

  const std::string str() const {
    return to_simple_string(value);
  }

  friend std::ostream& operator<<(std::ostream &_os, const Time& _v);

private:
  time_duration value;
};

class DateTime {
public:
  DateTime() :
    value(not_a_date_time) {
  }
  DateTime(const ptime& _v) :
    value(_v) {
  }
  DateTime(const Date& _v) :
    value(ptime(_v.get())) {
  }
  DateTime(const std::string& _v) {
    try {
      value = time_from_string(_v);
    }
    catch (...) {
      value = ptime(not_a_date_time);
    }
  }
  //DateTime(special_values _sv) : value(_sv) { }

  const ptime& get() const {
    return value;
  }

  const std::string iso_str() const {
    return to_iso_extended_string(value);
  }
  const std::string str() const {
    return to_simple_string(value);
  }
  bool is_special() const {
    return value.is_special();
  }
  operator ptime() const {
    return value;
  }

  friend bool operator<=(const DateTime &_left, const DateTime &_right);
  friend std::ostream& operator<<(std::ostream &_os, const DateTime& _v);
  friend DateTime operator+(const DateTime& _d, Time _time);
  friend DateTime operator-(const DateTime& _d, Time _time);
  friend DateTime operator+(const DateTime& _d, Days _days);
  friend DateTime operator+(const DateTime& _d, Months _months);
  friend DateTime operator+(const DateTime& _d, Years _years);
  friend DateTime operator+(const DateTime& _d, Hours _hours);
  friend DateTime operator+(const DateTime& _d, Minutes _minutes);
  friend DateTime operator+(const DateTime& _d, Seconds _seconds);
  friend DateTime operator-(const DateTime& _d, Days _days);
  friend DateTime operator-(const DateTime& _d, Months _months);
  friend DateTime operator-(const DateTime& _d, Years _years);
  friend DateTime operator-(const DateTime& _d, Hours _hours);
  friend DateTime operator-(const DateTime& _d, Minutes _minutes);
  friend DateTime operator-(const DateTime& _d, Seconds _seconds);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value);
  }

private:
  ptime value;
};

class DateInterval {
public:
  DateInterval() :
    value(date(neg_infin), date(pos_infin)), special(NONE), offset(0) {
  }
  DateInterval(const date_period& _v) :
    value(_v), special(NONE), offset(0) {
  }
  DateInterval(const Date& _start, const Date& _stop) :
    value(_start.get(), _stop.get()), special(NONE), offset(0) {
  }
  DateInterval(DateTimeIntervalSpecial _s, int _offset = 0,
      const Date& _start = Date(date(neg_infin)), const Date& _stop = Date(date(pos_infin))) :
    value(_start.get(), _stop.get()), special(_s), offset(_offset) {
    if (special == DAY) {
      value = date_period(_start.get(), _start.get() + days(1));
    }
  }
  const date_period& get() const {
    return value;
  }
  const std::string str() const {
    return to_simple_string(value);
  }
  const Date begin() const {
    return Date(value.begin());
  }
  const Date last() const {
    return Date(value.last());
  }
  const Date end() const {
    return Date(value.end());
  }
  bool isSpecial() const {
    return (special != NONE);
  }
  DateTimeIntervalSpecial getSpecial() const {
    return special;
  }
  int getSpecialOffset() const {
    return offset;
  }

  friend std::ostream& operator<<(std::ostream &_os, const DateInterval& _v);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value);
    _ar & BOOST_SERIALIZATION_NVP(special);
    _ar & BOOST_SERIALIZATION_NVP(offset);
  }

private:
  date_period value;
  DateTimeIntervalSpecial special;
  int offset;
};

class DateTimeInterval {
public:
  DateTimeInterval() :
    value(ptime(neg_infin), ptime(pos_infin)), special(NONE) {
  }
  DateTimeInterval(const time_period& _v) :
    value(_v), special(NONE) {
  }
  DateTimeInterval(const DateTime& _start, const DateTime& _stop) :
    value(_start.get(), _stop.get()), special(NONE) {
  }
  //DateTimeInterval(DateTimeIntervalSpecial _s, int _offset = 0) :
  //	value(ptime(neg_infin), ptime(pos_infin)), special(_s), offset(_offset) {
  //}
  DateTimeInterval(DateTimeIntervalSpecial _s, int _offset = 0,
      const DateTime& _start = DateTime(ptime(neg_infin)), const DateTime& _stop = DateTime(ptime(pos_infin))) :
    value(_start.get(), _stop.get()), special(_s), offset(_offset) {
    if (special == DAY) {
      value = time_period(ptime(_start.get().date()), ptime(_start.get().date()) + days(1));
    }
  }
  const time_period& get() const {
    return value;
  }
  const std::string str() const {
    return to_simple_string(value);
  }
  const DateTime begin() const {
    return DateTime(value.begin());
  }
  const DateTime last() const {
    return DateTime(value.last());
  }
  const DateTime end() const {
    return DateTime(value.end());
  }
  bool isSpecial() const {
    return (special != NONE);
  }
  DateTimeIntervalSpecial getSpecial() const {
    return special;
  }
  int getSpecialOffset() const {
    return offset;
  }
  friend std::ostream
      & operator<<(std::ostream &_os, const DateTimeInterval& _v);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value);
    _ar & BOOST_SERIALIZATION_NVP(special);
    _ar & BOOST_SERIALIZATION_NVP(offset);
  }

private:
  time_period value;
  DateTimeIntervalSpecial special;
  int offset;
};

class ID {
public:
  ID() :
    value(0) {
  }
  ID(unsigned long long _value) :
    value(_value) {
  }
  friend std::ostream& operator<<(std::ostream &_os, const ID& _v);
  friend bool operator<(const ID& _left, const ID& _right);
  operator unsigned long long() const {
    return value;
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value);
  }

  unsigned long long value;
};

class Money {
public:
  Money() :
    value(0) {
  }
  Money(long long _value) :
    value(_value) {
  }
  friend std::ostream& operator<<(std::ostream &_os, const Money& _v);
  friend bool operator<(const Money& _left, const Money& _right);
  operator unsigned long long() const {
    return value;
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value);
  }

  long long value;
};

// Test class for escaping and quoting notification
class String : public std::string {
public:
  String(const char* _str) :
    std::string(_str), escape(false), quote(false) {
  }
  String(const std::string& _str) :
    std::string(_str), escape(false), quote(false) {
  }
  void setEscape() {
    escape = true;
  }
  void setQuote() {
    quote = true;
  }
  bool getEscape() {
    return escape;
  }
  bool getQuote() {
    return quote;
  }

protected:
  bool escape;
  bool quote;
};

}

#endif /*DATE_TYPES_H_*/
