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

#ifndef DNS_KEY_HH_1B0B00DF1F674F65BAAD0B35172C897D
#define DNS_KEY_HH_1B0B00DF1F674F65BAAD0B35172C897D

#include <set>
#include <string>

namespace Fred {
namespace AutomaticKeysetManagement {

struct DnsKey {
    unsigned short flags;
    unsigned short protocol;
    unsigned short alg;
    std::string key;

    DnsKey(
            unsigned short _flags,
            unsigned short _protocol,
            unsigned short _alg,
            const std::string& _key)
        : flags(_flags),
          protocol(_protocol),
          alg(_alg),
          key(_key)
    {
    }

    bool operator<(const DnsKey& rhs) const;

    bool operator==(const DnsKey& rhs) const;
};

typedef std::set<DnsKey> DnsKeys;

std::string to_string(const DnsKey& dnskey);

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
