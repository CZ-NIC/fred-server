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
#include "src/libfred/zone/utils.hh"

namespace LibFred {
namespace Zone {

    CreateZone& CreateZone::set_enum_validation_period(const boost::gregorian::months _val_period)
    {
        val_period_ = _val_period;
        return *this;
    }

    CreateZone& CreateZone::set_sending_warning_letter(const bool _warning_letter)
    {
        warning_letter_ = _warning_letter;
        return *this;
    }

    unsigned long long CreateZone::exec(OperationContext& _ctx)
    {
        bool enum_zone = is_enum_zone(fqdn_);
        int dots_max;
        if (!enum_zone)
        {
            if (val_period_)
            {
                throw NotEnumZone();
            }
            dots_max = 1;
            val_period_ = boost::optional<boost::gregorian::months>(0);
        }

        if (enum_zone)
        {
            dots_max = 9;
            if (!val_period_)
            {
                val_period_ = boost::optional<boost::gregorian::months>(6);
            }
        }

        unsigned long long id;
        try
        {
            const Database::Result result = _ctx.get_conn().exec_params(
                "INSERT INTO zone (fqdn, ex_period_min, ex_period_max, val_period, dots_max, enum_zone, warning_letter) "
                    "VALUES ($1::text, $2::integer, $3::integer, $4::integer, $5::integer, $6::boolean, $7::boolean)"
                    "RETURNING id",
                Database::query_param_list(fqdn_)
                                          (ex_period_min_.number_of_months())
                                          (ex_period_max_.number_of_months())
                                          (val_period_->number_of_months())
                                          (dots_max)
                                          (enum_zone)
                                          (warning_letter_));
            id = result[0][0];
        }
        catch (const std::exception& e)
        {
            throw CreateZoneException();
        }
        return id;
    }

} // namespace Zone
} // namespace LibFred
