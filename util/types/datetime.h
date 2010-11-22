#ifndef DATETIME_H_
#define DATETIME_H_

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#include "datetime_common.h"
#include "date.h"
#include "config.h"

#ifdef HAVE_BOOST_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#endif

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Database {

/*
 * represent datetime type
 * - actually wrapped boost library ptime datetime type
 */

/* date time implementation */
class DateTime {
public:
  /* constructors */
  DateTime();
  DateTime(const Date& _v); 
  DateTime(DateTimeSpecial _s);
  DateTime(const std::string& _v);

  /* boost type manipulation compatibility */
  DateTime(const ptime& _v);
  const ptime& get() const;
  operator ptime() const {
    return value;
  }

  /* string construct and getters */ 
  void from_string(const std::string& _v);
  const std::string to_string() const;
  const std::string iso_str() const;
  const std::string str() const; 
  Date date() const;

  bool is_special() const;

  /* datetime comparison operators */
  friend bool operator<(const DateTime &_left, const DateTime &_right);
  friend bool operator>(const DateTime &_left, const DateTime &_right);
  friend bool operator<=(const DateTime &_left, const DateTime &_right);
  friend bool operator>=(const DateTime &_left, const DateTime &_right);
  friend bool operator==(const DateTime &_left, const DateTime &_right);
  friend bool operator!=(const DateTime &_left, const DateTime &_right);

  /* datetime math operators */
  friend DateTime operator+(const DateTime& _d, const Days& _days);
  friend DateTime operator+(const DateTime& _d, const Months& _months);
  friend DateTime operator+(const DateTime& _d, const Years& _years);
  friend DateTime operator+(const DateTime& _d, const Hours& _hours);
  friend DateTime operator+(const DateTime& _d, const Minutes& _minutes);
  friend DateTime operator+(const DateTime& _d, const Seconds& _seconds);
  friend DateTime operator-(const DateTime& _d, const Days& _days);
  friend DateTime operator-(const DateTime& _d, const Months& _months);
  friend DateTime operator-(const DateTime& _d, const Years& _years);
  friend DateTime operator-(const DateTime& _d, const Hours& _hours);
  friend DateTime operator-(const DateTime& _d, const Minutes& _minutes);
  friend DateTime operator-(const DateTime& _d, const Seconds& _seconds);

  /* datetime ouptut operator */
  friend std::ostream& operator<<(std::ostream &_os, const DateTime& _v);

  DateTime operator=(const DateTime &sec);

#ifdef HAVE_BOOST_SERIALIZATION
  /* boost serialization */
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(value);
  }
#endif

  friend class DateTimeInterval;

private:
  ptime value;
};

}

#endif /*DATETIME_H_*/

