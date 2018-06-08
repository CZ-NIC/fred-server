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

#ifndef UTIL_HH_0636E71587A64901B725E8785299FC18
#define UTIL_HH_0636E71587A64901B725E8785299FC18

#include "src/libfred/opcontext.hh"
#include "test/setup/fixtures.hh"

namespace Test {

namespace{
constexpr int seconds_per_hour = 60 * 60;

constexpr int new_ttl_in_seconds = 6 * seconds_per_hour;
constexpr char new_hostmaster[] = "hostmaster_test@localhost";
constexpr int new_refresh_in_seconds  = 4 * seconds_per_hour;
constexpr int new_update_retr_in_seconds = 2 * seconds_per_hour;
constexpr int new_expiry_in_seconds = 1 * 7 * 24 * seconds_per_hour;
constexpr int new_minimum_in_seconds = 3 * seconds_per_hour;
constexpr char new_ns_fqdn[] = "localhost_test";
}

} // namespace Test

#endif
