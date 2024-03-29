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
#include "src/backend/epp/public_request/create_authinfo_request_registry_email.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/backend/public_request/get_id_of_registered_object.hh"
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"

#include "libfred/object/get_id_of_registered.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/public_request/create_public_request.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/update_public_request.hh"

#include "util/log/context.hh"
#include "util/log/logger.hh"
#include "util/optional_value.hh"

namespace Epp {
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

} // namespace Epp::PublicRequest::{anonymous}

unsigned long long create_authinfo_request_registry_email(
        LibFred::OperationContext& _ctx,
        Fred::Backend::PublicRequest::ObjectType _object_type,
        const std::string& _object_handle,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _log_request_id)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        const auto object_id = Fred::Backend::PublicRequest::get_id_of_registered_object(_ctx, _object_type, _object_handle);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(_ctx));
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(_ctx, object_id);
        if (states.presents(LibFred::Object_State::server_transfer_prohibited))
        {
            if (_object_type != Fred::Backend::PublicRequest::ObjectType::contact)
            {
                throw Fred::Backend::PublicRequest::ObjectTransferProhibited{};
            }
            LOGGER.info("authinfo requested for contact blocked against transfer");
        }
        const auto emails = get_valid_registry_emails_of_registered_object(_ctx, _object_type, object_id);
        if (emails.empty())
        {
            throw Fred::Backend::PublicRequest::NoContactEmail();
        }
        const auto public_request_id = LibFred::CreatePublicRequest()
                .set_registrar_id(LibFred::RegistrarId(_registrar_id))
                .exec(locked_object,
                      Fred::Backend::PublicRequest::Type::get_iface_of<Fred::Backend::PublicRequest::Type::AuthinfoAutoRif>(),
                      _log_request_id);
        LibFred::UpdatePublicRequest()
            .set_status(LibFred::PublicRequest::Status::resolved)
            .exec(locked_object,
                  Fred::Backend::PublicRequest::Type::get_iface_of<Fred::Backend::PublicRequest::Type::AuthinfoAutoRif>(),
                  _log_request_id);

        return public_request_id;
    }
    catch (const Fred::Backend::PublicRequest::ObjectTransferProhibited& e)
    {
        LOGGER.info(e.what());
        throw;
    }
    catch (const Fred::Backend::PublicRequest::NoContactEmail& e)
    {
        LOGGER.info(e.what());
        throw;
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER.info(e.what());
        throw Fred::Backend::PublicRequest::ObjectNotFound();
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

} // namespace Epp::PublicRequest
} // namespace Epp
