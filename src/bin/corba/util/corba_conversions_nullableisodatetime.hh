/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef CORBA_CONVERSIONS_NULLABLEISODATETIME_HH_972092698C2F48D389B060B63B9DBA31
#define CORBA_CONVERSIONS_NULLABLEISODATETIME_HH_972092698C2F48D389B060B63B9DBA31

#include "src/bin/corba/NullableIsoDateTime.hh"
#include "util/db/nullable.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace CorbaConversion {
namespace Util {

Registry::NullableIsoDateTime_var
wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(
        const Nullable<boost::posix_time::ptime>& src);

} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
