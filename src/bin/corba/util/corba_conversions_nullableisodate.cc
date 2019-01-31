/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/bin/corba/util/corba_conversions_nullableisodate.hh"

#include "src/bin/corba/IsoDate.hh"
#include "src/bin/corba/NullableIsoDate.hh"
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
