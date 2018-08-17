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

#include "src/libfred/zone/zone_ns/util.hh"
#include <algorithm>
#include <sstream>

namespace LibFred {
namespace Zone {

std::string ip_addresses_to_string(
        const std::vector<boost::asio::ip::address> _ip_addresses)
{
    std::ostringstream ns_addrs;

    ns_addrs << "{";
    if (_ip_addresses.size() > 0)
    {
        ns_addrs << _ip_addresses.at(0);
    }
    if (_ip_addresses.size() > 1)
    {
        std::for_each(_ip_addresses.begin() + 1,
                _ip_addresses.end(),
                [&ns_addrs](const boost::asio::ip::address& _ip) { ns_addrs << "," << _ip; });
    }
    ns_addrs << "}";

    return ns_addrs.str();
}

} // namespace LibFred::Zone
} // namespace LibFred
