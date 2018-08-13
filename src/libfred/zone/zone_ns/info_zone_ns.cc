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

#include "src/libfred/zone/zone_ns/exceptions.hh"
#include "src/libfred/zone/zone_ns/info_zone_ns.hh"

#include <algorithm>
#include <boost/algorithm/string.hpp>

namespace LibFred {
namespace Zone {

InfoZoneNsData InfoZoneNs::exec(OperationContext& _ctx) const
{
    Database::Result result;
    try
    {
        result = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT id, zone, fqdn, "
                "CASE WHEN array_length(addrs, 1) IS NULL THEN NULL "
                "ELSE unnest(addrs) "
                "END AS addrs "
                "FROM zone_ns "
                "WHERE id = $1::bigint",
                // clang-format on
                Database::query_param_list(id_));
    }
    catch (const std::exception&)
    {
        throw InfoZoneNsException();
    }

    if (result.size() < 1)
    {
        throw NonExistentZoneNs();
    }

    InfoZoneNsData info_zone_ns_data;

    info_zone_ns_data.id = static_cast<unsigned long long>(result[0]["id"]);
    info_zone_ns_data.zone_id = static_cast<unsigned long long>(result[0]["zone"]);
    info_zone_ns_data.nameserver_fqdn = static_cast<std::string>(result[0]["fqdn"]);

    std::vector<boost::asio::ip::address> ip_addrs;
    if (result.size() == 1)
    {
        const std::string& address = static_cast<std::string>(result[0]["addrs"]);
        if (!address.empty())
        {
            ip_addrs.push_back(boost::asio::ip::address::from_string(address));
        }
    }
    else
    {
        std::for_each (result.begin(), result.end(),
                [&ip_addrs](const auto& s){ ip_addrs.push_back(boost::asio::ip::address::from_string(s["addrs"])); });
    }

    info_zone_ns_data.nameserver_ip_addresses = ip_addrs;

    return info_zone_ns_data;
}

} // namespace LibFred::Zone
} // namespace LibFred
