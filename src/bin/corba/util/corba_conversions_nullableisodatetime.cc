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

#include "src/bin/corba/util/corba_conversions_nullableisodatetime.hh"

#include "src/bin/corba/util/corba_conversions_isodatetime.hh"
#include "src/bin/corba/NullableIsoDateTime.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "util/db/nullable.hh"

#include <boost/date_time/posix_time/posix_time.hpp>


namespace CorbaConversion {
namespace Util {

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
