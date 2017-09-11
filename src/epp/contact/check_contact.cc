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

#include "src/epp/contact/check_contact.h"

#include "src/epp/contact/contact_handle_state_to_check_result.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/fredlib/contact/check_contact.h"

#include <boost/foreach.hpp>

namespace Epp {
namespace Contact {

std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > check_contact(
        Fred::OperationContext& _ctx,
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
            Fred::Contact::get_handle_syntax_validity(_ctx, handle),
            Fred::Contact::get_handle_registrability(_ctx, handle)
        );

    }

    return result;
}

} // namespace Epp::Contact
} // namespace Epp
