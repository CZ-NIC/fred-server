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

#include "src/backend/epp/keyset/update_keyset.hh"

#include "src/backend/epp/epp_extended_error.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/keyset/impl/limits.hh"

#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/poll/create_poll_message.hh"
#include "libfred/registrar/info_registrar.hh"

#include <strings.h>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

namespace {

LibFred::InfoKeysetData check_keyset_handle(
        const std::string& _keyset_handle,
        unsigned long long _registrar_id,
        LibFred::OperationContext& _ctx,
        std::string& _session_registrar_handle)
{
    try
    {
        const LibFred::InfoRegistrarData session_registrar =
            LibFred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data;
        const LibFred::InfoKeysetData keyset_data =
            LibFred::InfoKeysetByHandle(_keyset_handle).set_lock().exec(_ctx).info_keyset_data;
        const bool is_sponsoring_registrar =
            (keyset_data.sponsoring_registrar_handle == session_registrar.handle);
        const bool is_system_registrar = session_registrar.system.get_value_or(false);
        const bool is_registrar_authorized = (is_system_registrar || is_sponsoring_registrar);
        if (!is_registrar_authorized)
        {
            _ctx.get_log().info("check_keyset_handle failure: registrar not authorized for this operation");
            throw EppResponseFailure(EppResultFailure(
                    EppResultCode::authorization_error));
        }
        if (!is_system_registrar)
        {
            LibFred::LockObjectStateRequestLock(keyset_data.id).exec(_ctx);
            LibFred::PerformObjectStateRequest(keyset_data.id).exec(_ctx);
            const LibFred::ObjectStatesInfo keyset_states(LibFred::GetObjectStates(keyset_data.id).exec(_ctx));
            if (keyset_states.presents(LibFred::Object_State::server_update_prohibited) ||
                keyset_states.presents(LibFred::Object_State::delete_candidate))
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
            }
        }
        _session_registrar_handle = session_registrar.handle;
        return keyset_data;
    }
    catch (const LibFred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            _ctx.get_log().info("check_keyset_handle failure: keyset not found");
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        _ctx.get_log().error("check_keyset_handle failure: unexpected error has occurred");
        throw;
    }
    catch (const LibFred::InfoRegistrarById::Exception& e)
    {
        if (e.is_set_unknown_registrar_id())
        {
            _ctx.get_log().error("check_keyset_handle failure: registrar id not found");
            throw;
        }
        _ctx.get_log().error(
                "check_keyset_handle failure: unexpected error has occurred in "
                "InfoRegistrarById operation");
        throw;
    }
    catch (const EppResponseFailure& e)
    {
        _ctx.get_log().info(std::string("check_keyset_handle failure: ") + e.what());
        throw;
    }
    catch (...)
    {
        _ctx.get_log().error("unexpected exception in check_keyset_handle function");
        throw;
    }
}

struct CompareIgnoringCase
{
    bool operator()(const std::string& _lhs, const std::string& _rhs)const
    {
        const bool lhs_is_shorter = _lhs.length() < _rhs.length();
        const std::size_t size = lhs_is_shorter ? _lhs.length() : _rhs.length();
        const int cmp_result = ::strncasecmp(_lhs.c_str(), _rhs.c_str(), size);
        return (cmp_result < 0) ||
               ((cmp_result == 0) && lhs_is_shorter);
    }
};

typedef std::set<std::string, CompareIgnoringCase> Handles;

Handles get_handles_of_tech_contacts(const std::vector<LibFred::RegistrableObject::Contact::ContactReference>& _tech_contacts)
{
    Handles result;
    for (const auto& tech_contact : _tech_contacts)
    {
        result.insert(tech_contact.handle);
    }
    return result;
}

typedef bool Presents;

Presents copy_error(
        Param::Enum _param,
        Reason::Enum _reason,
        unsigned short _src_idx,
        unsigned short _dst_idx,
        EppResultFailure& _epp_result_errors)
{
    if (Epp::has_extended_error_with_param_index_reason(_epp_result_errors, _param, _src_idx, _reason))
    {
        _epp_result_errors.add_extended_error(
                EppExtendedError::of_vector_parameter(
                        _param,
                        _dst_idx,
                        _reason));
        return true;
    }
    return false;
}

typedef bool Success;

Success check_tech_contacts(
        const std::vector<std::string>& _tech_contacts_add,
        const std::vector<std::string>& _tech_contacts_rem,
        const LibFred::InfoKeysetData& _keyset_data,
        LibFred::OperationContext& _ctx,
        EppResultFailure& _policy_errors)
{
    if (_tech_contacts_add.empty() && _tech_contacts_rem.empty())  // nothing to do
    {
        return true;
    }

    Success check_result = true;

    if (Keyset::max_number_of_tech_contacts < _tech_contacts_add.size())
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_tech_add,
                        Reason::techadmin_limit));
        check_result = false;
    }

    const Handles current_tech_contacts = get_handles_of_tech_contacts(_keyset_data.tech_contacts);

    if (current_tech_contacts.size() < _tech_contacts_rem.size())
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_tech_rem,
                        Reason::can_not_remove_tech));
        check_result = false;
    }

    // prevents detailed checking of too long _tech_contacts_add or _tech_contacts_rem lists
    if (!check_result)
    {
        return false;
    }

    typedef std::map<std::string, unsigned short, CompareIgnoringCase> HandleIndex;

    HandleIndex unique_handles;
    unsigned short idx = 0;
    for (std::vector<std::string>::const_iterator handle_ptr = _tech_contacts_add.begin();
         handle_ptr != _tech_contacts_add.end(); ++handle_ptr, ++idx)
    {
        const HandleIndex::const_iterator handle_index_ptr = unique_handles.find(*handle_ptr);
        const bool handle_is_unique = handle_index_ptr == unique_handles.end();
        if (!handle_is_unique)
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_tech_add,
                            idx,
                            Reason::duplicated_contact));
            if (Epp::has_extended_error_with_param_index_reason(
                        _policy_errors,
                        Param::keyset_tech_add,
                        handle_index_ptr->second,
                        Reason::technical_contact_not_registered))
            {
                _policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::keyset_tech_add,
                                idx,
                                Reason::technical_contact_not_registered));
            }
            if (Epp::has_extended_error_with_param_index_reason(
                        _policy_errors,
                        Param::keyset_tech_add,
                        handle_index_ptr->second,
                        Reason::technical_contact_already_assigned))
            {
                _policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::keyset_tech_add,
                                idx,
                                Reason::technical_contact_already_assigned));
            }
            check_result = false;
            continue;
        }
        // unique handle
        unique_handles.insert(std::make_pair(*handle_ptr, idx));
        if (0 < current_tech_contacts.count(*handle_ptr))
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_tech_add,
                            idx,
                            Reason::technical_contact_already_assigned));
            check_result = false;
            continue;
        }
        switch (LibFred::Contact::get_handle_registrability(_ctx, *handle_ptr))
        {
            case LibFred::ContactHandleState::Registrability::registered:
                break;

            case LibFred::ContactHandleState::Registrability::available:
            case LibFred::ContactHandleState::Registrability::in_protection_period:
                _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_tech_add,
                            idx,
                            Reason::technical_contact_not_registered));
                check_result = false;
                break;
        }
    }

    unique_handles.clear();
    idx = 0;
    for (std::vector<std::string>::const_iterator handle_ptr = _tech_contacts_rem.begin();
         handle_ptr != _tech_contacts_rem.end(); ++handle_ptr, ++idx)
    {
        const HandleIndex::const_iterator handle_index_ptr = unique_handles.find(*handle_ptr);
        const bool handle_is_unique = handle_index_ptr == unique_handles.end();
        if (!handle_is_unique)
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_tech_rem,
                            idx,
                            Reason::duplicated_contact));
            if (Epp::has_extended_error_with_param_index_reason(
                        _policy_errors,
                        Param::keyset_tech_rem,
                        handle_index_ptr->second,
                        Reason::technical_contact_not_registered))
            {
                _policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::keyset_tech_rem,
                                idx,
                                Reason::technical_contact_not_registered));
            }

            check_result = false;
            continue;
        }
        // unique handle
        unique_handles.insert(std::make_pair(*handle_ptr, idx));
        if (current_tech_contacts.count(*handle_ptr) == 0)
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_tech_rem,
                            idx,
                            Reason::technical_contact_not_registered));
            check_result = false;
            continue;
        }
    }
    unique_handles.clear();

    if (!check_result)
    {
        return false;
    }

    const unsigned short number_of_tech_contacts =
            current_tech_contacts.size() + _tech_contacts_add.size() - _tech_contacts_rem.size();
    if (number_of_tech_contacts < Keyset::min_number_of_tech_contacts)
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_tech,
                        Reason::techadmin_limit));
        return false;
    }
    if (Keyset::max_number_of_tech_contacts < number_of_tech_contacts)
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_tech,
                        Reason::techadmin_limit));
        return false;
    }
    return true;
}

template <unsigned MIN_NUMBER_OF_DS_RECORDS, unsigned MAX_NUMBER_OF_DS_RECORDS>
Success check_ds_records(
        LibFred::OperationContext&,
        const std::vector<Keyset::DsRecord>&,
        const std::vector<Keyset::DsRecord>&,
        const LibFred::InfoKeysetData&,
        EppResultFailure&);

// specialization for requirement of no DS records
template <>
Success check_ds_records<0, 0>(
        LibFred::OperationContext&,
        const std::vector<Keyset::DsRecord>& _ds_records_add,
        const std::vector<Keyset::DsRecord>& _ds_records_rem,
        const LibFred::InfoKeysetData&,
        EppResultFailure& _policy_errors)
{
    if (_ds_records_add.empty() && _ds_records_rem.empty())
    {
        return true;
    }
    _policy_errors.add_extended_error(
            EppExtendedError::of_scalar_parameter(
                    Param::keyset_dsrecord,
                    Reason::dsrecord_limit));
    return false;
}

typedef std::set<Keyset::DnsKey> DnsKeys;
DnsKeys get_dns_keys(const std::vector<LibFred::DnsKey>& _dns_keys)
{
    DnsKeys result;
    for (std::vector<LibFred::DnsKey>::const_iterator dns_key_ptr = _dns_keys.begin();
         dns_key_ptr != _dns_keys.end(); ++dns_key_ptr)
    {
        result.insert(
                Keyset::DnsKey(
                        dns_key_ptr->get_flags(),
                        dns_key_ptr->get_protocol(),
                        dns_key_ptr->get_alg(),
                        dns_key_ptr->get_key()));
    }
    return result;
}

Success check_dns_keys(
        const std::vector<Keyset::DnsKey>& _dns_keys_add,
        const std::vector<Keyset::DnsKey>& _dns_keys_rem,
        const LibFred::InfoKeysetData& _keyset_data,
        LibFred::OperationContext& _ctx,
        EppResultFailure& _policy_errors)
{
    if (_dns_keys_add.empty() && _dns_keys_rem.empty())  // nothing to do
    {
        return true;
    }

    Success check_result = true;

    if (Keyset::max_number_of_dns_keys < _dns_keys_add.size())
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_dnskey,
                        Reason::dnskey_limit));
        check_result = false;
    }

    const DnsKeys current_dns_keys = get_dns_keys(_keyset_data.dns_keys);

    if (current_dns_keys.size() < _dns_keys_rem.size())
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_dnskey,
                        Reason::no_dnskey_dsrecord));
        check_result = false;
    }

    // prevents detailed checking of too long _tech_contacts_add or _tech_contacts_rem lists
    if (!check_result)
    {
        return false;
    }

    Keyset::DnsKey::AlgValidator alg_validator(_ctx);

    typedef std::map<Keyset::DnsKey, unsigned short> DnsKeyIndex;

    DnsKeyIndex unique_dns_keys;
    unsigned short idx = 0;
    for (std::vector<Keyset::DnsKey>::const_iterator dns_key_ptr = _dns_keys_add.begin();
         dns_key_ptr != _dns_keys_add.end(); ++dns_key_ptr, ++idx)
    {
        const DnsKeyIndex::const_iterator dns_key_index_ptr = unique_dns_keys.find(*dns_key_ptr);
        const bool dns_key_is_unique = dns_key_index_ptr == unique_dns_keys.end();
        if (!dns_key_is_unique)
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey_add,
                            idx,
                            Reason::duplicated_dnskey));
            copy_error(
                    Param::keyset_dnskey_add,
                    Reason::dnskey_exist,
                    dns_key_index_ptr->second,
                    idx,
                    _policy_errors);
            copy_error(
                    Param::keyset_dnskey_add,
                    Reason::dnskey_bad_flags,
                    dns_key_index_ptr->second,
                    idx,
                    _policy_errors);
            copy_error(
                    Param::keyset_dnskey_add,
                    Reason::dnskey_bad_protocol,
                    dns_key_index_ptr->second,
                    idx,
                    _policy_errors);
            copy_error(
                    Param::keyset_dnskey_add,
                    Reason::dnskey_bad_alg,
                    dns_key_index_ptr->second,
                    idx,
                    _policy_errors);
            copy_error(
                    Param::keyset_dnskey_add,
                    Reason::dnskey_bad_key_char,
                    dns_key_index_ptr->second,
                    idx,
                    _policy_errors);
            copy_error(
                    Param::keyset_dnskey_add,
                    Reason::dnskey_bad_key_len,
                    dns_key_index_ptr->second,
                    idx,
                    _policy_errors);
            check_result = false;
            continue;
        }

        // unique DNS key
        unique_dns_keys.insert(std::make_pair(*dns_key_ptr, idx));
        if (0 < current_dns_keys.count(*dns_key_ptr))
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey_add,
                            idx,
                            Reason::dnskey_exist));
            check_result = false;
            continue;
        }

        if (!dns_key_ptr->is_flags_correct())
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey_add,
                            idx,
                            Reason::dnskey_bad_flags));
            check_result = false;
        }

        if (!dns_key_ptr->is_protocol_correct())
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey_add,
                            idx,
                            Reason::dnskey_bad_protocol));
            check_result = false;
        }

        if (!alg_validator.is_alg_correct(*dns_key_ptr))
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey_add,
                            idx,
                            Reason::dnskey_bad_alg));
            check_result = false;
        }

        switch (dns_key_ptr->check_key())
        {
            case Keyset::DnsKey::CheckKey::ok:
                break;

            case Keyset::DnsKey::CheckKey::bad_char:
                _policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::keyset_dnskey_add,
                                idx,
                                Reason::dnskey_bad_key_char));
                check_result = false;
                break;

            case Keyset::DnsKey::CheckKey::bad_length:
                _policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::keyset_dnskey_add,
                                idx,
                                Reason::dnskey_bad_key_len));
                check_result = false;
                break;
        }
    }

    unique_dns_keys.clear();
    idx = 0;
    for (std::vector<Keyset::DnsKey>::const_iterator dns_key_ptr = _dns_keys_rem.begin();
         dns_key_ptr != _dns_keys_rem.end(); ++dns_key_ptr, ++idx)
    {
        const DnsKeyIndex::const_iterator dns_key_index_ptr = unique_dns_keys.find(*dns_key_ptr);
        const bool dns_key_is_unique = dns_key_index_ptr == unique_dns_keys.end();
        if (!dns_key_is_unique)
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey_rem,
                            idx,
                            Reason::duplicated_dnskey));
            copy_error(
                    Param::keyset_dnskey_rem,
                    Reason::dnskey_notexist,
                    dns_key_index_ptr->second,
                    idx,
                    _policy_errors);
            check_result = false;
            continue;
        }
        // unique DNS key
        unique_dns_keys.insert(std::make_pair(*dns_key_ptr, idx));
        if (current_dns_keys.count(*dns_key_ptr) == 0)
        {
            _policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::keyset_dnskey_rem,
                            idx,
                            Reason::dnskey_notexist));
            check_result = false;
            continue;
        }
    }
    unique_dns_keys.clear();

    if (!check_result)
    {
        return false;
    }

    if ((current_dns_keys.size() + _dns_keys_add.size()) < _dns_keys_rem.size())
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_dnskey,
                        Reason::no_dnskey_dsrecord));
        return false;
    }
    const unsigned short number_of_dns_keys = current_dns_keys.size() + _dns_keys_add.size() -
                                              _dns_keys_rem.size();
    if (number_of_dns_keys < Keyset::min_number_of_dns_keys)
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_dnskey,
                        Reason::no_dnskey_dsrecord));
        return false;
    }
    if (Keyset::max_number_of_dns_keys < number_of_dns_keys)
    {
        _policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::keyset_dnskey,
                        Reason::dnskey_limit));
        return false;
    }
    return true;
}

std::vector<LibFred::DnsKey> to_fred(const std::vector<Keyset::DnsKey>& _dns_keys)
{
    std::vector<LibFred::DnsKey> result;
    for (std::vector<Keyset::DnsKey>::const_iterator dns_key_ptr = _dns_keys.begin();
         dns_key_ptr != _dns_keys.end(); ++dns_key_ptr)
    {
        result.push_back(
                LibFred::DnsKey(
                        dns_key_ptr->get_flags(),
                        dns_key_ptr->get_protocol(),
                        dns_key_ptr->get_alg(),
                        dns_key_ptr->get_key()));
    }
    return result;
}

} // namespace Epp::Keyset::{anonymous}

UpdateKeysetResult update_keyset(
        LibFred::OperationContext& _ctx,
        const UpdateKeysetInputData& _update_keyset_data,
        const UpdateKeysetConfigData&,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    UpdateKeysetResult result;
    std::string session_registrar_handle;
    bool is_sponsoring_registrar = false;
    {
        const LibFred::InfoKeysetData keyset_data = check_keyset_handle(
                _update_keyset_data.keyset_handle,
                _session_data.registrar_id,
                _ctx,
                session_registrar_handle);

        is_sponsoring_registrar = keyset_data.sponsoring_registrar_handle == session_registrar_handle;
        EppResultFailure existing_objects = EppResultFailure(EppResultCode::object_exists);
        EppResultFailure missing_parameters = EppResultFailure(EppResultCode::required_parameter_missing);
        EppResultFailure policy_errors = EppResultFailure(EppResultCode::parameter_value_policy_error);
        EppResultFailure syntax_errors = EppResultFailure(EppResultCode::parameter_value_syntax_error);

        if (!check_tech_contacts(
                    _update_keyset_data.tech_contacts_add,
                    _update_keyset_data.tech_contacts_rem,
                    keyset_data,
                    _ctx,
                    policy_errors))
        {
            _ctx.get_log().info("check_tech_contacts failure");
        }

        if (!check_ds_records<Keyset::min_number_of_ds_records, Keyset::max_number_of_ds_records>(
                    _ctx,
                    _update_keyset_data.ds_records_add,
                    _update_keyset_data.ds_records_rem,
                    keyset_data,
                    policy_errors))
        {
            _ctx.get_log().info("check_ds_records failure");
        }

        if (!check_dns_keys(
                    _update_keyset_data.dns_keys_add,
                    _update_keyset_data.dns_keys_rem,
                    keyset_data,
                    _ctx,
                    policy_errors))
        {
            _ctx.get_log().info("check_dns_keys failure");
        }

        if (!policy_errors.empty())
        {
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

            {
                EppResultFailure policy_errors_to_throw = EppResultFailure(
                        EppResultCode::parameter_value_policy_error);

                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech_add,
                                Reason::technical_contact_not_registered));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech_add,
                                Reason::duplicated_contact));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech_add,
                                Reason::technical_contact_already_assigned));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }

                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech_rem,
                                Reason::technical_contact_not_registered));
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech_rem,
                                Reason::duplicated_contact));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }

                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_add,
                                Reason::duplicated_dnskey));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }

                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_add,
                                Reason::dnskey_bad_flags));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_add,
                                Reason::dnskey_bad_protocol));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_add,
                                Reason::dnskey_bad_alg));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_add,
                                Reason::dnskey_bad_key_char));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_add,
                                Reason::dnskey_bad_key_len));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_add,
                                Reason::dnskey_exist));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_rem,
                                Reason::duplicated_dnskey));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey_rem,
                                Reason::dnskey_notexist));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }

                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_dnskey,
                                Reason::no_dnskey_dsrecord));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
                policy_errors_to_throw.add_extended_errors(
                        Epp::extended_errors_with_param_reason(
                                policy_errors,
                                Param::keyset_tech_rem,
                                Reason::can_not_remove_tech));
                if (!policy_errors_to_throw.empty())
                {
                    throw EppResponseFailure(policy_errors_to_throw);
                }
            }
        }

        result.id = keyset_data.id;
    }

    try
    {
        const std::vector<LibFred::DnsKey> dns_keys_add = to_fred(_update_keyset_data.dns_keys_add);
        const std::vector<LibFred::DnsKey> dns_keys_rem = to_fred(_update_keyset_data.dns_keys_rem);
        result.update_history_id = LibFred::UpdateKeyset(
                _update_keyset_data.keyset_handle,
                session_registrar_handle,
                _update_keyset_data.authinfopw,
                _update_keyset_data.tech_contacts_add,
                _update_keyset_data.tech_contacts_rem,
                dns_keys_add,
                dns_keys_rem,
                _session_data.logd_request_id).exec(_ctx);
        if (!_update_keyset_data.tech_contacts_rem.empty())
        {
            const LibFred::InfoKeysetData keyset_data =
                LibFred::InfoKeysetByHandle(_update_keyset_data.keyset_handle).exec(_ctx).info_keyset_data;
            if (keyset_data.tech_contacts.size() < Keyset::min_number_of_tech_contacts)
            {
                throw EppResponseFailure(
                        EppResultFailure(EppResultCode::parameter_value_policy_error)
                        .add_extended_error(
                                EppExtendedError::of_scalar_parameter(
                                        Param::keyset_tech_rem,
                                        Reason::can_not_remove_tech)));
            }
        }
        if (!is_sponsoring_registrar)
        {
            LibFred::Poll::CreatePollMessage<LibFred::Poll::MessageType::update_keyset>().exec(_ctx, result.update_history_id);
        }
        return result;
    }
    catch (const LibFred::UpdateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_keyset_handle())
        {
            _ctx.get_log().warning("unknown keyset handle");
        }
        if (e.is_set_unknown_registrar_handle())
        {
            _ctx.get_log().warning("unknown registrar handle");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            _ctx.get_log().warning("unknown technical contact handle");
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            _ctx.get_log().warning("duplicate technical contact handle");
        }
        if (e.is_set_vector_of_unassigned_technical_contact_handle())
        {
            _ctx.get_log().warning("unassigned technical contact handle");
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            _ctx.get_log().warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unassigned_dns_key())
        {
            _ctx.get_log().warning("unassigned dns key");
        }
        throw;
    }
}

} // namespace Epp::Keyset
} // namespace Epp
