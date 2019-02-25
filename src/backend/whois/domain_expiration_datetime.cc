/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/whois/domain_expiration_datetime.hh"
#include "libfred/object_state/get_object_states.hh"
#include "src/deprecated/libfred/object_state/object_state_name.hh"

namespace Fred {
namespace Backend {
namespace Whois {

namespace {

boost::posix_time::ptime get_object_state_outzone_procedure_run_datetime_estimate(
        LibFred::OperationContext& _ctx,
        const boost::gregorian::date& _date)
{
    /*
         * procedure_run_datetime = (_date + enum_parameters.regular_day_outzone_procedure_period::hour)
         *                          at time zone (enum_parameters.regular_day_procedure_zone)
         */
    const Database::Result r = _ctx.get_conn().exec_params(
            // clang-format off
            "WITH"
                " proc_hour AS"
                " ("
                    " SELECT (val || ' hour')::interval AS value"
                        " FROM enum_parameters"
                      " WHERE"
                            " name = 'regular_day_outzone_procedure_period'"
                " ),"
                " proc_tz AS"
                " ("
                    " SELECT val AS value"
                        " FROM enum_parameters"
                      " WHERE"
                            " name = 'regular_day_procedure_zone'"
                " )"
                " SELECT"
                    " (($1::date + (SELECT value FROM proc_hour)::interval)::timestamp"
                        " AT TIME ZONE (SELECT value FROM proc_tz))::timestamp AS next_run_datetime",
            // clang-format on
            Database::query_param_list(_date));
    if (r.size() == 1)
    {
        return time_from_string(static_cast<std::string>(r[0]["next_run_datetime"]));
    }
    throw std::runtime_error("object state procedure run datetime estimation failed");
}


Optional<boost::posix_time::ptime> get_object_state_valid_from(
        LibFred::OperationContext& _ctx,
        unsigned long long _domain_id,
        const std::string& _state_name)
{
    const std::vector<LibFred::ObjectStateData> states = LibFred::GetObjectStates(_domain_id).exec(_ctx);
    for (std::vector<LibFred::ObjectStateData>::const_iterator it = states.begin(); it != states.end(); ++it)
    {
        if (it->state_name == _state_name)
        {
            return Optional<boost::posix_time::ptime>(it->valid_from_time);
        }
    }
    return Optional<boost::posix_time::ptime>();
}
}

boost::posix_time::ptime domain_expiration_datetime_estimate(
        LibFred::OperationContext& _ctx,
        const boost::gregorian::date& _exdate)
{
    return get_object_state_outzone_procedure_run_datetime_estimate(_ctx, _exdate);
}


boost::posix_time::ptime domain_validation_expiration_datetime_estimate(
        LibFred::OperationContext& _ctx,
        const boost::gregorian::date& _validation_exdate)
{
    return get_object_state_outzone_procedure_run_datetime_estimate(_ctx, _validation_exdate);
}


Optional<boost::posix_time::ptime> domain_expiration_datetime_actual(
        LibFred::OperationContext& _ctx,
        unsigned long long _domain_id)
{
    return get_object_state_valid_from(_ctx, _domain_id, LibFred::ObjectState::EXPIRED);
}


Optional<boost::posix_time::ptime> domain_validation_expiration_datetime_actual(
        LibFred::OperationContext& _ctx,
        unsigned long long _domain_id)
{
    return get_object_state_valid_from(_ctx, _domain_id, LibFred::ObjectState::NOT_VALIDATED);
}

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
