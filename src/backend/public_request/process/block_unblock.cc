/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/public_request/process/block_unblock.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"
#include "src/backend/public_request/lock_request_type.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/backend/public_request/process/exceptions.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_block_unblock.hh"
#include "src/backend/public_request/util/send_joined_address_email.hh"
#include "libfred/object/object_state.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/object_state/typedefs.hh"
#include "libfred/public_request/info_public_request.hh"
#include "src/deprecated/libfred/public_request/public_request_impl.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/public_request_on_status_action.hh"
#include "libfred/public_request/update_public_request.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrar/info_registrar.hh"
#include "src/util/corba_wrapper_decl.hh"

#include <exception>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Process {

namespace {

template <LockRequestType::Enum _lock_request_type>
void subprocess(
        LibFred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const LibFred::ObjectStatesInfo& _object_states);

template <>
void subprocess<LockRequestType::block_transfer>(
        LibFred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const LibFred::ObjectStatesInfo& _object_states)
{
    if (_object_states.presents(LibFred::Object_State::server_transfer_prohibited))
    {
        throw ObjectAlreadyBlocked();
    }

    if (_object_states.presents(LibFred::Object_State::server_update_prohibited))
    {
        throw HasDifferentBlock(); // different from request impl src/backend/public_request/public_request.cc
    }

    LibFred::StatusList status_list;
    status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
    LibFred::CreateObjectStateRequestId(_object_id, status_list).exec(_ctx);
    LibFred::PerformObjectStateRequest(_object_id).exec(_ctx);
}

template <>
void subprocess<LockRequestType::block_transfer_and_update>(
        LibFred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const LibFred::ObjectStatesInfo& _object_states)
{
    if (_object_states.presents(LibFred::Object_State::server_transfer_prohibited) &&
        _object_states.presents(LibFred::Object_State::server_update_prohibited))
    {
        throw ObjectAlreadyBlocked();
    }

    // different from request impl src/backend/public_request/public_request.cc
    if (_object_states.presents(LibFred::Object_State::server_update_prohibited) &&
       !_object_states.presents(LibFred::Object_State::server_transfer_prohibited))
    {
        _ctx.get_log().warning(
                boost::format("Request to block_transfer_and_update: object with id %1% has inconsistent states: "
                              "server_update_prohibited without server_transfer_prohibited") %
                _object_id);
    }

    LibFred::StatusList status_list;
    status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
    status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_update_prohibited));
    LibFred::CreateObjectStateRequestId(_object_id, status_list).exec(_ctx);
    LibFred::PerformObjectStateRequest(_object_id).exec(_ctx);
}

template <>
void subprocess<LockRequestType::unblock_transfer>(
        LibFred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const LibFred::ObjectStatesInfo& _object_states)
{
    if (_object_states.presents(LibFred::Object_State::server_update_prohibited))
    {
        throw HasDifferentBlock();
    }

    if (!_object_states.presents(LibFred::Object_State::server_transfer_prohibited))
    {
        throw ObjectNotBlocked();
    }

    LibFred::StatusList status_list;
    status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
    LibFred::CancelObjectStateRequestId(_object_id, status_list).exec(_ctx);
    LibFred::PerformObjectStateRequest(_object_id).exec(_ctx);
}

template <>
void subprocess<LockRequestType::unblock_transfer_and_update>(
        LibFred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const LibFred::ObjectStatesInfo& _object_states)
{
    if (_object_states.absents(LibFred::Object_State::server_update_prohibited))
    {
        if (_object_states.presents(LibFred::Object_State::server_transfer_prohibited))
        {
            throw HasDifferentBlock();
        }
        throw ObjectNotBlocked();
    }

    LibFred::StatusList status_list;
    status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
    status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_update_prohibited));
    LibFred::CancelObjectStateRequestId(_object_id, status_list).exec(_ctx);
}

template <LockRequestType::Enum _lock_request_type>
void process(const LibFred::LockedPublicRequestForUpdate& _locked_request)
{
    auto& ctx = _locked_request.get_ctx();
    const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(ctx, _locked_request);
    const auto object_id = request_info.get_object_id().get_value(); // oops

    LibFred::LockObjectStateRequestLock(object_id).exec(ctx);
    const LibFred::ObjectStatesInfo _object_states(LibFred::GetObjectStates(object_id).exec(ctx));

    if (_object_states.presents(LibFred::Object_State::mojeid_contact) ||
        _object_states.presents(LibFred::Object_State::server_blocked))
    {
        throw OperationProhibited();
    }

    subprocess<_lock_request_type>(ctx, object_id, _object_states);

    LibFred::PerformObjectStateRequest(object_id).exec(ctx);
}

ObjectType convert_libfred_object_type_to_public_request_objecttype(LibFred::Object_Type::Enum libfred_object_type)
{
        switch (libfred_object_type)
        {
            case LibFred::Object_Type::contact: return ObjectType::contact;
            case LibFred::Object_Type::nsset: return ObjectType::nsset;
            case LibFred::Object_Type::domain: return ObjectType::domain;
            case LibFred::Object_Type::keyset: return ObjectType::keyset;
        }
        throw std::runtime_error("unexpected LibFred::Object_Type");
}

unsigned long long send_request_block_email(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager)
{
    auto& ctx = _locked_request.get_ctx();
    const auto public_request_id = _locked_request.get_id();
    const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(ctx, _locked_request);
    const auto object_id = request_info.get_object_id().get_value(); // oops
    const std::string sql_query =
            "SELECT obr.name, "
                   "eot.name "
            "FROM object_registry obr "
            "JOIN enum_object_type eot ON eot.id = obr.type "
            "WHERE obr.id = $1::BIGINT AND "
                  "obr.erdate IS NULL";
    const Database::Result db_result = ctx.get_conn().exec_params(
            sql_query,
            Database::query_param_list(object_id));
    if (db_result.size() < 1)
    {
        throw ObjectNotFound();
    }
    if (1 < db_result.size())
    {
        throw std::runtime_error("too many objects for given id");
    }
    const std::string handle = static_cast<std::string>(db_result[0][0]);
    const LibFred::Object_Type::Enum object_type =
            Conversion::Enums::from_db_handle<LibFred::Object_Type>(static_cast<std::string>(db_result[0][1]));

    LibFred::Mailer::Parameters email_template_params;
    {
        const Database::Result dbres = ctx.get_conn().exec_params(
                "SELECT (create_time AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague')::DATE "
                "FROM public_request "
                "WHERE id = $1::BIGINT",
                Database::query_param_list(public_request_id));
        if (dbres.size() < 1)
        {
            throw NoPublicRequest();
        }
        if (1 < dbres.size())
        {
            throw std::runtime_error{"too many public requests for given id"};
        }
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("reqid", std::to_string(public_request_id)));
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("reqdate", static_cast<std::string>(dbres[0][0])));
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("handle", handle));
    }

    std::set<std::string> emails;
    const auto email_to_answer = request_info.get_email_to_answer();
    if (!email_to_answer.isnull())
    {
        emails.insert(email_to_answer.get_value()); // validity checked when public_request was created
    }
    else
    {
        emails = get_valid_registry_emails_of_registered_object(ctx, convert_libfred_object_type_to_public_request_objecttype(object_type), object_id);
        if (emails.empty())
        {
            throw NoContactEmail();
        }
    }

    const Util::EmailData data(emails, "request_block", email_template_params, std::vector<unsigned long long>());
    return send_joined_addresses_email(_mailer_manager, data);
}

} // namespace Fred::Backend::PublicRequest::Process::{anonymous}

void process_public_request_block_unblock_resolved(
        unsigned long long _public_request_id,
        const LibFred::PublicRequestTypeIface& _public_request_type,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager)
{
    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);
        const auto email_id = send_request_block_email(locked_request, _mailer_manager);
        try
        {
            const std::string public_request_type = _public_request_type.get_public_request_type();

            if (public_request_type == Type::get_iface_of<Type::BlockTransfer<ConfirmedBy::email>>().get_public_request_type() ||
                public_request_type == Type::get_iface_of<Type::BlockTransfer<ConfirmedBy::letter>>().get_public_request_type() ||
                public_request_type == Type::get_iface_of<Type::BlockTransfer<ConfirmedBy::government>>().get_public_request_type())
            {
                process<LockRequestType::block_transfer>(locked_request);
            }
            else if (public_request_type == Type::get_iface_of<Type::BlockChanges<ConfirmedBy::email>>().get_public_request_type() ||
                     public_request_type == Type::get_iface_of<Type::BlockChanges<ConfirmedBy::letter>>().get_public_request_type() ||
                     public_request_type == Type::get_iface_of<Type::BlockChanges<ConfirmedBy::government>>().get_public_request_type())
            {
                process<LockRequestType::block_transfer_and_update>(locked_request);
            }
            else if (public_request_type == Type::get_iface_of<Type::UnblockTransfer<ConfirmedBy::email>>().get_public_request_type() ||
                     public_request_type == Type::get_iface_of<Type::UnblockTransfer<ConfirmedBy::letter>>().get_public_request_type() ||
                     public_request_type == Type::get_iface_of<Type::UnblockTransfer<ConfirmedBy::government>>().get_public_request_type())
            {
                process<LockRequestType::unblock_transfer>(locked_request);
            }
            else if (public_request_type == Type::get_iface_of<Type::UnblockChanges<ConfirmedBy::email>>().get_public_request_type() ||
                     public_request_type == Type::get_iface_of<Type::UnblockChanges<ConfirmedBy::letter>>().get_public_request_type() ||
                     public_request_type == Type::get_iface_of<Type::UnblockChanges<ConfirmedBy::government>>().get_public_request_type())
            {
                process<LockRequestType::unblock_transfer_and_update>(locked_request);
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
                .set_answer_email_id(email_id)
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

} // namespace Fred::Backend::PublicRequest::Process
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
