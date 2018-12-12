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

#include "src/backend/public_request/create_personal_info_request_registry_email.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/get_id_of_registered_object.hh"
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
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

unsigned long long create_personal_info_request_registry_email(
        const std::string& contact_handle,
        const Optional<unsigned long long>& log_request_id)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto contact_id = get_id_of_registered_object(ctx, ObjectType::contact, contact_handle);
        const auto emails = get_valid_registry_emails_of_registered_object(ctx, ObjectType::contact, contact_id);
        if (emails.empty())
        {
            throw NoContactEmail();
        }
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, contact_id);
        const auto public_request_id = LibFred::CreatePublicRequest()
            .exec(locked_object, Type::get_iface_of<Type::PersonalInfoAuto>(), log_request_id);
        LibFred::UpdatePublicRequest()
            .set_status(LibFred::PublicRequest::Status::resolved)
            .exec(locked_object, Type::get_iface_of<Type::PersonalInfoAuto>(), log_request_id);
        ctx.commit_transaction();

        return public_request_id;
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const NoContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw NoContactEmail();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("create_personal_info_request (registry) failed due to an unknown exception");
        throw;
    }
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
