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

#ifndef INFO_ZONE_NS_DATA_HH_630870DB7F4141399329A97A85D26A5D
#define INFO_ZONE_NS_DATA_HH_630870DB7F4141399329A97A85D26A5D

#include <boost/asio/ip/address.hpp>
#include <string>
#include <vector>

namespace LibFred {
namespace ZoneNs {

struct InfoZoneNsData
{
    unsigned long long id;
    unsigned long long zone_id;
    std::string nameserver_fqdn;
    std::vector<boost::asio::ip::address> nameserver_ip_addresses;
};

} // namespace LibFred::ZoneNs
} // namespace LibFred

#endif
