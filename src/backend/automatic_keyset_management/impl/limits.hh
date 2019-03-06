/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef LIMITS_HH_25607D12A25843EFB808C7838E54D85C
#define LIMITS_HH_25607D12A25843EFB808C7838E54D85C

#include <boost/static_assert.hpp>

#include <cstddef>

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

// allowed values from interval <min, max>
const unsigned min_number_of_dns_keys =  1;
const unsigned max_number_of_dns_keys = 10;

// maximal allowed keyset handle length
const std::size_t keyset_handle_length_max = 30;

// maximal allowed handle length is used for automatically managed keyset handle
const int automatically_managed_keyset_handle_length = keyset_handle_length_max;

// suffix minimum (prefix value is generated randomly)
const int automatically_managed_keyset_handle_suffix_length_min = 8;

// prefix minimum (prefix value is taken from configuration)
const int automatically_managed_keyset_handle_prefix_length_min = 2;
// prefix maximum (some room must be left for the generated suffix)
const int automatically_managed_keyset_handle_prefix_length_max = automatically_managed_keyset_handle_length - automatically_managed_keyset_handle_suffix_length_min;

// check that conditions can be met
BOOST_STATIC_ASSERT(automatically_managed_keyset_handle_prefix_length_min >= 0);
BOOST_STATIC_ASSERT(automatically_managed_keyset_handle_suffix_length_min >= 0);
BOOST_STATIC_ASSERT(automatically_managed_keyset_handle_prefix_length_min + automatically_managed_keyset_handle_suffix_length_min <= automatically_managed_keyset_handle_length);
BOOST_STATIC_ASSERT(automatically_managed_keyset_handle_prefix_length_max >= automatically_managed_keyset_handle_prefix_length_min);

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
