#ifndef CORBA_UTIL_CORBA_CONVERSIONS_NULLABLE_TYPES_1234757910447
#define CORBA_UTIL_CORBA_CONVERSIONS_NULLABLE_TYPES_1234757910447

#include "corba/NullableTypes.hh"
#include "util/db/nullable.h"

/**
 *  @file
 *  conversions to and from CORBA mapping for nullable types
 */

namespace Corba {
    Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<long long>& in);
    Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<unsigned long long>& in);

    Nullable<unsigned long long> unwrap_nullable_ulonglong(const Registry::NullableULongLong * in);
}

#endif // end of #include guard
