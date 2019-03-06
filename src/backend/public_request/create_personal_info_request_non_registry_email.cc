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
#include "src/backend/public_request/create_personal_info_request_non_registry_email.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
#include "libfred/object/get_id_of_registered.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/opcontext.hh"
#include "libfred/public_request/create_public_request.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/update_public_request.hh"
#include "util/log/context.hh"
#include "util/optional_value.hh"

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

unsigned long long create_personal_info_request_non_registry_email(
        const std::string& contact_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmedBy confirmation_method,
        const std::string& specified_email)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto contact_id = LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, contact_handle);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, contact_id);
        const auto create_public_request_op = LibFred::CreatePublicRequest().set_email_to_answer(specified_email);
        switch (confirmation_method)
        {
            case ConfirmedBy::email:
            {
                const unsigned long long request_id =
                    create_public_request_op.exec(locked_object, Type::get_iface_of<Type::PersonalInfoEmail>(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case ConfirmedBy::letter:
            {
                const unsigned long long request_id =
                    create_public_request_op.exec(locked_object, Type::get_iface_of<Type::PersonalInfoPost>(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case ConfirmedBy::government:
            {
                const unsigned long long request_id =
                    create_public_request_op.exec(locked_object, Type::get_iface_of<Type::PersonalInfoGovernment>(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
        }
        throw std::runtime_error("unexpected confirmation method");
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER.info(e.what());
        throw ObjectNotFound();
    }
    catch (const LibFred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER.info(boost::diagnostic_information(e));
            throw InvalidContactEmail();
        }
        LOGGER.error(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("create_personal_info_request (non registry) failed due to an unknown exception");
        throw;
    }
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
