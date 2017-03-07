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

#include "src/epp/keyset/check_keyset.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/fredlib/keyset/check_keyset.h"

namespace Epp {
namespace Keyset {

namespace {

Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> validity_to_check_result(
        Fred::Keyset::HandleState::SyntaxValidity _validity)
{
    switch (_validity)
    {
        case Fred::Keyset::HandleState::invalid:
            return Keyset::KeysetHandleRegistrationObstruction::invalid_handle;

        case Fred::Keyset::HandleState::valid:
            return Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum>();
    }

    throw std::runtime_error("Invalid Fred::Keyset::HandleState::SyntaxValidity value.");
}


Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> keyset_handle_state_to_check_result(
        Fred::Keyset::HandleState::SyntaxValidity _handle_validity,
        Fred::Keyset::HandleState::Registrability _handle_registrability)
{
    switch (_handle_registrability)
    {
        case Fred::Keyset::HandleState::registered:
            return Keyset::KeysetHandleRegistrationObstruction::registered_handle;

        case Fred::Keyset::HandleState::in_protection_period:
            return Keyset::KeysetHandleRegistrationObstruction::protected_handle;

        case Fred::Keyset::HandleState::available:
            return validity_to_check_result(_handle_validity);
    }

    throw std::runtime_error("Invalid Fred::Keyset::HandleState::Registrability value.");
}


} // namespace Epp::{anonymous}

std::map<std::string, Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> > check_keyset(
        Fred::OperationContext& _ctx,
        const std::set<std::string>& _keyset_handles,
        unsigned long long _registrar_id)
{
    const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id)
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
                        Fred::Keyset::get_handle_syntax_validity(*handle_ptr),
                        Fred::Keyset::get_handle_registrability(_ctx, *handle_ptr));
    }

    return result;
}


} // namespace Epp::Keyset
} // namespace Epp
