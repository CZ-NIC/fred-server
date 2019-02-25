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
#ifndef UTIL_HH_AFA4EF87EFA8413CA377F7DD8B97DA23
#define UTIL_HH_AFA4EF87EFA8413CA377F7DD8B97DA23

#include "libfred/zone/zone_ns/info_zone_ns_data.hh"

namespace Test {

bool operator==(const ::LibFred::Zone::InfoZoneNsData& _lhs,
        const ::LibFred::Zone::InfoZoneNsData& _rhs);

} // namespace Test

#endif
