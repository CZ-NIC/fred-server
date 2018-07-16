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

#include "src/backend/public_request/process_public_request_authinfo.hh"

#include "src/backend/public_request/public_request.hh"
#include "src/backend/public_request/send_email.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/util/corba_wrapper_decl.hh"

#include <array>

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

unsigned long long send_authinfo(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager)
{
    auto& ctx = _locked_request.get_ctx();
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
    const LibFred::Object_Type::Enum object_type = Conversion::Enums::from_db_handle<LibFred::Object_Type>(static_cast<std::string>(db_result[0]["object_type"]));

    LibFred::Mailer::Parameters email_template_params;
    {
        const Database::Result dbres = ctx.get_conn().exec_params(
                "SELECT (create_time AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague')::DATE FROM public_request "
                "WHERE id=$1::BIGINT",
                Database::query_param_list(_public_request_id));
        if (dbres.size() < 1)
        {
            throw NoPublicRequest();
        }
        if (1 < dbres.size())
        {
            throw std::runtime_error("too many public requests for given id");
        }
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("reqid", boost::lexical_cast<std::string>(_public_request_id)));
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("reqdate", static_cast<std::string>(dbres[0][0])));
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("handle", handle));
    }

    std::string sql;
    std::string object_type_handle;
    switch (object_type)
    {
        case ObjectType::contact:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN contact c ON c.id=o.id "
                  "WHERE obr.name=UPPER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>''";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::contact);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "1"));
            break;
        case ObjectType::nsset:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN nsset n ON n.id=o.id "
                  "JOIN nsset_contact_map ncm ON ncm.nssetid=n.id "
                  "JOIN contact c ON c.id=ncm.contactid "
                  "WHERE obr.name=UPPER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>''";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::nsset);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "2"));
            break;
        case ObjectType::domain:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN domain d ON d.id=o.id "
                  "JOIN contact c ON c.id=d.registrant "
                  "WHERE obr.name=LOWER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>'' "
              "UNION "
                  "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN domain d ON d.id=o.id "
                  "JOIN domain_contact_map dcm ON dcm.domainid=d.id "
                  "JOIN contact c ON c.id=dcm.contactid AND c.id!=d.registrant "
                  "WHERE obr.name=LOWER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>'' AND "
                        "dcm.role=1";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::domain);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "3"));
            break;
        case ObjectType::keyset:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN keyset k ON k.id=o.id "
                  "JOIN keyset_contact_map kcm ON kcm.keysetid=k.id "
                  "JOIN contact c ON c.id=kcm.contactid "
                  "WHERE obr.name=UPPER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>''";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::keyset);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "4"));
            break;
    }
    const Database::Result dbres = _ctx.get_conn().exec_params(
            sql,
            Database::query_param_list(handle)(object_type_handle));
    if (dbres.size() < 1)
    {
        throw NoContactEmail();
    }

    email_template_params.insert(LibFred::Mailer::Parameters::value_type("authinfo", static_cast<std::string>(dbres[0][0])));
    std::set<std::string> recipients;
    for (unsigned idx = 0; idx < dbres.size(); ++idx)
    {
        recipients.insert(static_cast<std::string>(dbres[idx][1]));
    }
    const EmailData data(recipients, "sendauthinfo_pif", email_template_params, std::vector<unsigned long long>());
    return send_joined_addresses_email(_mailer_manager, data);
}

} // namespace Fred::Backend::PublicRequest::{anonymous}

void process_public_request_auth_info_resolved(
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

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
