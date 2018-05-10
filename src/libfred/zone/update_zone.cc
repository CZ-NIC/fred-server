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
#include "src/libfred/zone/utils.hh"

namespace LibFred {
namespace Zone {

UpdateZone& UpdateZone::set_ex_period_min(const int _ex_period_min)
{
    ex_period_min_ = _ex_period_min;
    return *this;
}

UpdateZone& UpdateZone::set_ex_period_max(const int _ex_period_max)
{
    ex_period_max_ = _ex_period_max;
    return *this;
}

UpdateZone& UpdateZone::set_enum_validation_period(const int _val_period)
{
    val_period_ = _val_period;
    return *this;
}

UpdateZone& UpdateZone::set_sending_warning_letter(const bool _warning_letter)
{
    warning_letter_ = _warning_letter;
    return *this;
}

unsigned long long UpdateZone::exec(OperationContext& _ctx)
{
    const bool is_data_for_update = (ex_period_min_ || ex_period_max_ || val_period_ || warning_letter_);
    if (!is_data_for_update)
    {
        throw NoZoneData();
    }

    if (val_period_ && !is_enum_zone(fqdn_))
    {
        throw NotEnumZone();
    }

    Database::QueryParams params;
    std::ostringstream object_sql;
    Util::HeadSeparator set_separator(" SET "," , ");

    object_sql << "UPDATE zone";
    if (ex_period_min_)
    {
        params.push_back(*ex_period_min_);
        object_sql << set_separator.get() <<  "ex_period_min = $" << params.size() << "::integer";
    }
    if (ex_period_max_)
    {
        params.push_back(*ex_period_max_);
        object_sql << set_separator.get() <<  "ex_period_max = $" << params.size() << "::integer";
    }
    if (val_period_)
    {
        params.push_back(*val_period_);
        object_sql << set_separator.get() <<  "val_period = $" << params.size() << "::integer";
    }
    if (warning_letter_)
    {
        params.push_back(warning_letter_.get());
        object_sql << set_separator.get() <<  "warning_letter = $" << params.size() << "::bool";
    }

    params.push_back(fqdn_);
    object_sql << " WHERE fqdn = $" << params.size() << "::text RETURNING id";

    Database::Result object_result;
    try
    {
        object_result = _ctx.get_conn().exec_params(
                object_sql.str(),
                params);
    }
    catch (const std::exception&)
    {
        throw UpdateZoneException();
    }

    if (object_result.size() != 1)
    {
        throw NonExistentZone();
    }
    const unsigned long long id = object_result[0][0];
    return id;
}

} // namespace Zone
} // namespace LibFred
