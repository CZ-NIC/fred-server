#ifndef CORBA_UTIL_CORBA_CONVERSIONS_NULLABLE_TYPES_1234757910447
#define CORBA_UTIL_CORBA_CONVERSIONS_NULLABLE_TYPES_1234757910447

#include "src/bin/corba/NullableTypes.hh"
#include "src/util/db/nullable.hh"
#include "src/util/optional_value.hh"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/**
 *  @file
 *  conversions to and from CORBA mapping for nullable types
 */

namespace LibFred {
namespace Corba {
/**
 * @throws std::out_of_range in case input is < 0
 */
Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<long long>& in);


Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<unsigned long long>& in);


Registry::NullableDate* wrap_nullable_date(const Nullable<boost::gregorian::date>& in);


Registry::NullableDateTime* wrap_nullable_datetime(const Nullable<boost::posix_time::ptime>& in);


Registry::NullableDateTime_var wrap_optional_datetime(const Optional<boost::posix_time::ptime>& in);


Nullable<unsigned long long> unwrap_nullable_ulonglong(const Registry::NullableULongLong* in);


Optional<unsigned long long> unwrap_nullable_ulonglong_to_optional(const Registry::NullableULongLong* in);


Nullable<std::string> unwrap_nullable_string(const Registry::NullableString* in);


Optional<std::string> unwrap_nullable_string_to_optional(const Registry::NullableString* in);


/**
 * Make CORBA valuetype from underlying CORBA type
 */
template <class CORBA_VALUE_TYPE, class CORBA_TYPE>
typename CORBA_VALUE_TYPE::_var_type
wrap_nullable_corba_type_to_corba_valuetype(const Nullable<CORBA_TYPE>& in)
{
    if (in.isnull())
    {
        return 0;
    }

    return new CORBA_VALUE_TYPE(in.get_value());
}


} // namespace LibFred::Corba
} // namespace LibFred

#endif
