/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/backend/public_request/process_public_request_block_unblock.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/libfred/object/object_state.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/object_state/cancel_object_state_request_id.hh"
#include "src/libfred/object_state/create_object_state_request_id.hh"
#include "src/libfred/object_state/lock_object_state_request_lock.hh"
#include "src/libfred/object_state/perform_object_state_request.hh"
#include "src/libfred/object_state/typedefs.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/libfred/public_request/public_request_impl.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/util/corba_wrapper_decl.hh"

#include <exception>

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

struct LockRequestType
{
    enum Enum
    {
        block_transfer,
        block_transfer_and_update,
        unblock_transfer,
        unblock_transfer_and_update
    };
};

void block_unblock(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        LockRequestType::Enum _lock_request_type)
{
    auto& ctx = _locked_request.get_ctx();
    const auto public_request_id = _locked_request.get_id();
    const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(ctx, _locked_request);
    const auto object_id = request_info.get_object_id().get_value(); // oops

    LibFred::LockObjectStateRequestLock(object_id).exec(ctx);
    const LibFred::ObjectStatesInfo object_states(LibFred::GetObjectStates(object_id).exec(ctx));

    if (object_states.presents(LibFred::Object_State::mojeid_contact) ||
        object_states.presents(LibFred::Object_State::server_blocked))
    {
        throw OperationProhibited();
    }

    switch (_lock_request_type)
    {
        case LockRequestType::block_transfer:
        {
            if (object_states.presents(LibFred::Object_State::server_transfer_prohibited))
            {
                throw ObjectAlreadyBlocked();
            }

            if (object_states.presents(LibFred::Object_State::server_update_prohibited))
            {
                throw HasDifferentBlock(); // different from request impl src/backend/public_request/public_request.cc
            }

            LibFred::StatusList status_list;
            status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
            LibFred::CreateObjectStateRequestId(object_id, status_list).exec(ctx);
            LibFred::PerformObjectStateRequest(object_id).exec(ctx);
        }
        case LockRequestType::block_transfer_and_update:
        {
            if (object_states.presents(LibFred::Object_State::server_transfer_prohibited) &&
                object_states.presents(LibFred::Object_State::server_update_prohibited))
            {
                throw ObjectAlreadyBlocked();
            }

            // different from request impl src/backend/public_request/public_request.cc
            if (object_states.presents(LibFred::Object_State::server_update_prohibited) &&
               !object_states.presents(LibFred::Object_State::server_transfer_prohibited))
            {
                ctx.get_log().warning(
                        boost::format("Request %1% inconsistent states: server_update_prohibited without server_transfer_prohibited") %
                        public_request_id);
            }

            LibFred::StatusList status_list;
            status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
            status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_update_prohibited));
            LibFred::CreateObjectStateRequestId(object_id, status_list).exec(ctx);
            LibFred::PerformObjectStateRequest(object_id).exec(ctx);
        }
        case LockRequestType::unblock_transfer:
        {
            if (object_states.presents(LibFred::Object_State::server_update_prohibited))
            {
                throw HasDifferentBlock();
            }

            if (!object_states.presents(LibFred::Object_State::server_transfer_prohibited))
            {
                throw ObjectNotBlocked();
            }

            LibFred::StatusList status_list;
            status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
            LibFred::CancelObjectStateRequestId(object_id, status_list).exec(ctx);
            LibFred::PerformObjectStateRequest(object_id).exec(ctx);
        }
        case LockRequestType::unblock_transfer_and_update:
        {

            if (object_states.absents(LibFred::Object_State::server_update_prohibited))
            {
                if (object_states.presents(LibFred::Object_State::server_transfer_prohibited))
                {
                    throw HasDifferentBlock();
                }
                throw ObjectNotBlocked();
            }
            LibFred::StatusList status_list;
            status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
            status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_update_prohibited));
            LibFred::CancelObjectStateRequestId(object_id, status_list).exec(ctx);
            LibFred::PerformObjectStateRequest(object_id).exec(ctx);
        }
    }
    throw std::logic_error("unexpected lock request type");
}

} // namespace Fred::Backend::PublicRequest::{anonymous}

void process_public_request_block_unblock_resolved(
        unsigned long long _public_request_id,
        const LibFred::PublicRequestTypeIface& _public_request_type)
{
    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);

        try
        {
            const std::string public_request_type = _public_request_type.get_public_request_type();

            if (public_request_type == "block_transfer_email_pif" ||
                public_request_type == "block_transfer_post_pif")
            {
                block_unblock(locked_request, LockRequestType::block_transfer);
            }
            else if (public_request_type == "block_changes_email_pif" ||
                     public_request_type == "block_changes_post_pif")
            {
                block_unblock(locked_request, LockRequestType::block_transfer_and_update);
            }
            else if (public_request_type == "unblock_transfer_email_pif" ||
                     public_request_type == "unblock_transfer_post_pif")
            {
                block_unblock(locked_request, LockRequestType::unblock_transfer);
            }
            else if (public_request_type == "unblock_changes_email_pif" ||
                     public_request_type == "unblock_changes_post_pif")
            {
                block_unblock(locked_request, LockRequestType::unblock_transfer_and_update);
            }
        }
        catch (const ObjectAlreadyBlocked& e)
        {
            ctx.get_log().info(
                    boost::format("Request %1% no action needed (%2%)") %
                    _public_request_id %
                    e.what());
        }
        catch (const ObjectNotBlocked& e)
        {
            ctx.get_log().info(
                    boost::format("Request %1% no action needed (%2%)") %
                    _public_request_id %
                    e.what());
        }

        try
        {
            LibFred::UpdatePublicRequest()
                .set_on_status_action(LibFred::PublicRequest::OnStatusAction::processed)
                .exec(locked_request, _public_request_type);
            ctx.commit_transaction();
        }
        catch (const std::exception& e)
        {
            ctx.get_log().info(
                    boost::format("Request %1% update failed (%2%)") %
                    _public_request_id %
                    e.what());
        }
        catch (...)
        {
            ctx.get_log().info(
                    boost::format("Request %1% update failed (unknown exception)") %
                    _public_request_id);
        }
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);
        LibFred::UpdatePublicRequest()
            .set_on_status_action(LibFred::PublicRequest::OnStatusAction::failed)
            .exec(locked_request, _public_request_type);
        ctx.commit_transaction();
        throw;
    }
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
