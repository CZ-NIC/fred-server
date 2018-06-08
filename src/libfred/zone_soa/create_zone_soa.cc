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

#include "src/libfred/zone_soa/create_zone_soa.hh"
#include "src/libfred/zone_soa/exceptions.hh"

namespace LibFred {
namespace ZoneSoa {

namespace {
constexpr int seconds_per_hour = 60 * 60;

constexpr int default_ttl_in_seconds = 5 * seconds_per_hour;
constexpr char default_hostmaster[] = "hostmaster@localhost";
constexpr int default_refresh_in_seconds  = 3 * seconds_per_hour;
constexpr int default_update_retr_in_seconds = seconds_per_hour;
constexpr int default_expiry_in_seconds = 2 * 7 * 24 * seconds_per_hour;
constexpr int default_minimum_in_seconds = 2 * seconds_per_hour;
constexpr char default_ns_fqdn[] = "localhost";
}

CreateZoneSoa::CreateZoneSoa(const std::string& _fqdn)
        : fqdn_(_fqdn),
          ttl_(default_ttl_in_seconds),
          hostmaster_(default_hostmaster),
          refresh_(default_refresh_in_seconds),
          update_retr_(default_update_retr_in_seconds),
          expiry_(default_expiry_in_seconds),
          minimum_(default_minimum_in_seconds),
          ns_fqdn_(default_ns_fqdn)
{
}

CreateZoneSoa& CreateZoneSoa::set_ttl(const int _ttl)
{
    ttl_ = _ttl;
    return *this;
}

CreateZoneSoa& CreateZoneSoa::set_hostmaster(const std::string& _hostmaster)
{
    hostmaster_ = _hostmaster;
    return *this;
}

CreateZoneSoa& CreateZoneSoa::set_refresh(const int _refresh)
{
    refresh_ = _refresh;
    return *this;
}

CreateZoneSoa& CreateZoneSoa::set_update_retr(const int _update_retr)
{
    update_retr_ = _update_retr;
    return *this;
}

CreateZoneSoa& CreateZoneSoa::set_expiry(const int _expiry)
{
    expiry_ = _expiry;
    return *this;
}

CreateZoneSoa& CreateZoneSoa::set_minimum(const int _minimum)
{
    minimum_ = _minimum;
    return *this;
}

CreateZoneSoa& CreateZoneSoa::set_ns_fqdn(const std::string& _ns_fqdn)
{
    ns_fqdn_ = _ns_fqdn;
    return *this;
}

unsigned long long CreateZoneSoa::exec(OperationContext& _ctx) const
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
    const unsigned long long zone_id = static_cast<unsigned long long>(zone_exists[0][0]);

    const Database::Result zone_soa_exists = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT zone FROM zone_soa WHERE zone=$1::bigint",
            // clang-format on
            Database::query_param_list(zone_id));
    if (zone_soa_exists.size() != 0)
    {
        throw AlreadyExistingZoneSoa();
    }

    try
    {
        const Database::Result create_result = _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO zone_soa (zone, ttl, hostmaster, refresh, update_retr, expiry, minimum, ns_fqdn) "
                "VALUES ($1::bigint, $2::integer, $3::text, $4::integer, $5::integer, $6::integer, $7::integer, $8::text) "
                "RETURNING zone",
                // clang-format on
                Database::query_param_list(zone_id)
                                        (ttl_)
                                        (hostmaster_)
                                        (refresh_)
                                        (update_retr_)
                                        (expiry_)
                                        (minimum_)
                                        (ns_fqdn_)
                );

        if (create_result.size() == 1)
        {
            const unsigned long long id = static_cast<unsigned long long>(create_result[0][0]);
            return id;
        }

    }
    catch (const std::exception& e)
    {
        throw CreateZoneSoaException();
    }
    throw CreateZoneSoaException();
}

} // namespace LibFred::ZoneSoa
} // namespace LibFred
