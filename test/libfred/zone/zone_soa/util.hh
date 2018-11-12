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

#ifndef UTIL_HH_AFC28FFA02DD4579986093F1A58881A6
#define UTIL_HH_AFC28FFA02DD4579986093F1A58881A6

#include "src/libfred/zone/zone_soa/info_zone_soa_data.hh"

#include <string>

namespace Test {

constexpr int seconds_per_hour = 60 * 60;

constexpr int new_ttl_in_seconds = 6 * seconds_per_hour;
const std::string new_hostmaster = "new_hostmaster@nic.cz";
constexpr int new_refresh_in_seconds  = 4 * seconds_per_hour;
constexpr int new_update_retr_in_seconds = 2 * seconds_per_hour;
constexpr int new_expiry_in_seconds = 1 * 7 * 24 * seconds_per_hour;
constexpr int new_minimum_in_seconds = 3 * seconds_per_hour;
const std::string new_ns_fqdn = "new.ns.nic.cz";

bool operator==(const ::LibFred::Zone::InfoZoneSoaData& _lhs, const ::LibFred::Zone::InfoZoneSoaData& _rhs);

} // namespace Test

#endif
