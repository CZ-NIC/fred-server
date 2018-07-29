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
        int _expiration_period_min = 12,
        int _expiration_period_max = 120,
        int _ttl = 18000,
        const std::string& _hostmaster = "hostmaster@localhost",
        int _refresh = 10600,
        int _update_retr = 3600,
        int _expiry = 1209600,
        int _minimum = 7200,
        const std::string& _ns_fqdn = "localhost")
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
        const std::string& _nameserver_fqdn = "localhost",
        const std::string& _nameserver_ip_addresses = "")
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
        std::vector<std::string> addrs;
        boost::split(addrs, _nameserver_ip_addresses, boost::is_any_of(","));

        std::vector<boost::asio::ip::address> ip_addrs;
        for_each(addrs.begin(),
                addrs.end(),
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
