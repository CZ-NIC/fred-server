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

#include "src/backend/admin/zone/zone.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/zone/create_zone.hh"
#include "src/libfred/zone/exceptions.hh"
#include "src/libfred/zone/zone_soa/create_zone_soa.hh"
#include "src/libfred/zone/zone_ns/create_zone_ns.hh"

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <vector>

namespace Admin {
namespace Zone {

void add_zone(
        const std::string& _fqdn,
        int _expiration_period_min,
        int _expiration_period_max,
        int _ttl,
        const std::string& _hostmaster,
        int _refresh,
        int _update_retr,
        int _expiry,
        int _minimum,
        const std::string& _ns_fqdn)
{
    LibFred::OperationContextCreator ctx;

    LibFred::Zone::CreateZone(_fqdn, _expiration_period_min, _expiration_period_max).exec(ctx);

    LibFred::Zone::CreateZoneSoa(_fqdn)
            .set_ttl(_ttl)
            .set_hostmaster(_hostmaster)
            .set_refresh(_refresh)
            .set_update_retr(_update_retr)
            .set_expiry(_expiry)
            .set_minimum(_minimum)
            .set_ns_fqdn(_ns_fqdn)
            .exec(ctx);
    ctx.commit_transaction();
}

void add_zone_ns(
        const std::string& _zone_fqdn,
        const std::string& _nameserver_fqdn,
        const std::string& _nameserver_ip_addresses)
{
    LibFred::OperationContextCreator ctx;

    if (_nameserver_ip_addresses.empty())
    {
        LibFred::Zone::CreateZoneNs(_zone_fqdn)
                .set_nameserver_fqdn(_nameserver_fqdn)
                .exec(ctx);
    }
    else
    {
        std::vector<std::string> splitted_addresses;
        boost::split(splitted_addresses, _nameserver_ip_addresses, boost::is_any_of(","));

        std::vector<boost::asio::ip::address> ip_addrs;
        std::for_each(splitted_addresses.begin(),
                splitted_addresses.end(),
                [&ip_addrs](const std::string& s) { ip_addrs.push_back(boost::asio::ip::address::from_string(s)); });

        LibFred::Zone::CreateZoneNs(_zone_fqdn)
                .set_nameserver_fqdn(_nameserver_fqdn)
                .set_nameserver_ip_addresses(ip_addrs)
                .exec(ctx);
    }
    ctx.commit_transaction();
}

} // namespace Admin::Zone
} // namespace Zone
