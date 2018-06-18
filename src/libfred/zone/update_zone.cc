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

#include "src/libfred/zone/update_zone.hh"
#include "src/libfred/zone/util.hh"

namespace LibFred {
namespace Zone {

UpdateZone& UpdateZone::set_expiration_period_min_in_months(const int _expiration_period_min_in_months)
{
    expiration_period_min_in_months_ = _expiration_period_min_in_months;
    return *this;
}

UpdateZone& UpdateZone::set_expiration_period_max_in_months(const int _expiration_period_max_in_months)
{
    expiration_period_max_in_months_ = _expiration_period_max_in_months;
    return *this;
}

UpdateZone& UpdateZone::set_enum_validation_period_in_months(const int _enum_validation_period_in_months)
{
    enum_validation_period_in_months_ = _enum_validation_period_in_months;
    return *this;
}

UpdateZone& UpdateZone::set_sending_warning_letter(const bool _sending_warning_letter)
{
    sending_warning_letter_ = _sending_warning_letter;
    return *this;
}

unsigned long long UpdateZone::exec(OperationContext& _ctx) const
{
    const bool values_for_update_are_set = (expiration_period_min_in_months_ != boost::none
            || expiration_period_max_in_months_ != boost::none
            || enum_validation_period_in_months_ != boost::none
            || sending_warning_letter_ != boost::none);
    if (!values_for_update_are_set)
    {
        throw NoZoneData();
    }

    const bool enum_val_period_is_set = enum_validation_period_in_months_ != boost::none;
    if (enum_val_period_is_set && !is_enum_zone(fqdn_))
    {
        throw NotEnumZone();
    }

    Database::QueryParams params;
    std::ostringstream object_sql;
    Util::HeadSeparator set_separator(" SET ", ", ");

    object_sql << "UPDATE zone";
    if (expiration_period_min_in_months_ != boost::none)
    {
        params.push_back(*expiration_period_min_in_months_);
        object_sql << set_separator.get() <<  "ex_period_min = $" << params.size() << "::integer";
    }
    if (expiration_period_max_in_months_ != boost::none)
    {
        params.push_back(*expiration_period_max_in_months_);
        object_sql << set_separator.get() <<  "ex_period_max = $" << params.size() << "::integer";
    }
    if (enum_validation_period_in_months_ != boost::none)
    {
        params.push_back(*enum_validation_period_in_months_);
        object_sql << set_separator.get() <<  "val_period = $" << params.size() << "::integer";
    }
    if (sending_warning_letter_ != boost::none)
    {
        params.push_back(*sending_warning_letter_);
        object_sql << set_separator.get() <<  "warning_letter = $" << params.size() << "::bool";
    }

    params.push_back(fqdn_);
    object_sql << " WHERE fqdn = LOWER($" << params.size() << "::text) RETURNING id";

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
            throw NonExistentZone();
        }
        else
        {
            throw std::runtime_error("Duplicity in database");
        }
    }
    catch (const NonExistentZone&)
    {
        throw;
    }
    catch (const std::exception&)
    {
        throw UpdateZoneException();
    }
}

} // namespace LibFred::Zone
} // namespace LibFred
