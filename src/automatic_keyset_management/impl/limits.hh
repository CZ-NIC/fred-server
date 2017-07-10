/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef LIMITS_H_7770C55BBBB94FAC8325481636FB0020
#define LIMITS_H_7770C55BBBB94FAC8325481636FB0020

#include <cstddef>

namespace Fred {
namespace AutomaticKeysetManagement {

// allowed values from interval <min, max>
const unsigned min_number_of_dns_keys =  1;
const unsigned max_number_of_dns_keys = 10;
const std::size_t keyset_handle_length_max = 30;

} // namespace Epp::AutomaticKeysetManagement
} // namespace Epp

#endif
