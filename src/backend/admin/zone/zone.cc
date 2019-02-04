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
#include "libfred/opcontext.hh"
#include "libfred/zone/create_zone.hh"
#include "libfred/zone/exceptions.hh"
#include "libfred/zone/zone_ns/create_zone_ns.hh"
#include "libfred/zone/zone_soa/create_zone_soa.hh"

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <vector>

namespace Admin {
namespace Zone {

void add_zone(const std::string& _fqdn,
        const unsigned short _expiration_period_min,
        const unsigned short _expiration_period_max,
        const std::string& _hostmaster,
        const std::string& _ns_fqdn,
        const boost::optional<unsigned long> _ttl,
        const boost::optional<unsigned long> _refresh,
        const boost::optional<unsigned long> _update_retr,
        const boost::optional<unsigned long> _expiry,
        const boost::optional<unsigned long> _minimum)
{
    LibFred::OperationContextCreator ctx;

    LibFred::Zone::CreateZone(_fqdn, _expiration_period_min, _expiration_period_max).exec(ctx);

    LibFred::Zone::CreateZoneSoa(_fqdn, _hostmaster, _ns_fqdn)
            .set_ttl(_ttl)
            .set_refresh(_refresh)
            .set_update_retr(_update_retr)
            .set_expiry(_expiry)
            .set_minimum(_minimum)
            .exec(ctx);

    ctx.commit_transaction();
}

void add_zone_ns(
        const std::string& _zone_fqdn,
        const std::string& _nameserver_fqdn,
        const std::vector<boost::asio::ip::address>& _nameserver_ip_addresses)
{
    LibFred::OperationContextCreator ctx;

    LibFred::Zone::CreateZoneNs(_zone_fqdn, _nameserver_fqdn)
            .set_nameserver_ip_addresses(_nameserver_ip_addresses)
            .exec(ctx);

    ctx.commit_transaction();
}

} // namespace Admin::Zone
} // namespace Zone
