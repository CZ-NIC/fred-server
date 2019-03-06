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
#include "src/backend/automatic_keyset_management/dns_key.hh"

#include "src/backend/automatic_keyset_management/impl/util.hh"

#include <string>

namespace LibFred {
namespace AutomaticKeysetManagement {


bool operator<(const DnsKey& lhs, const DnsKey& rhs)
{
    if (lhs.key != rhs.key) {
        return lhs.key < rhs.key;
    }
    if (lhs.alg != rhs.alg) {
        return lhs.alg < rhs.alg;
    }
    if (lhs.protocol != rhs.protocol) {
        return lhs.protocol < rhs.protocol;
    }
    return lhs.flags < rhs.flags;
}


bool operator==(const DnsKey& lhs, const DnsKey& rhs)
{
    return !(lhs < rhs || rhs < lhs);
}


std::string to_string(const DnsKey& dnskey)
{
    static const std::string delim = ", ";
    return "[flags: " + Impl::quote(dnskey.flags) + delim +
           "protocol: " + Impl::quote(dnskey.protocol) + delim +
           "algorithm: " + Impl::quote(dnskey.alg) + delim +
           "key: " + Impl::quote(dnskey.key) + "]";
}


} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

