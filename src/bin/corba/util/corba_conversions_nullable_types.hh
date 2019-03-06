/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
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
