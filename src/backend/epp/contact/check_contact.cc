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
#include "src/backend/epp/contact/check_contact.hh"

#include "src/backend/epp/contact/contact_handle_state_to_check_result.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "libfred/registrable_object/contact/check_contact.hh"

#include <boost/foreach.hpp>

namespace Epp {
namespace Contact {

std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > check_contact(
        LibFred::OperationContext& _ctx,
        const std::set<std::string>& _contact_handles,
        const CheckContactConfigData& _check_contact_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > result;

    BOOST_FOREACH(const std::string& handle, _contact_handles) {

        result[handle] = contact_handle_state_to_check_result(
            LibFred::Contact::get_handle_syntax_validity(_ctx, handle),
            LibFred::Contact::get_handle_registrability(_ctx, handle)
        );

    }

    return result;
}

} // namespace Epp::Contact
} // namespace Epp
