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

#include "src/backend/epp/domain/create_domain.hh"

#include "src/backend/epp/domain/impl/domain_billing.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/impl/util.hh"

#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/domain.hh"
#include "libfred/registrable_object/domain/domain_name.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/registrar_zone_access.hh"
#include "libfred/zone/zone.hh"

#include "util/db/param_query_composition.hh"
#include "util/optional_value.hh"
#include "util/util.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <set>
#include <string>


namespace Epp {
namespace Domain {

CreateDomainResult create_domain(
        LibFred::OperationContext& _ctx,
        const CreateDomainInputData& _data,
        const CreateDomainConfigData& _create_domain_config_data,
        const SessionData& _session_data)
{
    EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

    // check registrar logged in
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    const LibFred::InfoRegistrarData session_registrar =
            LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;

    const bool is_system_registrar = session_registrar.system.get_value_or(false);

    const LibFred::Domain::DomainRegistrability::Enum domain_registrability =
            LibFred::Domain::get_domain_registrability_by_domain_fqdn(_ctx, _data.fqdn, is_system_registrar);

    // check fqdn has known zone
    if (domain_registrability == LibFred::Domain::DomainRegistrability::zone_not_in_registry)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_fqdn,
                                                         Reason::not_applicable_domain)));
    }

    // check fqdn syntax
    if (LibFred::Domain::get_domain_fqdn_syntax_validity(_ctx, _data.fqdn, is_system_registrar) != LibFred::Domain::DomainFqdnSyntaxValidity::valid)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_syntax_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_fqdn,
                                                         Reason::bad_format_fqdn)));
    }

    // check fqdn is not already registered
    if (domain_registrability == LibFred::Domain::DomainRegistrability::registered)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
    }

    // check fqdn is not blacklisted
    if (domain_registrability == LibFred::Domain::DomainRegistrability::blacklisted)
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_fqdn,
                        Reason::blacklisted_domain));
    }

    // get zone data
    const LibFred::Zone::Data zone_data = LibFred::Zone::find_zone_in_fqdn(
            _ctx,
            LibFred::Zone::rem_trailing_dot(_data.fqdn));

    // start of db transaction, utc timestamp without timezone, will be timestamp of domain creation crdate
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
            static_cast<std::string>(_ctx.get_conn().exec(
                    "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));

    // warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time =
            boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    // check registrar zone access permission
    if (!LibFred::is_zone_accessible_by_registrar(
                _session_data.registrar_id,
                zone_data.id,
                current_local_date,
                _ctx))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    // check nsset exists if set
    if (!_data.nsset.empty() &&
            (LibFred::Nsset::get_handle_registrability(_ctx, _data.nsset) != LibFred::NssetHandleState::Registrability::registered))
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_nsset,
                        Reason::nsset_notexist));
    }

    // check keyset exists if set
    if (!_data.keyset.empty() &&
            (LibFred::Keyset::get_handle_registrability(_ctx, _data.keyset) != LibFred::Keyset::HandleState::registered))
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_keyset,
                        Reason::keyset_notexist));
    }

    // check registrant contact set
    if (_data.registrant.empty())
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_registrant,
                                                         Reason::registrant_notexist)));
    }

    // check registrant contact exists
    if ((LibFred::Contact::get_handle_registrability(_ctx, _data.registrant) != LibFred::ContactHandleState::Registrability::registered))
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_registrant,
                        Reason::registrant_notexist));
    }

    // check length of initial domain registration period
    unsigned domain_registration_in_months = 0;
    try
    {
        domain_registration_in_months = boost::numeric_cast<unsigned>(
                _data.period.get_length_of_domain_registration_in_months());
    }
    catch (const boost::numeric::bad_numeric_cast&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_range_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_period,
                                                         Reason::period_range)));
    }
    if (domain_registration_in_months == 0)
    {
        // get min domain registration period by zone
        domain_registration_in_months = zone_data.ex_period_min;
    }
    if (domain_registration_in_months < zone_data.ex_period_min)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_range_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_period,
                                                         Reason::period_too_short)));
    }
    else if (domain_registration_in_months > zone_data.ex_period_max)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_range_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_period,
                                                         Reason::period_range)));
    }
    if (domain_registration_in_months % zone_data.ex_period_min != 0)
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_policy));
    }

    // check expiration date of ENUM domain validation
    if (zone_data.is_enum && !_data.enum_validation_extension)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::required_parameter_missing));
    }

    // check no ENUM validation date in case of non ENUM domain
    if (!zone_data.is_enum && _data.enum_validation_extension)
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_vector_parameter(
                        Param::domain_ext_val_date,
                        0,
                        Reason::valexpdate_not_used));
    }

    // check range of ENUM domain validation expiration
    if (zone_data.is_enum)
    {
        const boost::gregorian::date new_valexdate = (*_data.enum_validation_extension).get_valexdate();

        if (is_new_enum_domain_validation_expiration_date_invalid(
                    new_valexdate,
                    current_local_date,
                    zone_data.enum_validation_period,
                    boost::optional<boost::gregorian::date>(),
                    _ctx))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_range_error)
                                             .add_extended_error(
                                                     EppExtendedError::of_vector_parameter(
                                                         Param::domain_ext_val_date,
                                                         0,
                                                         Reason::valexpdate_not_valid)));
        }
    }

    // check admin contacts
    std::set<std::string> admin_contact_duplicity;
    for (unsigned i = 0; i < _data.admin_contacts.size(); ++i)
    {
        if (LibFred::Contact::get_handle_registrability(_ctx, _data.admin_contacts.at(i)) != LibFred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_admin,
                            boost::numeric_cast<unsigned short>(i),
                            Reason::admin_notexist));
        }
        else
        { // check admin contact duplicity
            if (!admin_contact_duplicity.insert(boost::algorithm::to_upper_copy(_data.admin_contacts.at(i))).second)
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::domain_admin,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::duplicated_contact));
            }
        }
    }

    if (!parameter_value_policy_errors.empty())
    {
        throw EppResponseFailure(parameter_value_policy_errors);
    }

    try
    {

        // domain_expiration_date = current_local_date + domain_registration_in_months using postgresql date arithmetics
        const boost::gregorian::date domain_expiration_date = boost::gregorian::from_simple_string(
                static_cast<std::string>(_ctx.get_conn().exec_params(
                        Database::ParamQuery(
                                "SELECT (")
                                .param_date(current_local_date)(" + ")
                                .param_bigint(domain_registration_in_months)(" * ('1 month'::interval))::date "))[0][0]));

        const Optional<boost::gregorian::date>
                enum_validation_expiration(zone_data.is_enum
                    ? Optional<boost::gregorian::date>((*_data.enum_validation_extension).get_valexdate())
                    : Optional<boost::gregorian::date>());

        const Optional<bool>
                enum_publish_flag(zone_data.is_enum
                                  ? Optional<bool>((*_data.enum_validation_extension).get_publish())
                                  : Optional<bool>());

        const LibFred::CreateDomain::Result result = LibFred::CreateDomain(
                _data.fqdn,
                session_registrar.handle,
                _data.registrant,
                {}, // authinfopw
                _data.nsset.empty()
                        ? Optional<Nullable<std::string> >()
                        : Optional<Nullable<std::string> >(_data.nsset),
                _data.keyset.empty()
                        ? Optional<Nullable<std::string> >()
                        : Optional<Nullable<std::string> >(_data.keyset),
                _data.admin_contacts,
                domain_expiration_date,
                enum_validation_expiration,
                enum_publish_flag,
                _session_data.logd_request_id).exec(_ctx, "UTC");

        if (result.creation_time.is_special())
        {
            throw std::runtime_error("domain create failed");
        }

        if (_create_domain_config_data.rifd_epp_operations_charging)
        {
            const bool charging_is_disabled = is_system_registrar || session_registrar.is_internal;
            if (!charging_is_disabled)
            {
                create_domain_bill_item(
                        _data.fqdn,
                        result.creation_time,
                        _session_data.registrar_id,
                        result.create_object_result.object_id,
                        _ctx);

                renew_domain_bill_item(
                        _data.fqdn,
                        result.creation_time,
                        _session_data.registrar_id,
                        result.create_object_result.object_id,
                        domain_registration_in_months,
                        current_local_date,
                        domain_expiration_date,
                        _ctx);
            }
        }

        return CreateDomainResult(
                result.create_object_result.object_id,
                result.create_object_result.history_id,
                result.creation_time,
                current_local_date,
                domain_expiration_date,
                domain_registration_in_months);
    }
    catch (const LibFred::CreateDomain::Exception& e)
    {

        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle())
        {
            throw;
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
    catch (const BillingFailure&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::billing_failure));
    }
}


} // namespace Epp::Domain
} // namespace Epp
