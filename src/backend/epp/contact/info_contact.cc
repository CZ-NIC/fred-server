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

#include "src/backend/epp/contact/info_contact.hh"

#include "src/backend/admin/contact/verification/contact_states/enum.hh"
#include "src/backend/epp/contact/impl/contact_output.hh"
#include "src/backend/epp/contact/status_value.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/util.hh"

#include "src/deprecated/libfred/registrable_object/contact.hh"

#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/registrar/info_registrar.hh"

#include <boost/foreach.hpp>

#include <string>

namespace Epp {
namespace Contact {

InfoContactOutputData info_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const InfoContactConfigData& _info_contact_config_data,
        const Password& _authinfopw,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        auto info_contact_operation = LibFred::InfoContactByHandle{_contact_handle};
        if (!_authinfopw->empty())
        {
            info_contact_operation.set_lock();  // authinfopw usage counter can be incremented
        }
        const auto info_contact_data = info_contact_operation.exec(_ctx, "UTC").info_contact_data;

        const std::vector<LibFred::ObjectStateData> contact_states_data =
                LibFred::GetObjectStates(info_contact_data.id).exec(_ctx);

        return get_info_contact_output(
                        _ctx,
                        info_contact_data,
                        _authinfopw,
                        *_info_contact_config_data.contact_data_share_policy_rules,
                        _session_data,
                        contact_states_data);
    }
    catch (const ContactDataSharePolicyRules::InvalidAuthorizationInformation&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const LibFred::InfoContactByHandle::Exception& e)
    {
        if (e.is_set_unknown_contact_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Contact
} // namespace Epp
