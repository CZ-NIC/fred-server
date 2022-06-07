/*
 * Copyright (C) 2018-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/public_request/create_authinfo_request_registry_email.hh"

#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/get_id_of_registered_object.hh"
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"
#include "src/backend/public_request/lock_request_type.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"

#include "libfred/object/get_id_of_registered.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/opcontext.hh"
#include "libfred/public_request/create_public_request.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/update_public_request.hh"

#include "util/log/context.hh"
#include "util/log/logger.hh"
#include "util/optional_value.hh"

#include <algorithm>

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

unsigned long long create_authinfo_request_registry_email(
        ObjectType object_type,
        const std::string& object_handle,
        unsigned long long registrar_id,
        const Optional<unsigned long long>& log_request_id)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto object_id = get_id_of_registered_object(ctx, object_type, object_handle);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(ctx));
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        if (states.presents(LibFred::Object_State::server_transfer_prohibited))
        {
            if (object_type != ObjectType::contact)
            {
                throw ObjectTransferProhibited{};
            }
            LOGGER.info("authinfo requested for contact blocked against transfer");
        }
        const auto emails = get_valid_registry_emails_of_registered_object(ctx, object_type, object_id);
        if (emails.empty())
        {
            throw NoContactEmail();
        }
        const auto public_request_id = LibFred::CreatePublicRequest()
                .set_registrar_id(registrar_id)
                .exec(locked_object, Type::get_iface_of<Type::AuthinfoAuto>(), log_request_id);
        LibFred::UpdatePublicRequest()
            .set_status(LibFred::PublicRequest::Status::resolved)
            .exec(locked_object, Type::get_iface_of<Type::AuthinfoAuto>(), log_request_id);
        ctx.commit_transaction();

        return public_request_id;
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER.info(e.what());
        throw ObjectNotFound();
    }
    catch (const ObjectTransferProhibited& e)
    {
        LOGGER.info(e.what());
        throw;
    }
    catch (const NoContactEmail& e)
    {
        LOGGER.info(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("create_authinfo_request (registry) failed due to an unknown exception");
        throw;
    }
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
