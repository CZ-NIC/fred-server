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
}

#endif // end of #include guard
