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

#include "src/libfred/zone/create_zone.hh"
#include "src/libfred/zone/util.hh"

namespace LibFred {
namespace Zone {

CreateZone& CreateZone::set_enum_validation_period_in_months(const int _enum_validation_period_in_months)
{
    enum_validation_period_in_months_ = _enum_validation_period_in_months;
    return *this;
}

CreateZone& CreateZone::set_sending_warning_letter(const bool _sending_warning_letter)
{
    sending_warning_letter_ = _sending_warning_letter;
    return *this;
}

unsigned long long CreateZone::exec(OperationContext& _ctx) const
{
    const bool enum_zone = is_enum_zone(fqdn_);
    const bool enum_val_period_is_set = enum_validation_period_in_months_ != boost::none;
    if (!enum_zone && enum_val_period_is_set)
    {
        throw NotEnumZone();
    }

    const int dots_max = enum_zone ? 9 : 1;
    const int dummy_val_period_months_nonenum = 0;
    const int default_val_period_months_enum = 6;
    const int validation_period_in_months =
            enum_zone
                    ? enum_validation_period_in_months_.value_or(default_val_period_months_enum)
                    : dummy_val_period_months_nonenum;
    try
    {
        const Database::Result create_result = _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO zone (fqdn, ex_period_min, ex_period_max, val_period, dots_max, enum_zone, warning_letter) "
                "VALUES (LOWER($1::text), $2::integer, $3::integer, $4::integer, $5::integer, $6::boolean, $7::boolean) "
                "RETURNING id",
                // clang-format on
                Database::query_param_list(fqdn_)
                                          (expiration_period_min_in_months_)
                                          (expiration_period_max_in_months_)
                                          (validation_period_in_months)
                                          (dots_max)
                                          (enum_zone)
                                          (sending_warning_letter_));
        if (create_result.size() == 1)
        {
            const unsigned long long id = static_cast<unsigned long long>(create_result[0][0]);
            return id;
        }
    }
    catch (const std::exception&)
    {
        throw CreateZoneException();
    }
    throw CreateZoneException();
}

} // namespace LibFred::Zone
} // namespace LibFred
