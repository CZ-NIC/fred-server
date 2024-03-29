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

#include "src/backend/public_request/process/authinfo.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/backend/public_request/process/exceptions.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/util/make_object_type.hh"
#include "src/backend/public_request/util/get_public_request_uuid.hh"
#include "src/backend/public_request/util/send_joined_address_email.hh"
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/corba_wrapper_decl.hh"

#include "libfred/object/object_states_info.hh"
#include "libfred/object/object_type.hh"
#include "libfred/object/store_authinfo.hh"
#include "libfred/object/generate_authinfo_password.hh"
#include "libfred/public_request/info_public_request.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/public_request_on_status_action.hh"
#include "libfred/public_request/update_public_request.hh"
#include "libfred/registrar/info_registrar.hh"

#include "libhermes/struct.hh"

#include <boost/uuid/string_generator.hpp>

#include <utility>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Process {

namespace {

enum class EmailType
{
    sendauthinfo_pif,
    sendauthinfo_epp
};

std::string get_email_type_name(EmailType email_type)
{
    switch (email_type)
    {
        case EmailType::sendauthinfo_pif: return "sendauthinfo_pif";
        case EmailType::sendauthinfo_epp: return "sendauthinfo_epp";
    }
    throw std::runtime_error{"unexpected email type"};
}

EmailType get_email_type(const LibFred::PublicRequestTypeIface& public_request)
{
    const auto public_request_type = public_request.get_public_request_type();
    if (public_request_type == Type::get_iface_of<Type::AuthinfoAutoRif>().get_public_request_type())
    {
        return EmailType::sendauthinfo_epp;
    }
    if ((public_request_type == Type::get_iface_of<Type::AuthinfoAuto>().get_public_request_type()) ||
        (public_request_type == Type::get_iface_of<Type::AuthinfoEmail>().get_public_request_type()) ||
        (public_request_type == Type::get_iface_of<Type::AuthinfoGovernment>().get_public_request_type()) ||
        (public_request_type == Type::get_iface_of<Type::AuthinfoPost>().get_public_request_type()))
    {
        return EmailType::sendauthinfo_pif;
    }
    throw std::runtime_error{"unexpected public request type: " + public_request_type};
}

std::string get_template_name_subject(EmailType _email_type)
{
    switch (_email_type) {
        case EmailType::sendauthinfo_pif: return "send-authinfo-subject.txt";
        case EmailType::sendauthinfo_epp: return "send-authinfo-subject.txt";
    }
    throw std::runtime_error{"unexpected email type"};
}

std::string get_template_name_body(EmailType _email_type)
{
    switch (_email_type) {
        case EmailType::sendauthinfo_pif: return "send-authinfo-pif-body.txt";
        case EmailType::sendauthinfo_epp: return "send-authinfo-epp-body.txt";
    }
    throw std::runtime_error{"unexpected email type"};
}

auto get_authinfo_ttl()
{
    static const auto ttl = []()
    {
        try
        {
            return CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_authinfo_ttl();
        }
        catch (...) { }
        try
        {
            return CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->authinfo_ttl;
        }
        catch (...) { }
        return std::chrono::seconds{14 * 24 * 3600};
    }();
    return ttl;
}

void send_authinfo_email(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        const MessengerArgs& _messenger_args,
        EmailType _email_type)
{
    auto& ctx = _locked_request.get_ctx();
    const auto public_request_id = _locked_request.get_id();
    const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(ctx, _locked_request);
    if (request_info.get_registrar_id().isnull())
    {
        throw std::runtime_error{"no registrar specified"};
    }
    const auto object_id = request_info.get_object_id().get_value(); // oops
    // clang-format off
    const std::string sql_query =
            "SELECT oreg.name AS handle, "
                   "oreg.uuid AS object_uuid, "
                   "eot.name AS object_type "
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
    const auto object_uuid = boost::uuids::string_generator{}(static_cast<std::string>(db_result[0]["object_uuid"]));
    LibHermes::Struct email_template_params;
    email_template_params.emplace(LibHermes::StructKey{"handle"}, LibHermes::StructValue{handle});
    const auto object_type = Util::make_object_type(static_cast<std::string>(db_result[0]["object_type"]));
    const auto public_request_uuid = Util::get_public_request_uuid(ctx, public_request_id);
    const auto registrar_info = LibFred::InfoRegistrarById{request_info.get_registrar_id().get_value()}.exec(ctx).info_registrar_data;
    if (_email_type == EmailType::sendauthinfo_pif)
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
        email_template_params.emplace(LibHermes::StructKey{"reqid"}, LibHermes::StructValue{boost::lexical_cast<std::string>(public_request_id)});
        email_template_params.emplace(LibHermes::StructKey{"reqdate"}, LibHermes::StructValue{static_cast<std::string>(dbres[0][0])});
    }
    else if (_email_type == EmailType::sendauthinfo_epp)
    {
        if (!request_info.get_registrar_id().isnull())
        {
            email_template_params.emplace(LibHermes::StructKey{"registrar"}, LibHermes::StructValue{registrar_info.name.get_value_or(registrar_info.handle)});
            email_template_params.emplace(LibHermes::StructKey{"registrar_url"}, LibHermes::StructValue{registrar_info.url.get_value_or("")});
        }
    }
    std::set<Util::EmailData::Recipient> recipients;
    const auto email_to_answer = request_info.get_email_to_answer();
    if (!email_to_answer.isnull())
    {
        recipients.insert(Util::EmailData::Recipient{email_to_answer.get_value(), boost::none}); // validity checked when public_request was created
    }
    else
    {
        recipients = get_valid_registry_emails_of_registered_object(ctx, object_type, object_id);
        if (recipients.empty())
        {
            throw NoContactEmail();
        }
    }

    static const auto to_libhermes_type = [](ObjectType object_type)
    {
        switch (object_type)
        {
            case ObjectType::contact: return 1;
            case ObjectType::nsset: return 2;
            case ObjectType::domain: return 3;
            case ObjectType::keyset: return 4;
        }
        throw std::runtime_error("unexpected value of ObjectType");
    };
    const auto get_plaintext_password = [&]()
    {
        auto password = LibFred::generate_authinfo_pw().password_;
        LibFred::Object::StoreAuthinfo{
                LibFred::Object::ObjectId{object_id},
                registrar_info.id,
                get_authinfo_ttl()}.exec(ctx, password);
        return password;
    };
    email_template_params.emplace(LibHermes::StructKey{"type"}, LibHermes::StructValue{to_libhermes_type(object_type)});
    email_template_params.emplace(LibHermes::StructKey{"authinfo"}, LibHermes::StructValue{get_plaintext_password()});

    const Util::EmailData email_data{
            recipients,
            get_email_type_name(_email_type),
            get_template_name_subject(_email_type),
            get_template_name_body(_email_type),
            email_template_params,
            object_type,
            object_uuid,
            public_request_uuid,
            {}};

    send_joined_addresses_email(_messenger_args.endpoint, _messenger_args.archive, email_data);
}

} // namespace Fred::Backend::PublicRequest::Process::{anonymous}

void process_public_request_authinfo_resolved(
        unsigned long long _public_request_id,
        const LibFred::PublicRequestTypeIface& _public_request_type,
        const MessengerArgs& _messenger_args)
{
    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);

        try
        {
            send_authinfo_email(locked_request, _messenger_args, get_email_type(_public_request_type));
        }
        catch (const std::exception& e)
        {
            ctx.get_log().info(boost::format("Request %1% sending email failed (%2%)") % _public_request_id % e.what());
        }
        catch (...)
        {
            ctx.get_log().info(boost::format("Request %1% sending email failed") % _public_request_id);
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
                    boost::format("Request %1% update failed (%2%), but email was sent") %
                    _public_request_id %
                    e.what());
        }
        catch (...)
        {
            ctx.get_log().info(
                    boost::format("Request %1% update failed (unknown exception), but email was sent") %
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
