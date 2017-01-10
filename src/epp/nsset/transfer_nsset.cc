/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/nsset/transfer_nsset.h"

#include "src/epp/error.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/transfer_nsset.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/poll/create_epp_action_poll_message_impl.h"
#include "src/fredlib/poll/message_types.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <boost/foreach.hpp>

#include <set>
#include <string>

namespace Epp {
namespace Nsset {

unsigned long long transfer_nsset(
        Fred::OperationContext& _ctx,
        const std::string& _nsset_handle,
        const std::string& _authinfopw,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    if ( Fred::Nsset::get_handle_registrability(_ctx, _nsset_handle) != Fred::NssetHandleState::Registrability::registered ) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    const Fred::InfoNssetData nsset_data_before_transfer = Fred::InfoNssetByHandle(_nsset_handle).set_lock().exec(_ctx).info_nsset_data;

    const std::string session_registrar_handle = Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data.handle;

    if (nsset_data_before_transfer.sponsoring_registrar_handle == session_registrar_handle) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(nsset_data_before_transfer.id).exec(_ctx);
    Fred::PerformObjectStateRequest(nsset_data_before_transfer.id).exec(_ctx);

    const Fred::ObjectStatesInfo nsset_states_before_transfer(Fred::GetObjectStates(nsset_data_before_transfer.id).exec(_ctx));

    if (nsset_states_before_transfer.presents(Fred::Object_State::server_transfer_prohibited) ||
        nsset_states_before_transfer.presents(Fred::Object_State::delete_candidate))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
    }

    try {
        unsigned long long post_transfer_history_id =
            Fred::TransferNsset(
                    nsset_data_before_transfer.id,
                    session_registrar_handle,
                    _authinfopw,
                    _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable<unsigned long long>())
            .exec(_ctx);

        Fred::Poll::CreateEppActionPollMessage(post_transfer_history_id,
                                               Fred::Poll::nsset,
                                               Fred::Poll::TRANSFER_NSSET).exec(_ctx);

        return post_transfer_history_id;

    }
    catch (const Fred::UnknownNssetId&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const Fred::IncorrectAuthInfoPw&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }
}

} // namespace Epp::Nsset
} // namespace Epp
