#ifndef CORBA_UTIL_CORBA_CONVERSIONS_DATETIME_687431534534
#define CORBA_UTIL_CORBA_CONVERSIONS_DATETIME_687431534534

#include "src/corba/DateTime.hh"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

/**
 *  @file
 *  conversions to and from CORBA mapping for date and time handling types
 */

namespace Corba {
    typedef boost::posix_time::ptime ptime;
    typedef boost::gregorian::date date;

    /**
     * In case @in is a special value (not a time, infinity etc.) all members of returned struct are set to zero.
     */
    ccReg::DateTimeType wrap_time(ptime in);

    /**
     * In case @in is a special value (not a date, infinity etc.) all members of returned struct are set to zero.
     */
    ccReg::DateType wrap_date(date in);


    /**
     * Make CORBA string from boost date
     * @param in is input date
     * @return string with date in text form YYYY-MM-DD allocated by CORBA::string_dup
     * @throws std::exception if in is not valid date
     */
    CORBA::String_var wrap_date_to_corba_string(boost::gregorian::date in);

    /**
     * Make CORBA string from boost ptime
     * @param in is input posix time
     * @return string with date and time in text form YYYY-MM-DDTHH:MM:SS.fffffffff
     * dot with decimal fractions are optional, allocated by CORBA::string_dup
     * @throws std::exception if in is not valid posix time
     */
    CORBA::String_var wrap_ptime_to_corba_string(boost::posix_time::ptime in);
}

#endif // end of #include guard
