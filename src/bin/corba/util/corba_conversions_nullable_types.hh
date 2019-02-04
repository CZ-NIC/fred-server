#ifndef CORBA_CONVERSIONS_NULLABLE_TYPES_HH_B09E37E52D7B440DA9714F6AE9769442
#define CORBA_CONVERSIONS_NULLABLE_TYPES_HH_B09E37E52D7B440DA9714F6AE9769442

#include "src/bin/corba/NullableTypes.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"

/**
 *  @file
 *  conversions to and from CORBA mapping for nullable types
 */

namespace CorbaConversion {
namespace Util {

/**
 * @throws std::out_of_range in case input is < 0
 */
Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<long long>& in);


Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<unsigned long long>& in);


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


} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
