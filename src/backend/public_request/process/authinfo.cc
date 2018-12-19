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

#include "src/backend/public_request/process/authinfo.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/backend/public_request/process/exceptions.hh"
#include "src/backend/public_request/util/send_joined_address_email.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/object/object_type.hh"
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/domain/info_domain.hh"
#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrable_object/nsset/info_nsset.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/util/corba_wrapper_decl.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Process {

namespace {

ObjectType convert_libfred_object_type_to_public_request_objecttype(
        const LibFred::Object_Type::Enum _libfred_object_type)
{
        switch (_libfred_object_type)
        {
            case LibFred::Object_Type::contact:
                return ObjectType::contact;

            case LibFred::Object_Type::nsset:
                return ObjectType::nsset;

            case LibFred::Object_Type::domain:
                return ObjectType::domain;

            case LibFred::Object_Type::keyset:
                return ObjectType::keyset;

        }
        throw std::runtime_error("unexpected LibFred::Object_Type");
}


unsigned long long send_authinfo(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager)
{
    auto& ctx = _locked_request.get_ctx();
    const auto public_request_id = _locked_request.get_id();
    const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(ctx, _locked_request);
    const auto object_id = request_info.get_object_id().get_value(); // oops
    // clang-format off
    const std::string sql_query =
            "SELECT oreg.name AS handle, eot.name AS object_type "
              "FROM object_registry oreg "
              "JOIN enum_object_type eot "
                "ON oreg.type = eot.id "
             "WHERE oreg.id = $1::BIGINT "
               "AND oreg.erdate IS NULL";
    // clang-format on
    const Database::Result db_result = ctx.get_conn().exec_params(
            sql_query,
            Database::query_param_list(object_id));
    if (db_result.size() < 1)
    {
        throw ObjectNotFound();
    }
    if (db_result.size() > 1)
    {
        throw std::runtime_error("too many objects for given id");
    }
    const std::string handle = static_cast<std::string>(db_result[0]["handle"]);
    const LibFred::Object_Type::Enum object_type =
            Conversion::Enums::from_db_handle<LibFred::Object_Type>(static_cast<std::string>(db_result[0]["object_type"]));

    LibFred::Mailer::Parameters email_template_params;
    {
        const Database::Result dbres = ctx.get_conn().exec_params(
                "SELECT (create_time AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague')::DATE FROM public_request "
                "WHERE id=$1::BIGINT",
                Database::query_param_list(public_request_id));
        if (dbres.size() < 1)
        {
            throw NoPublicRequest();
        }
        if (1 < dbres.size())
        {
            throw std::runtime_error("too many public requests for given id");
        }
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("reqid", boost::lexical_cast<std::string>(public_request_id)));
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

    std::string type;
    std::string authinfo;
    switch (object_type)
    {
        case LibFred::Object_Type::contact:
            type = "1";
            authinfo = LibFred::InfoContactById(object_id).exec(ctx).info_contact_data.authinfopw;
            break;
        case LibFred::Object_Type::nsset:
            type = "2";
            authinfo = LibFred::InfoNssetById(object_id).exec(ctx).info_nsset_data.authinfopw;
            break;
        case LibFred::Object_Type::domain:
            type = "3";
            authinfo = LibFred::InfoDomainById(object_id).exec(ctx).info_domain_data.authinfopw;
            break;
        case LibFred::Object_Type::keyset:
            type = "4";
            authinfo = LibFred::InfoKeysetById(object_id).exec(ctx).info_keyset_data.authinfopw;
            break;
    }
    email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", type));
    email_template_params.insert(LibFred::Mailer::Parameters::value_type("authinfo", authinfo));

    const Util::EmailData data(emails, "sendauthinfo_pif", email_template_params, std::vector<unsigned long long>());
    return send_joined_addresses_email(_mailer_manager, data);
}

} // namespace Fred::Backend::PublicRequest::Process::{anonymous}

void process_public_request_authinfo_resolved(
        unsigned long long _public_request_id,
        const LibFred::PublicRequestTypeIface& _public_request_type,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager)
{
    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);
        const unsigned long long email_id = send_authinfo(locked_request, _mailer_manager);
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
                    boost::format("Request %1% update failed (%2%), but email %3% sent") %
                    _public_request_id %
                    e.what() %
                    email_id);
        }
        catch (...)
        {
            ctx.get_log().info(
                    boost::format("Request %1% update failed (unknown exception), but email %2% sent") %
                    _public_request_id %
                    email_id);
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
