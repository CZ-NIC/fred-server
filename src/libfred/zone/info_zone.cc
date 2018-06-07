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
#include "src/libfred/zone/util.hh"


namespace LibFred {
namespace Zone {

InfoZoneData InfoZone::exec(OperationContext& _ctx) const
{
    Database::Result result;
    try
    {
        result = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT fqdn, ex_period_max, ex_period_min, val_period, dots_max, enum_zone, warning_letter "
                "FROM zone "
                "WHERE fqdn = LOWER($1::text)",
                // clang-format on
                Database::query_param_list(fqdn_));
    }
    catch (const std::exception&)
    {
        throw InfoZoneException();
    }

    if (result.size() != 1)
    {
        throw NonExistentZone();
    }

    InfoZoneData info_zone_data;
    const bool enum_zone = static_cast<bool>(result[0]["enum_zone"]);

    if (enum_zone)
    {
        EnumZone enum_zone;
        enum_zone.fqdn = static_cast<std::string>(result[0]["fqdn"]);
        enum_zone.expiration_period_max_in_months = static_cast<int>(result[0]["ex_period_max"]);
        enum_zone.expiration_period_min_in_months = static_cast<int>(result[0]["ex_period_min"]);
        enum_zone.validation_period_in_months = static_cast<int>(result[0]["val_period"]);
        enum_zone.dots_max = static_cast<int>(result[0]["dots_max"]);
        enum_zone.sending_warning_letter = static_cast<bool>(result[0]["warning_letter"]);
        info_zone_data = enum_zone;
    }
    else
    {
        NonEnumZone non_enum_zone;
        non_enum_zone.fqdn = static_cast<std::string>(result[0]["fqdn"]);
        non_enum_zone.expiration_period_max_in_months = static_cast<int>(result[0]["ex_period_max"]);
        non_enum_zone.expiration_period_min_in_months = static_cast<int>(result[0]["ex_period_min"]);
        non_enum_zone.dots_max = static_cast<int>(result[0]["dots_max"]);
        non_enum_zone.sending_warning_letter = static_cast<bool>(result[0]["warning_letter"]);
        info_zone_data = non_enum_zone;
    }

    return info_zone_data;
}

} // namespace LibFred::Zone
} // namespace LibFred
