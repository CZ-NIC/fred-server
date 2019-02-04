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

#include "src/backend/epp/keyset/delete_keyset.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/keyset/status_value.hh"
#include "libfred/registrable_object/keyset/delete_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/object/object_state.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrar/info_registrar.hh"

#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

namespace {

bool presents(
        const std::set<LibFred::Object_State::Enum>& _state,
        LibFred::Object_State::Enum _flag)
{
    return _state.find(_flag) != _state.end();
}

} // namespace Epp::Keyset::{anonymous}

unsigned long long delete_keyset(
        LibFred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const DeleteKeysetConfigData& _delete_keyset_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const LibFred::InfoKeysetData keyset_data = LibFred::InfoKeysetByHandle(_keyset_handle).set_lock()
                                                 .exec(_ctx).info_keyset_data;
        const LibFred::InfoRegistrarData session_registrar =
                LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;
        const bool is_sponsoring_registrar =
                (keyset_data.sponsoring_registrar_handle == session_registrar.handle);
        const bool is_system_registrar = session_registrar.system.get_value_or(false);
        const bool is_registrar_authorized = (is_system_registrar || is_sponsoring_registrar);
        if (!is_registrar_authorized)
        {
            _ctx.get_log().info("keyset_delete failure: registrar not authorized for this operation");
            throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error)
                                             .add_extended_error(
                                                     EppExtendedError::of_scalar_parameter(
                                                             Param::registrar_autor,
                                                             Reason::unauthorized_registrar)));
        }

        // do it before any object state related checks
        LibFred::LockObjectStateRequestLock(keyset_data.id).exec(_ctx);
        LibFred::PerformObjectStateRequest(keyset_data.id).exec(_ctx);

        typedef std::vector<LibFred::ObjectStateData> StatesData;

        StatesData states_data = LibFred::GetObjectStates(keyset_data.id).exec(_ctx);

        typedef std::set<LibFred::Object_State::Enum> ObjectStates;

        ObjectStates keyset_states;
        for (StatesData::const_iterator data_ptr = states_data.begin();
             data_ptr != states_data.end();
             ++data_ptr)
        {
            keyset_states.insert(Conversion::Enums::from_db_handle<LibFred::Object_State>(data_ptr->state_name));
        }
        if (!is_system_registrar && (presents(keyset_states, LibFred::Object_State::server_update_prohibited) ||
                                     presents(keyset_states, LibFred::Object_State::server_delete_prohibited) ||
                                     presents(keyset_states, LibFred::Object_State::delete_candidate)))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }

        if (presents(keyset_states, LibFred::Object_State::linked))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_association_prohibits_operation));
        }

        LibFred::DeleteKeysetByHandle(_keyset_handle).exec(_ctx);

        return keyset_data.historyid;
    }
    catch (const LibFred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        throw;
    }
    catch (const LibFred::DeleteKeysetByHandle::Exception& e)
    {

        // general errors (possibly but not NECESSARILLY caused by input data) signaling unknown/bigger problems have priority
        if (e.is_set_unknown_keyset_handle())
        {
            throw;
        }

        if (e.is_set_object_linked_to_keyset_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_association_prohibits_operation));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}


} // namespace Epp::Keyset
} // namespace Epp
