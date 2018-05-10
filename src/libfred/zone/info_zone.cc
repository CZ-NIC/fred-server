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

#include "src/libfred/zone/info_zone_data.hh"
#include "src/libfred/zone/info_zone.hh"
#include "src/libfred/zone/utils.hh"


namespace LibFred {
namespace Zone {

InfoZoneData InfoZone::exec(OperationContext& _ctx)
{
    Database::Result result;
    try
    {
        result = _ctx.get_conn().exec_params(
            "SELECT fqdn, ex_period_max, ex_period_min, val_period, dots_max, enum_zone, warning_letter FROM zone "
                "WHERE fqdn = $1::text",
            Database::query_param_list(fqdn_));
    }
    catch (const std::exception&)
    {
        throw InfoZoneException();
    }
    if (result.size() == 0)
    {
        throw NonExistentZone();
    }
    InfoZoneData info_zone_data;
    info_zone_data.fqdn = static_cast<std::string>(result[0]["fqdn"]);
    info_zone_data.ex_period_max = static_cast<int>(result[0]["ex_period_max"]);
    info_zone_data.ex_period_min = static_cast<int>(result[0]["ex_period_min"]);
    info_zone_data.val_period = static_cast<int>(result[0]["val_period"]);
    info_zone_data.dots_max = result[0]["dots_max"];
    info_zone_data.enum_zone = result[0]["enum_zone"];
    info_zone_data.warning_letter = result[0]["warning_letter"];
    return info_zone_data;
}

} // namespace Zone
} // namespace LibFred
