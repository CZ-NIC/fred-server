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

#include "src/backend/epp/keyset/transfer_keyset.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/impl/util.hh"
#include "libfred/exception.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/transfer_keyset.hh"
#include "libfred/object/object_state.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object/transfer_object_exception.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/poll/create_poll_message.hh"
#include "libfred/poll/message_type.hh"
#include "libfred/registrar/info_registrar.hh"

#include <string>

namespace Epp {
namespace Keyset {

unsigned long long transfer_keyset(
        LibFred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const std::string& _authinfopw,
        const TransferKeysetConfigData& _transfer_keyset_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const LibFred::InfoKeysetData keyset_data_before_transfer =
                LibFred::InfoKeysetByHandle(_keyset_handle)
                        .set_lock()
                        .exec(_ctx)
                        .info_keyset_data;

        const LibFred::InfoRegistrarData session_registrar =
                LibFred::InfoRegistrarById(_session_data.registrar_id)
                        .exec(_ctx)
                        .info_registrar_data;

        const bool is_sponsoring_registrar = (keyset_data_before_transfer.sponsoring_registrar_handle ==
                                              session_registrar.handle);

        if (is_sponsoring_registrar)
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
        }

        const bool is_system_registrar = session_registrar.system.get_value_or(false);
        if (!is_system_registrar)
        {
            // do it before any object state related checks
            LibFred::LockObjectStateRequestLock(keyset_data_before_transfer.id).exec(_ctx);
            LibFred::PerformObjectStateRequest(keyset_data_before_transfer.id).exec(_ctx);

            const LibFred::ObjectStatesInfo keyset_states_before_transfer(LibFred::GetObjectStates(
                            keyset_data_before_transfer.id).exec(_ctx));

            if (keyset_states_before_transfer.presents(LibFred::Object_State::server_transfer_prohibited) ||
                keyset_states_before_transfer.presents(LibFred::Object_State::delete_candidate))
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
            }
        }

        const unsigned long long post_transfer_history_id =
            LibFred::TransferKeyset(
                    keyset_data_before_transfer.id,
                    session_registrar.handle,
                    _authinfopw,
                    _session_data.logd_request_id.isset()
                    ? _session_data.logd_request_id.get_value()
                    : Nullable< unsigned long long >())
            .exec(_ctx);

        LibFred::Poll::CreatePollMessage<LibFred::Poll::MessageType::transfer_keyset>()
                .exec(_ctx, post_transfer_history_id);

        return post_transfer_history_id;

    }
    catch (const LibFred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        throw;
    }
    catch (const LibFred::UnknownKeysetId&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const LibFred::IncorrectAuthInfoPw&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const LibFred::NewRegistrarIsAlreadySponsoring&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }
}

} // namespace Epp::Keyset
} // namespace Epp
