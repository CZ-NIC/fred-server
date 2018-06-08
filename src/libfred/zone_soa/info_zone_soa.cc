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

#include "src/libfred/zone_soa/info_zone_soa.hh"
#include "src/libfred/zone_soa/exceptions.hh"

namespace LibFred {
namespace ZoneSoa {

InfoZoneSoaData InfoZoneSoa::exec(OperationContext& _ctx) const
{
    const Database::Result zone_exists = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT id FROM zone WHERE fqdn=LOWER($1::text)",
            // clang-format on
            Database::query_param_list(fqdn_));
    if (zone_exists.size() != 1)
    {
        throw NonExistentZone();
    }
    const unsigned long long zone_id = zone_exists[0][0];

    Database::Result result;
    try
    {
        result = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT zone, ttl, hostmaster, refresh, update_retr, expiry, minimum, ns_fqdn "
                "FROM zone_soa "
                "WHERE zone = $1::bigint",
                // clang-format on
                Database::query_param_list(zone_id));
    }
    catch (const std::exception&)
    {
        throw InfoZoneSoaException();
    }

    if (result.size() != 1)
    {
        throw NonExistentZoneSoa();
    }

    InfoZoneSoaData info_zone_soa_data;

    info_zone_soa_data.zone = static_cast<unsigned long long>(result[0]["zone"]);
    info_zone_soa_data.ttl = static_cast<int>(result[0]["ttl"]);
    info_zone_soa_data.hostmaster = static_cast<std::string>(result[0]["hostmaster"]);
    info_zone_soa_data.refresh = static_cast<unsigned long long>(result[0]["refresh"]);
    info_zone_soa_data.update_retr = static_cast<unsigned long long>(result[0]["update_retr"]);
    info_zone_soa_data.expiry = static_cast<unsigned long long>(result[0]["expiry"]);
    info_zone_soa_data.minimum = static_cast<unsigned long long>(result[0]["minimum"]);
    info_zone_soa_data.ns_fqdn = static_cast<std::string>(result[0]["ns_fqdn"]);

    return info_zone_soa_data;
}

} // namespace LibFred::ZoneSoa
} // namespace LibFred
