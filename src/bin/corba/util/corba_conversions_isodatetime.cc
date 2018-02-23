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

#include "src/bin/corba/util/corba_conversions_isodatetime.hh"

#include "src/bin/corba/IsoDateTime.hh"
#include "src/bin/corba/NullableIsoDateTime.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/util/db/nullable.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>


namespace CorbaConversion {
namespace Util {

boost::posix_time::ptime
unwrap_IsoDateTime_to_boost_posix_time_ptime(
        const Registry::IsoDateTime src)
{
    return time_from_string(src.value.in());
}

void
unwrap_IsoDateTime_to_boost_posix_time_ptime(
        const Registry::IsoDateTime src,
        boost::posix_time::ptime& dst)
{
    dst = unwrap_IsoDateTime_to_boost_posix_time_ptime(src);
}

Nullable<boost::posix_time::ptime>
unwrap_NullableIsoDateTime_to_Nullable_boost_posix_time_ptime(
        const Registry::NullableIsoDateTime* src_ptr)
{
    if (src_ptr == NULL)
    {
        return Nullable<boost::posix_time::ptime>();
    }
    return unwrap_IsoDateTime_to_boost_posix_time_ptime(src_ptr->_boxed_in());
}

void
unwrap_NullableIsoDateTime_to_Nullable_boost_posix_time_ptime(
        const Registry::NullableIsoDateTime* src_ptr,
        Nullable<boost::posix_time::ptime>& dst)
{
    unwrap_NullableIsoDateTime_to_Nullable_boost_posix_time_ptime(src_ptr);
}

Registry::IsoDateTime
wrap_boost_posix_time_ptime_to_IsoDateTime(
        const boost::posix_time::ptime& src)
{
    if (src.is_special())
    {
        throw std::runtime_error("ptime is special");
    }
    Registry::IsoDateTime iso_date_time;
    iso_date_time.value = LibFred::Corba::wrap_string_to_corba_string(boost::posix_time::to_iso_extended_string(src) + "Z")._retn();
    return iso_date_time;
}

void
wrap_boost_posix_time_ptime_to_IsoDateTime(
        const boost::posix_time::ptime& src,
        Registry::IsoDateTime& dst)
{
    dst = wrap_boost_posix_time_ptime_to_IsoDateTime(src);
}

Registry::NullableIsoDateTime_var
        wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(
        const Nullable<boost::posix_time::ptime>& src)
{
    if (src.isnull()) {
        return Registry::NullableIsoDateTime_var();
    }

    Registry::NullableIsoDateTime_var result(new Registry::NullableIsoDateTime());
    wrap_boost_posix_time_ptime_to_IsoDateTime(src.get_value(), result.in()->_value());
    return result._retn();
}

} // namespace CorbaConversion::Util
} // namespace CorbaConversion
