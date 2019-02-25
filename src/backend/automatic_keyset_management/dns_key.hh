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
#ifndef DNS_KEY_HH_1B0B00DF1F674F65BAAD0B35172C897D
#define DNS_KEY_HH_1B0B00DF1F674F65BAAD0B35172C897D

#include <set>
#include <string>

namespace Fred {
namespace Backend {
namespace AutomaticKeysetManagement {

struct DnsKey
{
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

    friend bool operator<(const DnsKey& _lhs, const DnsKey& _rhs);

    friend bool operator==(const DnsKey& _lhs, const DnsKey& _rhs);

    unsigned short flags;
    unsigned short protocol;
    unsigned short alg;
    std::string key;
};

std::string to_string(const DnsKey& dnskey);

} // namespace Fred::Backend::AutomaticKeysetManagement
} // namespace Fred::Backend
} // namespace Fred

#endif
