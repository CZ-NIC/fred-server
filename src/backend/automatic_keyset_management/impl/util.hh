/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef UTIL_HH_4675308061E4499E93103CD4A942696F
#define UTIL_HH_4675308061E4499E93103CD4A942696F

#include "libfred/registrable_object/keyset/keyset_dns_key.hh"

#include <string>

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

std::string quote(const std::string& str);

std::string quote(int value);

std::string to_string(const LibFred::DnsKey& dnskey);

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
