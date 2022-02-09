/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/epp/domain/renew_domain.hh"

#include "src/backend/epp/domain/impl/domain_billing.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/util.hh"
#include "src/backend/epp/reason.hh"

#include "libfred/registrable_object/domain/domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/object_has_state.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/registrar_zone_access.hh"
#include "libfred/zone/zone.hh"

#include "util/db/param_query_composition.hh"
#include "util/map_at.hh"
#include "util/optional_value.hh"
#include "util/util.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>


namespace Epp {
namespace Domain {

RenewDomainResult renew_domain(
        LibFred::OperationContext& _ctx,
        const RenewDomainInputData& _renew_domain_input_data,
        const RenewDomainConfigData& _renew_domain_config_data,
        const SessionData& _session_data)
{
    // start of db transaction, utc timestamp without timezone, will be timestamp of domain creation crdate
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
            static_cast<std::string>(_ctx.get_conn().exec(
                                             "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));

    // warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time =
        boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    // check registrar logged in
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    // get zone data
    // check fqdn has known zone
    LibFred::Zone::Data zone_data;
    try
    {
        zone_data =
            LibFred::Zone::find_zone_in_fqdn(
                    _ctx,
                    LibFred::Zone::rem_trailing_dot(_renew_domain_input_data.fqdn));
    }
    catch (const LibFred::Zone::Exception& e)
    {
        if (e.is_set_unknown_zone_in_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    // check registrar zone access permission
    if (!LibFred::is_zone_accessible_by_registrar(
                _session_data.registrar_id,
                zone_data.id,
                current_local_date,
                _ctx))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    // check if fqdn is registered and get domain info data and lock domain for update
    LibFred::InfoDomainData info_domain_data_before_renew;
    try
    {
        info_domain_data_before_renew =
                LibFred::InfoDomainByFqdn(_renew_domain_input_data.fqdn)
                        .set_lock()
                        .exec(_ctx, "UTC")
                        .info_domain_data;
    }
    catch (const LibFred::InfoDomainByFqdn::Exception& ex)
    {
        if (ex.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    // check current exdate
    try
    {
        const boost::gregorian::date current_exdate = boost::gregorian::from_simple_string(
                _renew_domain_input_data.current_exdate);
        if (current_exdate != info_domain_data_before_renew.expiration_date)
        {
            throw std::runtime_error("input exdate");
        }
    }
    catch (const std::exception&)
    {
        throw EppResponseFailure(
                EppResultFailure(EppResultCode::parameter_value_policy_error)
                        .add_extended_error(
                                EppExtendedError::of_scalar_parameter(
                                        Param::domain_cur_exp_date,
                                        Reason::curexpdate_not_expdate)));
    }

    EppResultFailure parameter_value_range_errors = EppResultFailure(
            EppResultCode::parameter_value_range_error);
    // check length of new domain registration period
    unsigned domain_registration_in_months = 0;
    try
    {
        domain_registration_in_months = boost::numeric_cast<unsigned>(
                _renew_domain_input_data.period.get_length_of_domain_registration_in_months());
    }
    catch (const boost::numeric::bad_numeric_cast&)
    {
        parameter_value_range_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_range));
    }
    if (domain_registration_in_months == 0)
    {
        // get min domain registration period by zone
        domain_registration_in_months = zone_data.ex_period_min;
    }
    if (domain_registration_in_months < zone_data.ex_period_min)
    {
        parameter_value_range_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_too_short));
    }
    else if (domain_registration_in_months > zone_data.ex_period_max)
    {
        parameter_value_range_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_range));
    }
    if (domain_registration_in_months % zone_data.ex_period_min != 0)
    {
        throw EppResponseFailure(
                EppResultFailure(EppResultCode::parameter_value_policy_error)
                        .add_extended_error(
                                EppExtendedError::of_scalar_parameter(
                                        Param::domain_period,
                                        Reason::period_policy)));
    }

    // check if domain renew is possible
    Database::Result exdate_result = _ctx.get_conn().exec_params(
            Database::ParamQuery
                ("SELECT (").param_date(info_domain_data_before_renew.expiration_date)
                (" + ").param_bigint(domain_registration_in_months)(
                    " * ('1 month'::interval))::date as new_exdate, ")
                ("(").param_date(current_local_date)
                (" + ").param_bigint(zone_data.ex_period_max)(" * ('1 month'::interval))::date as max_exdate"));

    const boost::gregorian::date new_exdate =
        boost::gregorian::from_simple_string(static_cast<std::string>(exdate_result[0]["new_exdate"]));
    const boost::gregorian::date max_exdate =
        boost::gregorian::from_simple_string(static_cast<std::string>(exdate_result[0]["max_exdate"]));

    // curExpDate + domain_registration_in_months <= current_local_date + zone.ex_period_max (in months)
    if (new_exdate > max_exdate)
    {
        parameter_value_range_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_range));
    }

    // check no ENUM validation date in case of non ENUM domain
    if (!zone_data.is_enum && _renew_domain_input_data.enum_validation_extension)
    {
        EppResultFailure parameter_value_policy_errors = EppResultFailure(
                EppResultCode::parameter_value_policy_error);

        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_vector_parameter(
                        Param::domain_ext_val_date,
                        0,
                        Reason::valexpdate_not_used));

        throw EppResponseFailure(parameter_value_policy_errors);
    }

    // check new ENUM domain validation expiration
    if (zone_data.is_enum && _renew_domain_input_data.enum_validation_extension)
    {
        const boost::gregorian::date new_valexdate =
            (*_renew_domain_input_data.enum_validation_extension).get_valexdate();

        // ENUM validation expiration date is optional, if missing ENUM domain is not currently validated
        const boost::optional<boost::gregorian::date> current_valexdate =
            info_domain_data_before_renew.enum_domain_validation.isnull()
            ? boost::optional<boost::gregorian::date>()
            : boost::optional<boost::gregorian::date>(
                    info_domain_data_before_renew.enum_domain_validation.get_value().validation_expiration);

        if (is_new_enum_domain_validation_expiration_date_invalid(
                    new_valexdate,
                    current_local_date,
                    zone_data.enum_validation_period,
                    current_valexdate,
                    _ctx))
        {
            parameter_value_range_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_ext_val_date,
                            0,
                            Reason::valexpdate_not_valid));
        }
    }

    // check sponsoring or system registrar
    const LibFred::InfoRegistrarData session_registrar =
        LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;

    const bool is_system_registrar = session_registrar.system.get_value_or(false);

    if (info_domain_data_before_renew.sponsoring_registrar_handle != session_registrar.handle &&
        !is_system_registrar)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    const LibFred::ObjectStatesInfo domain_states(LibFred::GetObjectStates(info_domain_data_before_renew.id).exec(_ctx));

    if (!session_registrar.system.get_value_or_default() &&
        (domain_states.presents(LibFred::Object_State::server_renew_prohibited)
         ||
         domain_states.presents(LibFred::Object_State::delete_candidate)))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
    }

    if (!parameter_value_range_errors.empty())
    {
        throw EppResponseFailure(EppResultFailure(parameter_value_range_errors));
    }

    try
    {
        LibFred::RenewDomain renew_domain = LibFred::RenewDomain(
                _renew_domain_input_data.fqdn,
                session_registrar.handle,
                new_exdate);

        if (zone_data.is_enum && _renew_domain_input_data.enum_validation_extension)
        {
            renew_domain.set_enum_validation_expiration(
                    (*_renew_domain_input_data.enum_validation_extension).get_valexdate())
                .set_enum_publish_flag(
                    (*_renew_domain_input_data.enum_validation_extension).get_publish());
        }

        if (_session_data.logd_request_id.isset())
        {
            renew_domain.set_logd_request_id(_session_data.logd_request_id.get_value());
        }

        unsigned long long renewed_domain_history_id = renew_domain.exec(_ctx);

        if (_renew_domain_config_data.rifd_epp_operations_charging)
        {
            const bool charging_is_disabled = is_system_registrar || session_registrar.is_internal;
            if (!charging_is_disabled)
            {
                renew_domain_bill_item(
                        _renew_domain_input_data.fqdn,
                        current_utc_time,
                        _session_data.registrar_id,
                        info_domain_data_before_renew.id,
                        domain_registration_in_months,
                        info_domain_data_before_renew.expiration_date,
                        new_exdate,
                        _ctx);
            }
        }

        return RenewDomainResult(
                info_domain_data_before_renew.id,
                renewed_domain_history_id,
                current_utc_time,
                info_domain_data_before_renew.expiration_date,
                new_exdate,
                domain_registration_in_months);

    }
    catch (const LibFred::RenewDomain::Exception& e)
    {
        throw;
    }
    catch (const BillingFailure&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::billing_failure));
    }
}


} // namespace Epp::Domain
} // namespace EPp
