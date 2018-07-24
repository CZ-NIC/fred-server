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
#include "src/bin/corba/util/corba_conversions_string.hh"

#include <boost/date_time/posix_time/posix_time.hpp>


namespace CorbaConversion {
namespace Util {

boost::posix_time::ptime
unwrap_IsoDateTime_to_boost_posix_time_ptime(
        const Registry::IsoDateTime src)
{
    return boost::posix_time::from_iso_extended_string(src.value.in());
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

} // namespace CorbaConversion::Util
} // namespace CorbaConversion
