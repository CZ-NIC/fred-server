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

#include "src/backend/public_request/create_block_unblock_request.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/get_id_of_registered_object.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_block_unblock.hh"
#include "src/libfred/object/get_id_of_registered.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/util/log/context.hh"
#include "src/util/optional_value.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

std::string create_ctx_function_name(const char *fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const std::string &_op_name)
        : ctx_operation_(_op_name)
    { }
private:
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(create_ctx_function_name(__FUNCTION__))

} // namespace Fred::Backend::PublicRequest::{anonymous}

unsigned long long create_block_unblock_request(
        ObjectType object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmedBy confirmation_method,
        LockRequestType::Enum lock_request_type)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        LibFred::OperationContextCreator ctx;
        const unsigned long long object_id = get_id_of_registered_object(ctx, object_type, object_handle);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(ctx));
        if (states.presents(LibFred::Object_State::mojeid_contact) ||
            states.presents(LibFred::Object_State::server_blocked))
        {
            throw OperationProhibited();
        }
        switch (lock_request_type)
        {
            case LockRequestType::block_transfer:
            {
                if (states.presents(LibFred::Object_State::server_transfer_prohibited))
                {
                    throw ObjectAlreadyBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::BlockTransfer>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::block_transfer_and_update:
            {
                if (states.presents(LibFred::Object_State::server_transfer_prohibited) &&
                    states.presents(LibFred::Object_State::server_update_prohibited))
                {
                    throw ObjectAlreadyBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::BlockChanges>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::unblock_transfer:
            {
                if (states.presents(LibFred::Object_State::server_update_prohibited))
                {
                    throw HasDifferentBlock();
                }
                if (states.absents(LibFred::Object_State::server_transfer_prohibited))
                {
                    throw ObjectNotBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::UnblockTransfer>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::unblock_transfer_and_update:
            {
                if (states.absents(LibFred::Object_State::server_update_prohibited))
                {
                    if (states.presents(LibFred::Object_State::server_transfer_prohibited))
                    {
                        throw HasDifferentBlock();
                    }
                    throw ObjectNotBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::UnblockChanges>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
        }
        throw std::runtime_error("unexpected lock request type");
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const OperationProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const ObjectAlreadyBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const ObjectNotBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const HasDifferentBlock& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const LibFred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).info(boost::diagnostic_information(e));
            throw InvalidContactEmail();
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
