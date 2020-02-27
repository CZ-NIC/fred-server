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
#ifndef CORBA_CONVERSIONS_ISODATETIME_HH_6C81DCD1D79F45E2AC3190723E2F6DE3
#define CORBA_CONVERSIONS_ISODATETIME_HH_6C81DCD1D79F45E2AC3190723E2F6DE3

#include "corba/IsoDateTime.hh"
#include "src/util/tz/local_timestamp.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace CorbaConversion {
namespace Util {

Tz::LocalTimestamp
unwrap_IsoDateTime_to_Tz_LocalTimestamp(
        const Registry::IsoDateTime src);

boost::posix_time::ptime
unwrap_IsoDateTime_to_boost_posix_time_ptime(
        const Registry::IsoDateTime src);

Registry::IsoDateTime
wrap_boost_posix_time_ptime_to_IsoDateTime(
        const boost::posix_time::ptime& src);

void
wrap_boost_posix_time_ptime_to_IsoDateTime(
        const boost::posix_time::ptime& src,
        Registry::IsoDateTime& dst);

} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
