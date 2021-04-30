/*
 * Copyright (C) 2018-2020  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/util/corba_conversions_nullableisodate.hh"

#include "corba/IsoDate.hh"
#include "corba/NullableIsoDate.hh"
#include "src/bin/corba/util/corba_conversions_isodate.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "util/db/nullable.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace CorbaConversion {
namespace Util {

Nullable<boost::gregorian::date>
unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr)
{
    if (src_ptr == nullptr)
    {
        return Nullable<boost::gregorian::date>();
    }
    return unwrap_IsoDate_to_boost_gregorian_date(src_ptr->_boxed_in());
}

void
unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr,
        Nullable<boost::gregorian::date>& dst)
{
    dst = unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(src_ptr);
}

boost::optional<boost::gregorian::date>
unwrap_NullableIsoDate_to_optional_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr)
{
    if (src_ptr == nullptr)
    {
        return boost::optional<boost::gregorian::date>();
    }
    return unwrap_IsoDate_to_boost_gregorian_date(src_ptr->_boxed_in());
}

void
unwrap_NullableIsoDate_to_optional_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr,
        boost::optional<boost::gregorian::date>& dst)
{
    dst = unwrap_NullableIsoDate_to_optional_boost_gregorian_date(src_ptr);
}

Registry::NullableIsoDate_var
wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(
        const Nullable<boost::gregorian::date>& src)
{
    if (src.isnull()) {
        return Registry::NullableIsoDate_var();
    }
    Registry::NullableIsoDate_var result(new Registry::NullableIsoDate());
    wrap_boost_gregorian_date_to_IsoDate(src.get_value(), result.in()->_value());
    return result._retn();
}

} // namespace CorbaConversion::Util
} // namespace CorbaConversion
