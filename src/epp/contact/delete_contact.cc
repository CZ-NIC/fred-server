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

#include "src/epp/contact/delete_contact.h"

#include "src/epp/exception.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object/object_states_info.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/epp/impl/util.h"

namespace Epp {
namespace Contact {

unsigned long long delete_contact(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const DeleteContactConfigData& _delete_contact_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    if ( Fred::Contact::get_handle_registrability(_ctx, _contact_handle) != Fred::ContactHandleState::Registrability::registered ) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    const Fred::InfoContactData contact_data_before_delete =
            Fred::InfoContactByHandle(_contact_handle)
                    .set_lock()
                    .exec(_ctx)
                    .info_contact_data;

    const Fred::InfoRegistrarData session_registrar =
            Fred::InfoRegistrarById(_session_data.registrar_id)
                    .exec(_ctx)
                    .info_registrar_data;

    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_sponsoring_registrar = (contact_data_before_delete.sponsoring_registrar_handle ==
                                          session_registrar.handle);

    const bool is_operation_permitted = (is_system_registrar || is_sponsoring_registrar);

    if (!is_operation_permitted) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::registrar_autor,
                                                         Reason::unauthorized_registrar)));
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(contact_data_before_delete.id).exec(_ctx);
    Fred::PerformObjectStateRequest(contact_data_before_delete.id).exec(_ctx);

    const Fred::ObjectStatesInfo contact_states_before_delete(Fred::GetObjectStates(contact_data_before_delete.id).exec(_ctx));
    if (contact_states_before_delete.presents(Fred::Object_State::linked))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_association_prohibits_operation));
    }

    if (!is_system_registrar) {
        if (contact_states_before_delete.presents(Fred::Object_State::server_update_prohibited) ||
            contact_states_before_delete.presents(Fred::Object_State::server_delete_prohibited) ||
            contact_states_before_delete.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    try {

        Fred::DeleteContactByHandle(_contact_handle).exec(_ctx);

        return contact_data_before_delete.historyid;

    }
    catch(const Fred::DeleteContactByHandle::Exception& e) {

        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if ( e.is_set_unknown_contact_handle() ) {
            throw;
        }

        if ( e.is_set_object_linked_to_contact_handle() ) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Contact
} // namespace Epp
