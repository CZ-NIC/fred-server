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
#include "src/backend/epp/contact/delete_contact.hh"

#include "src/backend/epp/exception.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "src/backend/epp/impl/util.hh"

namespace Epp {
namespace Contact {

unsigned long long delete_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const DeleteContactConfigData& _delete_contact_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    LibFred::InfoContactData contact_data_before_delete;

    try
    {
        contact_data_before_delete =
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

    const LibFred::InfoRegistrarData session_registrar =
            LibFred::InfoRegistrarById(_session_data.registrar_id)
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
    LibFred::LockObjectStateRequestLock(contact_data_before_delete.id).exec(_ctx);
    LibFred::PerformObjectStateRequest(contact_data_before_delete.id).exec(_ctx);

    const LibFred::ObjectStatesInfo contact_states_before_delete(LibFred::GetObjectStates(contact_data_before_delete.id).exec(_ctx));
    if (contact_states_before_delete.presents(LibFred::Object_State::linked))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_association_prohibits_operation));
    }

    if (!is_system_registrar) {
        if (contact_states_before_delete.presents(LibFred::Object_State::server_update_prohibited) ||
            contact_states_before_delete.presents(LibFred::Object_State::server_delete_prohibited) ||
            contact_states_before_delete.presents(LibFred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    try {

        LibFred::DeleteContactByHandle(_contact_handle).exec(_ctx);

        return contact_data_before_delete.historyid;

    }
    catch(const LibFred::DeleteContactByHandle::Exception& e) {

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
