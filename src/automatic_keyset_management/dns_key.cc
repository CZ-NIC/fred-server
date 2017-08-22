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

#include "src/automatic_keyset_management/dns_key.hh"

#include "src/automatic_keyset_management/impl/util.hh"

#include <string>

namespace Fred {
namespace AutomaticKeysetManagement {

std::string to_string(const DnsKey& dnskey)
{
    static const std::string delim = ", ";
    return "[flags: " + quote(dnskey.flags) + delim +
           "protocol: " + quote(dnskey.protocol) + delim +
           "algorithm: " + quote(dnskey.alg) + delim +
           "key: " + quote(dnskey.key) + "]";
}

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

