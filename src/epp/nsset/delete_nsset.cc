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

#include "src/epp/nsset/delete_nsset.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/nsset.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/delete_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/object/object_states_info.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar_data.h"

namespace Epp {
namespace Nsset {

unsigned long long delete_nsset(
        Fred::OperationContext& _ctx,
        const std::string& _nsset_handle,
        const DeleteNssetConfigData& _delete_nsset_config_data,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    Fred::InfoNssetData nsset_data_before_delete;

    try
    {
        nsset_data_before_delete =
                Fred::InfoNssetByHandle(_nsset_handle)
                        .set_lock()
                        .exec(_ctx)
                        .info_nsset_data;
    }
    catch(const Fred::InfoNssetByHandle::Exception& ex)
    {
        if(ex.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
    }

    const Fred::InfoRegistrarData session_registrar =
            Fred::InfoRegistrarById(_session_data.registrar_id)
                    .exec(_ctx)
                    .info_registrar_data;

    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_sponsoring_registrar = (nsset_data_before_delete.sponsoring_registrar_handle ==
                                          session_registrar.handle);

    const bool is_registrar_authorized = (is_system_registrar || is_sponsoring_registrar);

    if (!is_registrar_authorized)
    {
        throw EppResponseFailure(
                EppResultFailure(EppResultCode::authorization_error)
                .add_extended_error(
                        EppExtendedError::of_scalar_parameter(
                                Param::registrar_autor,
                                Reason::unauthorized_registrar)));
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(nsset_data_before_delete.id).exec(_ctx);
    Fred::PerformObjectStateRequest(nsset_data_before_delete.id).exec(_ctx);

    const Fred::ObjectStatesInfo nsset_states_before_delete(Fred::GetObjectStates(
                    nsset_data_before_delete.id).exec(_ctx));
    if (!is_system_registrar)
    {
        if (nsset_states_before_delete.presents(Fred::Object_State::server_update_prohibited) ||
            nsset_states_before_delete.presents(Fred::Object_State::server_delete_prohibited) ||
            nsset_states_before_delete.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    if (nsset_states_before_delete.presents(Fred::Object_State::linked))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_association_prohibits_operation));
    }

    try
    {
        Fred::DeleteNssetByHandle(_nsset_handle).exec(_ctx);
        return nsset_data_before_delete.historyid;
    }
    catch (const Fred::DeleteNssetByHandle::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_nsset_handle())
        {
            throw;
        }

        if (e.is_set_object_linked_to_nsset_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_association_prohibits_operation));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

}//namespace Epp::Nsset
}//namespace Epp
