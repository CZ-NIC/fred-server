/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/keyset/create_keyset.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/keyset/impl/limits.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/map_at.hh"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

namespace {

typedef bool Success;

Success check_keyset_handle(
        const std::string& _keyset_handle,
        LibFred::OperationContext& _ctx,
        EppResultFailure& existing_objects,
        EppResultFailure& policy_errors,
        EppResultFailure& syntax_errors)
{
    switch (LibFred::Keyset::get_handle_syntax_validity(_ctx, _keyset_handle))
    {
        case LibFred::Keyset::HandleState::valid:
            switch (LibFred::Keyset::get_handle_registrability(_ctx, _keyset_handle))
            {
                case LibFred::Keyset::HandleState::registered:
                    existing_objects.add_extended_error(
                        EppExtendedError::of_scalar_parameter(
                                Param::
                                keyset_handle,
                                Reason::existing));
                    return false;

                case LibFred::Keyset::HandleState::in_protection_period:
                    policy_errors.add_extended_error(
                        EppExtendedError::of_scalar_parameter(
                                Param::
                                keyset_handle,
                                Reason::protected_period));
                    return false;

                case LibFred::Keyset::HandleState::available:
                    return true;
            }
            throw std::runtime_error("unexpected keyset handle registrability value");

        case LibFred::Keyset::HandleState::invalid:
            syntax_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_handle,
                        Reason::bad_format_keyset_handle));
            return false;
    }
    throw std::runtime_error("unexpected keyset handle syntax validity value");
}


Success check_tech_contacts(
        const std::vector<std::string>& _tech_contacts,
        LibFred::OperationContext& _ctx,
        EppResultFailure& missing_parameters,
        EppResultFailure& policy_errors)
{
    if (_tech_contacts.size() < Keyset::min_number_of_tech_contacts)
    {
        missing_parameters.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_tech,
                        Reason::technical_contact_not_registered));
        return false;
    }
    if (Keyset::max_number_of_tech_contacts < _tech_contacts.size())
    {
        policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_tech,
                        Reason::techadmin_limit));
        return false;
    }

    Success existing_tech_contacts = true;

    typedef std::map<std::string, unsigned short> HandleIndex;

    HandleIndex unique_handles;
    unsigned short idx = 0;
    for (std::vector<std::string>::const_iterator handle_ptr = _tech_contacts.begin();
         handle_ptr != _tech_contacts.end(); ++handle_ptr, ++idx)
    {
        const HandleIndex::const_iterator handle_index_ptr = unique_handles.find(*handle_ptr);
        if (handle_index_ptr != unique_handles.end())   // duplicate handle
        {
            policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_tech,
                            idx,
                            Reason::duplicated_contact));
            if (Epp::has_extended_error_with_param_index_reason(
                        policy_errors,
                        Param::keyset_tech,
                        handle_index_ptr->second,
                        Reason::technical_contact_not_registered))
            {
                policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::keyset_tech,
                                idx,
                                Reason::technical_contact_not_registered));
            }
            existing_tech_contacts = false;
        }
        else   // unique handle
        {
            unique_handles.insert(std::make_pair(*handle_ptr, idx));
            switch (LibFred::Contact::get_handle_registrability(_ctx, *handle_ptr))
            {
                case LibFred::ContactHandleState::Registrability::registered:
                    break;

                case LibFred::ContactHandleState::Registrability::available:
                case LibFred::ContactHandleState::Registrability::in_protection_period:
                    policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::keyset_tech,
                                    idx,
                                    Reason::technical_contact_not_registered));
                    existing_tech_contacts = false;
                    break;
            }
        }
    }
    return existing_tech_contacts;
}


template <unsigned MIN_NUMBER_OF_DS_RECORDS, unsigned MAX_NUMBER_OF_DS_RECORDS>
Success check_ds_records(
        const std::vector<Keyset::DsRecord>&,
        EppResultFailure& policy_errors);


// specialization for requirement of no ds_record
template <>
Success check_ds_records<0, 0>(
        const std::vector<Keyset::DsRecord>& _ds_records,
        EppResultFailure& policy_errors)
{
    if (_ds_records.empty())
    {
        return true;
    }
    policy_errors.add_extended_error(
            EppExtendedError::of_scalar_parameter(
                    Param::keyset_dsrecord,
                    Reason::dsrecord_limit));
    return false;
}


Success check_dns_keys(
        const std::vector<Keyset::DnsKey>& _dns_keys,
        LibFred::OperationContext& _ctx,
        EppResultFailure& missing_parameters,
        EppResultFailure& policy_errors)
{
    if (_dns_keys.size() < Keyset::min_number_of_dns_keys)
    {
        missing_parameters.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_dnskey,
                        Reason::no_dnskey));
        return false;
    }
    if (Keyset::max_number_of_dns_keys < _dns_keys.size())
    {
        policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_dnskey,
                        Reason::dnskey_limit));
        return false;
    }

    Keyset::DnsKey::AlgValidator alg_validator(_ctx);

    typedef std::map<Keyset::DnsKey, unsigned short> DnsKeyIndex;

    DnsKeyIndex unique_dns_keys;
    Success correct = true;
    unsigned short idx = 0;
    for (std::vector<Keyset::DnsKey>::const_iterator dns_key_ptr = _dns_keys.begin();
         dns_key_ptr != _dns_keys.end(); ++dns_key_ptr, ++idx)
    {
        // check DNS key uniqueness
        const DnsKeyIndex::const_iterator dns_key_index_ptr = unique_dns_keys.find(*dns_key_ptr);
        if (dns_key_index_ptr != unique_dns_keys.end())   // duplicate DNS key
        {
            policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::keyset_dnskey,
                            Reason::duplicated_dnskey));
            static const Param::Enum param = Param::keyset_dnskey;
            static const Reason::Enum reasons[] =
            {
                Reason::dnskey_bad_flags,
                Reason::dnskey_bad_protocol,
                Reason::dnskey_bad_alg,
                Reason::dnskey_bad_key_char,
                Reason::dnskey_bad_key_len
            };
            static const std::size_t number_of_reasons = sizeof (reasons) / sizeof (*reasons);
            static const Reason::Enum* const reasons_end = reasons + number_of_reasons;
            for (const Reason::Enum* reason_ptr = reasons; reason_ptr < reasons_end; ++reason_ptr)
            {
                const Reason::Enum reason = *reason_ptr;
                const unsigned short index_of_first_occurrence = dns_key_index_ptr->second;
                if (Epp::has_extended_error_with_param_index_reason(
                            policy_errors,
                            param,
                            index_of_first_occurrence,
                            reason))
                {
                    policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    param,
                                    idx,
                                    reason));
                }
            }
            correct = false;
            continue;
        }

        // unique DNS key
        unique_dns_keys.insert(std::make_pair(*dns_key_ptr, idx));

        if (!dns_key_ptr->is_flags_correct())
        {
            policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey,
                            idx,
                            Reason::dnskey_bad_flags));
            correct = false;
        }

        if (!dns_key_ptr->is_protocol_correct())
        {
            policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey,
                            idx,
                            Reason::dnskey_bad_protocol));
            correct = false;
        }

        if (!alg_validator.is_alg_correct(*dns_key_ptr))
        {
            policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey,
                            idx,
                            Reason::dnskey_bad_alg));
            correct = false;
        }

        switch (dns_key_ptr->check_key())
        {
            case Keyset::DnsKey::CheckKey::ok:
                break;

            case Keyset::DnsKey::CheckKey::bad_char:
                policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey,
                            idx,
                            Reason::dnskey_bad_key_char));
                correct = false;
                break;

            case Keyset::DnsKey::CheckKey::bad_length:
                policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey,
                            idx,
                            Reason::dnskey_bad_key_len));
                correct = false;
                break;
        }
    }
    return correct;
}


} // namespace Epp::Keyset::{anonymous}

CreateKeysetResult create_keyset(
        LibFred::OperationContext& _ctx,
        const CreateKeysetInputData& _keyset_data,
        const CreateKeysetConfigData& _create_keyset_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    {
        EppResultFailure existing_objects   = EppResultFailure(EppResultCode::object_exists);
        EppResultFailure missing_parameters = EppResultFailure(EppResultCode::required_parameter_missing);
        EppResultFailure policy_errors      = EppResultFailure(EppResultCode::parameter_value_policy_error);
        EppResultFailure syntax_errors      = EppResultFailure(EppResultCode::parameter_value_syntax_error);

        if (!check_keyset_handle(
                    _keyset_data.keyset_handle,
                    _ctx,
                    existing_objects,
                    policy_errors,
                    syntax_errors))
        {
            _ctx.get_log().info("check_keyset_handle failure");
        }
        if (!check_tech_contacts(_keyset_data.tech_contacts, _ctx, missing_parameters, policy_errors))
        {
            _ctx.get_log().info("check_tech_contacts failure");
        }
        if (!check_ds_records<Keyset::min_number_of_ds_records,
                    Keyset::max_number_of_ds_records>(_keyset_data.ds_records, policy_errors))
        {
            _ctx.get_log().info("check_ds_records failure");
        }
        if (!check_dns_keys(_keyset_data.dns_keys, _ctx, missing_parameters, policy_errors))
        {
            _ctx.get_log().info("check_dns_keys failure");
        }

        if (!existing_objects.empty() || !missing_parameters.empty() || !policy_errors.empty() ||
            !syntax_errors.empty())
        {

            if (!missing_parameters.empty())
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::required_parameter_missing));
            }

            if (Epp::has_extended_error_with_param_reason(
                        policy_errors,
                        Param::keyset_dsrecord,
                        Reason::dsrecord_limit)  ||
                Epp::has_extended_error_with_param_reason(
                        policy_errors,
                        Param::keyset_tech,
                        Reason::techadmin_limit) ||
                Epp::has_extended_error_with_param_reason(
                        policy_errors,
                        Param::keyset_dnskey,
                        Reason::dnskey_limit))
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
            }

            if (!syntax_errors.empty())
            {
                throw EppResponseFailure(syntax_errors);
            }

            if (!existing_objects.empty())
            {
                throw EppResponseFailure(existing_objects);
            }

            {
                EppResultFailure policy_errors_to_throw = EppResultFailure(
                        EppResultCode::parameter_value_policy_error);

                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_handle,
                                Reason::protected_period));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }

                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech,
                                Reason::technical_contact_not_registered));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech,
                                Reason::duplicated_contact));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey,
                                Reason::duplicated_dnskey));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey,
                                Reason::dnskey_bad_flags));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey,
                                Reason::dnskey_bad_protocol));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey,
                                Reason::dnskey_bad_alg));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey,
                                Reason::dnskey_bad_key_char));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey,
                                Reason::dnskey_bad_key_len));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
            }

            // existing_objects, missing_parameters, syntax_errors were already thrown
            // but we played with policy_errors; if we missed some policy error, throw policy_errors now
            throw EppResponseFailure(
                    EppResultFailure(EppResultCode::command_failed)
                            .add_extended_errors(
                                    policy_errors.extended_errors().get_value_or(
                                            std::set<
                                                    EppExtendedError>())));
        }
    }

    try
    {
        std::vector<LibFred::DnsKey> dns_keys;
        for (std::vector<Keyset::DnsKey>::const_iterator dns_key_ptr = _keyset_data.dns_keys.begin();
             dns_key_ptr != _keyset_data.dns_keys.end(); ++dns_key_ptr)
        {
            dns_keys.push_back(
                    LibFred::DnsKey(
                            dns_key_ptr->get_flags(),
                            dns_key_ptr->get_protocol(),
                            dns_key_ptr->get_alg(),
                            dns_key_ptr->get_key()));
        }
        const LibFred::CreateKeyset::Result op_result =
                LibFred::CreateKeyset(
                        _keyset_data.keyset_handle,
                        LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle,
                        _keyset_data.authinfopw,
                        dns_keys,
                        _keyset_data.tech_contacts)
                        .exec(_ctx, _session_data.logd_request_id, "UTC");

        CreateKeysetResult result;
        result.id = op_result.create_object_result.object_id,
        result.create_history_id = op_result.create_object_result.history_id,
        result.crdate = op_result.creation_time;
        return result;
    }
    catch (const LibFred::CreateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle())
        {
            _ctx.get_log().warning("unknown registrar handle");
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            _ctx.get_log().warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            _ctx.get_log().warning("unknown technical contact handle");
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            _ctx.get_log().warning("duplicate technical contact handle");
        }
        throw;
    }
}


} // namespace Epp::Keyset
} // namespace Epp
