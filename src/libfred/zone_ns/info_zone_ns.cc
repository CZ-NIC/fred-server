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

#include "src/libfred/zone_ns/exceptions.hh"
#include "src/libfred/zone_ns/info_zone_ns.hh"

#include <algorithm>
#include <boost/algorithm/string.hpp>

namespace LibFred {
namespace ZoneNs {

namespace {

std::vector<boost::asio::ip::address> get_ip_addresses(const std::string& _ip_addresses)
{
    std::vector<boost::asio::ip::address> ip_addrs;

    if (_ip_addresses != "{}")
    {
        const std::string& addresses_without_braces = _ip_addresses.substr(1, _ip_addresses.size() - 2);

        std::vector<std::string> splitted_addresses;
        boost::split(splitted_addresses, addresses_without_braces, boost::is_any_of(","));

        for_each(splitted_addresses.begin(),
                splitted_addresses.end(),
                [&ip_addrs](const std::string& s){ip_addrs.push_back(boost::asio::ip::address::from_string(s));});
    }

    return ip_addrs;
}

} // namespace LibFred::ZoneNs::{anonymous}

InfoZoneNsData InfoZoneNs::exec(OperationContext& _ctx) const
{
    Database::Result result;
    try
    {
        result = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT id, zone, fqdn, addrs "
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
    else if (result.size() > 1)
    {
        throw std::runtime_error("Duplicity in database");
    }

    InfoZoneNsData info_zone_ns_data;

    info_zone_ns_data.id = static_cast<unsigned long long>(result[0]["id"]);
    info_zone_ns_data.zone_id = static_cast<unsigned long long>(result[0]["zone"]);
    info_zone_ns_data.nameserver_fqdn = static_cast<std::string>(result[0]["fqdn"]);
    info_zone_ns_data.nameserver_ip_addresses = get_ip_addresses(static_cast<std::string>(result[0]["addrs"]));

    return info_zone_ns_data;
}

} // namespace LibFred::ZoneNs
} // namespace LibFred
