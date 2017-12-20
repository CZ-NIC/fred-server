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

#include "src/backend/epp/contact/transfer_contact.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/impl/util.hh"
#include "src/libfred/registrable_object/contact/check_contact.hh"
#include "src/libfred/registrable_object/contact/create_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/transfer_contact.hh"
#include "src/libfred/exception.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/object/transfer_object_exception.hh"
#include "src/libfred/object_state/lock_object_state_request_lock.hh"
#include "src/libfred/object_state/object_has_state.hh"
#include "src/libfred/object_state/perform_object_state_request.hh"
#include "src/libfred/poll/create_poll_message.hh"
#include "src/libfred/poll/message_type.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/util/optional_value.hh"

#include <string>

namespace Epp {
namespace Contact {

unsigned long long transfer_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const TransferContactConfigData& _transfer_contact_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    LibFred::InfoContactData contact_data_before_transfer;

    try
    {
        contact_data_before_transfer =
                LibFred::InfoContactByHandle(_contact_handle)
                        .set_lock()
                        .exec(_ctx)
                        .info_contact_data;
    }
    catch (const LibFred::InfoContactByHandle::Exception& ex)
    {
        if (ex.is_set_unknown_contact_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }


    contact_data_before_transfer =
            LibFred::InfoContactByHandle(_contact_handle)
                    .set_lock()
                    .exec(_ctx)
                    .info_contact_data;

    const LibFred::InfoRegistrarData session_registrar =
        LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;

    const bool is_sponsoring_registrar = (contact_data_before_transfer.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    if (is_sponsoring_registrar) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }

    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    if (!is_system_registrar) {
        // do it before any object state related checks
        LibFred::LockObjectStateRequestLock(contact_data_before_transfer.id).exec(_ctx);
        LibFred::PerformObjectStateRequest(contact_data_before_transfer.id).exec(_ctx);

        const LibFred::ObjectStatesInfo contact_states_before_transfer(LibFred::GetObjectStates(contact_data_before_transfer.id).exec(_ctx));

        if (contact_states_before_transfer.presents(LibFred::Object_State::server_transfer_prohibited) ||
            contact_states_before_transfer.presents(LibFred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    if (contact_data_before_transfer.authinfopw != _authinfopw) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }

    try {
        const unsigned long long post_transfer_history_id =
            LibFred::TransferContact(
                contact_data_before_transfer.id,
                session_registrar.handle,
                _authinfopw,
                _session_data.logd_request_id.isset() ? _session_data.logd_request_id.get_value() : Nullable<unsigned long long>()
            ).exec(_ctx);

        LibFred::Poll::CreatePollMessage<LibFred::Poll::MessageType::transfer_contact>()
                .exec(_ctx, post_transfer_history_id);

        return post_transfer_history_id;
    }
    catch (const LibFred::UnknownContactId&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const LibFred::IncorrectAuthInfoPw&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const LibFred::NewRegistrarIsAlreadySponsoring&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }
}

} // namespace Epp::Contact
} // namespace Epp
