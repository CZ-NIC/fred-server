#include "src/fredlib/opcontext.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/email/email_utils.h"

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace Fred
{

struct NoPublicRequest : std::exception
{
    virtual const char* what() const throw()
    {
        return "no public request found";
    }
};

struct NoContactEmail : std::exception
{
    virtual const char* what() const throw()
    {
        return "no contact email found";
    }
};

unsigned long long send_authinfo(
    const unsigned long long public_request_id,
    const std::string& handle,
    Fred::Object_Type::Enum object_type,
    boost::shared_ptr<Mailer::Manager> manager)
{
    Fred::OperationContextCreator ctx;
    Database::Result request = ctx.get_conn().exec_params(
            "SELECT create_time FROM public_request "
            "WHERE id = $1::bigint ",
            Database::query_param_list(public_request_id));
    if (request.size() < 1)
    {
        throw NoPublicRequest();
    }

    Mailer::Parameters email_template_params;
    email_template_params.insert(Mailer::Parameters::value_type("reqid", boost::lexical_cast<std::string>(public_request_id)));
    email_template_params.insert(Mailer::Parameters::value_type("reqdate", request[0][0]));
    email_template_params.insert(Mailer::Parameters::value_type("type", boost::lexical_cast<std::string>(object_type + 1))); // to be 1-4 instead of 0-3
    email_template_params.insert(Mailer::Parameters::value_type("handle", handle));

    Database::Result authinfo_email;
    if (object_type == Fred::Object_Type::contact)
    {
        authinfo_email = ctx.get_conn().exec_params(
                "SELECT authinfopw, email FROM object "
                    "JOIN object_registry ON object.id = object_registry.id "
                    "JOIN contact ON object.id = contact.id "
                "WHERE object_registry.name = $1::text AND email IS NOT NULL ",
                Database::query_param_list(handle));
    }
    else if (object_type == Fred::Object_Type::nsset)
    {
        authinfo_email = ctx.get_conn().exec_params(
                "SELECT authinfopw, email FROM object "
                    "JOIN object_registry ON object.id = object_registry.id "
                    "JOIN nsset ON object.id = nsset.id "
                    "JOIN nsset_contact_map ON nsset.id = nsset_contact_map.nssetid "
                    "JOIN contact ON nsset_contact_map.contactid = contact.id "
                "WHERE object_registry.name = $1::text AND email IS NOT NULL ",
                Database::query_param_list(handle));
    }
    else if (object_type == Fred::Object_Type::domain)
    {
        authinfo_email = ctx.get_conn().exec_params(
                "SELECT authinfopw, email FROM object "
                    "JOIN object_registry ON object.id = object_registry.id "
                    "JOIN domain ON object.id = domain.id "
                    "JOIN contact ON domain.registrant = contact.id "
                "WHERE object_registry.name = $1::text AND email IS NOT NULL "
                "UNION "
                "SELECT authinfopw, email FROM object "
                    "JOIN object_registry ON object.id = object_registry.id "
                    "JOIN domain ON object.id = domain.id "
                    "JOIN domain_contact_map dcm ON domain.id = dcm.domainid "
                    "JOIN contact ON dcm.contactid = contact.id "
                "WHERE object_registry.name = $1::text AND dcm.role=1 AND email IS NOT NULL ",
                Database::query_param_list(handle));
    }
    else if (object_type == Fred::Object_Type::keyset)
    {
        authinfo_email = ctx.get_conn().exec_params(
                "SELECT authinfopw, email FROM object "
                    "JOIN object_registry ON object.id = object_registry.id "
                    "JOIN keyset ON object.id = keyset.id "
                    "JOIN keyset_contact_map ON keyset.id = keyset_contact_map.keysetid "
                    "JOIN contact ON keyset_contact_map.contactid = contact.id "
                "WHERE object_registry.name = $1::text AND email IS NOT NULL ",
                Database::query_param_list(handle));
    }
    if (authinfo_email.size() < 1)
    {
        throw NoContactEmail();
    }

    std::set<std::string> emails;
    for (Database::Result::Iterator it = authinfo_email.begin(); it != authinfo_email.end(); ++it)
    {
        emails.insert((*it)[1]);
    }
    email_template_params.insert(Mailer::Parameters::value_type("authinfo", authinfo_email[0][0]));
    const EmailData data(emails, "sendauthinfo_pif", email_template_params);
    return send_joined_addresses_email(manager, data);
}

} // namespace Fred
