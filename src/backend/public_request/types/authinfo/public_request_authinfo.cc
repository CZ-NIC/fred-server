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

#include "src/backend/public_request/types/authinfo/public_request_authinfo.hh"
#include "src/backend/public_request/types/impl/public_request_impl.hh"
#include "src/backend/public_request/send_email.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

namespace {

struct AuthinfoImplementation
{
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        LibFred::PublicRequestTypeIface::PublicRequestTypes res;
        res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new T));
        return res;
    }
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum,
            LibFred::PublicRequest::Status::Enum) const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    };
    template <typename T>
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status) const
    {
        return LibFred::PublicRequest::OnStatusAction::processed;
    };
};

typedef ImplementedBy<AuthinfoImplementation> AuthinfoPublicRequest;

extern const char authinfo_auto_pif[] = "authinfo_auto_pif";
typedef AuthinfoPublicRequest::Named<authinfo_auto_pif> AuthinfoAuto;

extern const char authinfo_email_pif[] = "authinfo_email_pif";
typedef AuthinfoPublicRequest::Named<authinfo_email_pif> AuthinfoEmail;

extern const char authinfo_post_pif[] = "authinfo_post_pif";
typedef AuthinfoPublicRequest::Named<authinfo_post_pif> AuthinfoPost;

} // namespace Fred::Backend::PublicRequest::Type::{anonymous}
} // namespace Fred::Backend::PublicRequest::Type

unsigned long long send_authinfo(
        unsigned long long public_request_id,
        const std::string& handle,
        PublicRequestImpl::ObjectType::Enum object_type,
        std::shared_ptr<LibFred::Mailer::Manager> manager)
{
    LibFred::OperationContextCreator ctx;
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

    std::string sql;
    std::string object_type_handle;
    switch (object_type)
    {
        case PublicRequestImpl::ObjectType::contact:
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
        case PublicRequestImpl::ObjectType::nsset:
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
        case PublicRequestImpl::ObjectType::domain:
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
        case PublicRequestImpl::ObjectType::keyset:
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
    const Database::Result dbres = ctx.get_conn().exec_params(
            sql,
            Database::query_param_list(handle)(object_type_handle));
    if (dbres.size() < 1)
    {
        throw PublicRequestImpl::NoContactEmail();
    }

    email_template_params.insert(LibFred::Mailer::Parameters::value_type("authinfo", static_cast<std::string>(dbres[0][0])));
    std::set<std::string> recipients;
    for (unsigned idx = 0; idx < dbres.size(); ++idx)
    {
        recipients.insert(static_cast<std::string>(dbres[idx][1]));
    }
    const EmailData data(recipients, "sendauthinfo_pif", email_template_params, std::vector<unsigned long long>());
    return send_joined_addresses_email(manager, data);
}

void check_authinfo_request_permission(const LibFred::ObjectStatesInfo& states)
{
    if (states.presents(LibFred::Object_State::server_transfer_prohibited))
    {
        throw PublicRequestImpl::ObjectTransferProhibited();
    }
}

const LibFred::PublicRequestTypeIface& get_auth_info_auto_iface()
{
    static const Type::AuthinfoAuto singleton;
    return singleton;
}

const LibFred::PublicRequestTypeIface& get_auth_info_email_iface()
{
    static const Type::AuthinfoEmail singleton;
    return singleton;
}

const LibFred::PublicRequestTypeIface& get_auth_info_post_iface()
{
    static const Type::AuthinfoPost singleton;
    return singleton;
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
