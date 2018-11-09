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

#ifndef DEFAULT_VALUES_HH_C52F6BD816B642DFB3E605253A871D76
#define DEFAULT_VALUES_HH_C52F6BD816B642DFB3E605253A871D76

namespace LibFred {
namespace Zone {

constexpr int seconds_per_hour = 60 * 60;

constexpr unsigned long default_ttl_in_seconds = 5 * seconds_per_hour;
constexpr unsigned long default_refresh_in_seconds  = 3 * seconds_per_hour;
constexpr unsigned long default_update_retr_in_seconds = seconds_per_hour;
constexpr unsigned long default_expiry_in_seconds = 2 * 7 * 24 * seconds_per_hour;
constexpr unsigned long default_minimum_in_seconds = 2 * seconds_per_hour;

} // namespace LibFred::Zone
} // namespace LibFred

#endif
