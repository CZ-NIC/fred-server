#ifndef CORBA_UTIL_CORBA_CONVERSIONS_DATETIME_687431534534
#define CORBA_UTIL_CORBA_CONVERSIONS_DATETIME_687431534534

#include "src/bin/corba/DateTime.hh"
#include "src/util/db/nullable.hh"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/**
 *  @file
 *  conversions to and from CORBA mapping for date and time handling types
 */

namespace LibFred {
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


/**
 * Make nullable CORBA string from nullable boost date for generic CORBA valuetype wrapper
 * @param in is input nullable date
 * @return nullable string with date in text form YYYY-MM-DD allocated by CORBA::string_dupm or is_null if in is null or special
 */
Nullable<CORBA::String_var> wrap_nullable_date_to_nullable_corba_string(
        const Nullable<boost::gregorian::date>& in);


/**
 * Make nullable CORBA string from nullable boost ptime for generic CORBA valuetype wrapper
 * @param in is input nullable posix time
 * @return nullable string with date and time in text form YYYY-MM-DDTHH:MM:SS.fffffffff  is_null if in is null or special
 * dot with decimal fractions are optional, allocated by CORBA::string_dup
 */
Nullable<CORBA::String_var> wrap_nullable_ptime_to_nullable_corba_string(
        const Nullable<boost::posix_time::ptime>& in);


} // namespace LibFred::Corba
} // namespace LibFred

#endif
