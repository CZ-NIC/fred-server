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

#include "src/epp/contact/transfer_contact.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/transfer_contact.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/object_states_info.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/poll/create_epp_action_poll_message_impl.h"
#include "src/fredlib/poll/message_types.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {
namespace Contact {

unsigned long long transfer_contact(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const TransferContactConfigData& _transfer_contact_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    if (Fred::Contact::get_handle_registrability(_ctx, _contact_handle) != Fred::ContactHandleState::Registrability::registered) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    const Fred::InfoContactData contact_data_before_transfer =
        Fred::InfoContactByHandle(_contact_handle).set_lock().exec(_ctx).info_contact_data;

    const Fred::InfoRegistrarData session_registrar =
        Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;

    const bool is_sponsoring_registrar = (contact_data_before_transfer.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    if (is_sponsoring_registrar) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }

    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    if (!is_system_registrar) {
        // do it before any object state related checks
        Fred::LockObjectStateRequestLock(contact_data_before_transfer.id).exec(_ctx);
        Fred::PerformObjectStateRequest(contact_data_before_transfer.id).exec(_ctx);

        const Fred::ObjectStatesInfo contact_states_before_transfer(Fred::GetObjectStates(contact_data_before_transfer.id).exec(_ctx));

        if (contact_states_before_transfer.presents(Fred::Object_State::server_transfer_prohibited) ||
            contact_states_before_transfer.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    if (contact_data_before_transfer.authinfopw != _authinfopw) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }

    try {
        const unsigned long long post_transfer_history_id =
            Fred::TransferContact(
                contact_data_before_transfer.id,
                session_registrar.handle,
                _authinfopw,
                _session_data.logd_request_id.isset() ? _session_data.logd_request_id.get_value() : Nullable<unsigned long long>()
            ).exec(_ctx);

        Fred::Poll::CreateEppActionPollMessage(post_transfer_history_id,
                                               Fred::Poll::contact,
                                               Fred::Poll::TRANSFER_CONTACT).exec(_ctx);

        return post_transfer_history_id;
    }
    catch (const Fred::UnknownContactId&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const Fred::IncorrectAuthInfoPw&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }
}

} // namespace Epp::Contact
} // namespace Epp
