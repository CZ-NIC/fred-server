/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/keyset/check_keyset.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"

namespace Epp {
namespace Keyset {

namespace {

Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> validity_to_check_result(
        LibFred::Keyset::HandleState::SyntaxValidity _validity)
{
    switch (_validity)
    {
        case LibFred::Keyset::HandleState::invalid:
            return Keyset::KeysetHandleRegistrationObstruction::invalid_handle;

        case LibFred::Keyset::HandleState::valid:
            return Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum>();
    }

    throw std::runtime_error("Invalid LibFred::Keyset::HandleState::SyntaxValidity value.");
}


Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> keyset_handle_state_to_check_result(
        LibFred::Keyset::HandleState::SyntaxValidity _handle_validity,
        LibFred::Keyset::HandleState::Registrability _handle_registrability)
{
    switch (_handle_registrability)
    {
        case LibFred::Keyset::HandleState::registered:
            return Keyset::KeysetHandleRegistrationObstruction::registered_handle;

        case LibFred::Keyset::HandleState::in_protection_period:
            return Keyset::KeysetHandleRegistrationObstruction::protected_handle;

        case LibFred::Keyset::HandleState::available:
            return validity_to_check_result(_handle_validity);
    }

    throw std::runtime_error("Invalid LibFred::Keyset::HandleState::Registrability value.");
}


} // namespace Epp::{anonymous}

std::map<std::string, Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> > check_keyset(
        LibFred::OperationContext& _ctx,
        const std::set<std::string>& _keyset_handles,
        const CheckKeysetConfigData&,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    typedef std::set<std::string> Handles;
    typedef std::map<std::string, Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> > CheckResult;

    CheckResult result;

    for (Handles::const_iterator handle_ptr = _keyset_handles.begin();
         handle_ptr != _keyset_handles.end(); ++handle_ptr)
    {
        result[*handle_ptr] =
                keyset_handle_state_to_check_result(
                        LibFred::Keyset::get_handle_syntax_validity(_ctx, *handle_ptr),
                        LibFred::Keyset::get_handle_registrability(_ctx, *handle_ptr));
    }

    return result;
}


} // namespace Epp::Keyset
} // namespace Epp
