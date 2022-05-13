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
#include "src/backend/public_request/util/send_joined_address_email.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object/object_type.hh"
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"
#include "libfred/public_request/info_public_request.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/public_request_on_status_action.hh"
#include "libfred/public_request/update_public_request.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrar/info_registrar.hh"
#include "src/util/corba_wrapper_decl.hh"

#include "libhermes/struct.hh"

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

enum class EmailType
{
    sendauthinfo_pif,
    sendauthinfo_epp
};

std::string get_email_type_name(EmailType email_type)
{
    switch (email_type)
    {
        case EmailType::sendauthinfo_pif:
            return "sendauthinfo_pif";
        case EmailType::sendauthinfo_epp:
            return "sendauthinfo_epp";
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
        case EmailType::sendauthinfo_epp: return "send_authinfo-subject.txt";
    }
    throw std::runtime_error{"unexpected email type"};
}

std::string get_template_name_body(EmailType _email_type)
{
    switch (_email_type) {
        case EmailType::sendauthinfo_pif: return "send-authinfo-pif-body.txt";
        case EmailType::sendauthinfo_epp: return "send_authinfo-epp-body.txt";
    }
    throw std::runtime_error{"unexpected email type"};
}

void send_authinfo_email(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        const MessengerArgs& _messenger_args,
        EmailType _email_type)
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
    LibHermes::Struct email_template_params;
    email_template_params.emplace(LibHermes::StructKey{"handle"}, LibHermes::StructValue{handle});
    const LibFred::Object_Type::Enum object_type =
            Conversion::Enums::from_db_handle<LibFred::Object_Type>(static_cast<std::string>(db_result[0]["object_type"]));
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
            const auto registrar_info = LibFred::InfoRegistrarById{request_info.get_registrar_id().get_value()}.exec(ctx).info_registrar_data;
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
        recipients = get_valid_registry_emails_of_registered_object(ctx, convert_libfred_object_type_to_public_request_objecttype(object_type), object_id);
        if (recipients.empty())
        {
            throw NoContactEmail();
        }
    }

    int type;
    std::string authinfo;
    switch (object_type)
    {
        case LibFred::Object_Type::contact:
            type = 1;
            authinfo = LibFred::InfoContactById(object_id).exec(ctx).info_contact_data.authinfopw;
            break;
        case LibFred::Object_Type::nsset:
            type = 2;
            authinfo = LibFred::InfoNssetById(object_id).exec(ctx).info_nsset_data.authinfopw;
            break;
        case LibFred::Object_Type::domain:
            type = 3;
            authinfo = LibFred::InfoDomainById(object_id).exec(ctx).info_domain_data.authinfopw;
            break;
        case LibFred::Object_Type::keyset:
            type = 4;
            authinfo = LibFred::InfoKeysetById(object_id).exec(ctx).info_keyset_data.authinfopw;
            break;
    }
    email_template_params.emplace(LibHermes::StructKey{"type"}, LibHermes::StructValue{type});
    email_template_params.emplace(LibHermes::StructKey{"authinfo"}, LibHermes::StructValue{authinfo});

    const Util::EmailData email_data{
            recipients,
            get_email_type_name(_email_type),
            get_template_name_subject(_email_type),
            get_template_name_body(_email_type),
            email_template_params,
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
        send_authinfo_email(
                locked_request,
                _messenger_args,
                get_email_type(_public_request_type));
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
