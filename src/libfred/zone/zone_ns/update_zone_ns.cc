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

#include "src/libfred/zone/info_zone.hh"
#include "src/libfred/zone/zone_ns/exceptions.hh"
#include "src/libfred/zone/zone_ns/update_zone_ns.hh"
#include "src/libfred/zone/zone_ns/util.hh"
#include "src/util/db/query_param.hh"
#include "src/util/util.hh"

#include <sstream>

namespace LibFred {
namespace Zone {

UpdateZoneNs::UpdateZoneNs(unsigned long long _id)
    : id_(_id)
{
}

UpdateZoneNs& UpdateZoneNs::set_zone_fqdn(const boost::optional<std::string>& _zone_fqdn)
{
    zone_fqdn_ = _zone_fqdn;
    return *this;
}

UpdateZoneNs& UpdateZoneNs::set_nameserver_fqdn(const boost::optional<std::string>& _nameserver_fqdn)
{
    nameserver_fqdn_ = _nameserver_fqdn;
    return *this;
}

UpdateZoneNs& UpdateZoneNs::set_nameserver_ip_addresses(
        const std::vector<boost::asio::ip::address>& _nameserver_ip_addresses)
{
    nameserver_ip_addresses_ = _nameserver_ip_addresses;
    return *this;
}

unsigned long long UpdateZoneNs::exec(OperationContext& _ctx) const
{
    const bool values_for_update_are_set = (zone_fqdn_ != boost::none
            || nameserver_fqdn_ != boost::none
            || nameserver_ip_addresses_ != boost::none);
    if (!values_for_update_are_set)
    {
        throw NoZoneNsData();
    }

    Database::QueryParams params;
    std::ostringstream object_sql;
    Util::HeadSeparator set_separator(" SET ", ", ");

    object_sql << "UPDATE zone_ns";
    if (zone_fqdn_ != boost::none)
    {
        const LibFred::Zone::InfoZoneData zone_info = LibFred::Zone::InfoZone(*zone_fqdn_).exec(_ctx);
        const unsigned long long zone_id = LibFred::Zone::get_zone_id(zone_info);

        params.push_back(zone_id);
        object_sql << set_separator.get() <<  "zone = $" << params.size() << "::bigint";
    }
    if (nameserver_fqdn_ != boost::none)
    {
        params.push_back(*nameserver_fqdn_);
        object_sql << set_separator.get() <<  "fqdn = LOWER($" << params.size() << "::text)";
    }
    if (nameserver_ip_addresses_ != boost::none)
    {
        params.push_back(ip_addresses_to_string(nameserver_ip_addresses_.get()));
        object_sql << set_separator.get() <<  "addrs = $" << params.size() << "::inet[]";
    }

    params.push_back(id_);
    object_sql << " WHERE id = $" << params.size() << "::bigint RETURNING id";

    try
    {
        const Database::Result update_result = _ctx.get_conn().exec_params(
                object_sql.str(),
                params);
        if (update_result.size() == 1)
        {
            const unsigned long long id = static_cast<unsigned long long>(update_result[0][0]);
            return id;
        }
        else if (update_result.size() < 1)
        {
            throw NonExistentZoneNs();
        }
        else
        {
            throw std::runtime_error("Duplicity in database");
        }
    }
    catch (const NonExistentZoneNs&)
    {
        throw;
    }
    catch (const std::exception&)
    {
        throw UpdateZoneNsException();
    }
}

} // namespace LibFred::Zone
} // namespace LibFred
