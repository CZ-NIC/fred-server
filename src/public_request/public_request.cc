#include "src/public_request/public_request.h"
#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/public_request/update_public_request.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object/get_present_object_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/public_request/public_request_type_iface.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/public_request/info_public_request.h"
#include "util/cfg/handle_registry_args.h"
#include "util/cfg/config_handler_decl.h"
#include "util/corba_wrapper_decl.h"
#include "util/types/stringify.h"
#include "util/db/query_param.h"

#include <boost/format.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <set>
#include <map>
#include <stdexcept>
#include <sstream>

namespace Registry {
namespace PublicRequestType {
namespace {

class Common : public Fred::PublicRequestTypeIface
{
public:
    const Fred::PublicRequestTypeIface& iface() const { return *static_cast<const Fred::PublicRequestTypeIface*>(this); }
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            Fred::PublicRequest::Status::Enum _old_status,
            Fred::PublicRequest::Status::Enum _new_status) const
    {
        return PublicRequestTypes();
    }
};

class AuthinfoAuto : public Common
{
private:
    std::string get_public_request_type() const { return "authinfo_auto_pif"; }
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoAuto));
        return res;
    }
};

class AuthinfoEmail : public Common
{
private:
    std::string get_public_request_type() const { return "authinfo_email_pif"; }
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoEmail));
        return res;
    }
};

class AuthinfoPost : public Common
{
private:
    std::string get_public_request_type() const { return "authinfo_post_pif"; }
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoPost));
        return res;
    }
};

class BlockUnblockCommon : public Common
{
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
};

struct Block
{
    struct Changes
    {
        class ByEmail : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "block_changes_email_pif"; }
        };

        class ByPost : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "block_changes_post_pif"; }
        };
    };

    struct Transfer
    {
        class ByEmail : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "block_transfer_email_pif"; }
        };

        class ByPost : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "block_transfer_post_pif"; }
        };
    };
};

struct Unblock
{
    struct Changes
    {
        class ByEmail : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "unblock_changes_email_pif"; }
        };

        class ByPost : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "unblock_changes_post_pif"; }
        };
    };

    struct Transfer
    {
        class ByEmail : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "unblock_transfer_email_pif"; }
        };

        class ByPost : public BlockUnblockCommon
        {
        private:
            std::string get_public_request_type() const { return "unblock_transfer_post_pif"; }
        };
    };
};


Fred::PublicRequestTypeIface::PublicRequestTypes BlockUnblockCommon::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes res;
    res.insert(IfacePtr(new Block::Changes::ByEmail));
    res.insert(IfacePtr(new Block::Changes::ByPost));
    res.insert(IfacePtr(new Block::Transfer::ByEmail));
    res.insert(IfacePtr(new Block::Transfer::ByPost));
    res.insert(IfacePtr(new Unblock::Changes::ByEmail));
    res.insert(IfacePtr(new Unblock::Changes::ByPost));
    res.insert(IfacePtr(new Unblock::Transfer::ByEmail));
    res.insert(IfacePtr(new Unblock::Transfer::ByPost));
    return res;
}

}//namespace Registry::PublicRequestType::{anonymous}
}//namespace Registry::PublicRequestType

namespace {

struct NoPublicRequest : std::exception
{
    virtual const char* what() const throw()
    {
        return "no public request found";
    }
};

struct EmailData
{
    EmailData(
            const std::set<std::string>& _recipient_email_addresses,
            const std::string& _template_name,
            const std::map<std::string, std::string>& _template_parameters)
        : recipient_email_addresses(_recipient_email_addresses),
          template_name(_template_name),
          template_parameters(_template_parameters)
    {}
    const std::set<std::string> recipient_email_addresses;
    const std::string template_name;
    const std::map<std::string, std::string> template_parameters;
};

struct FailedToSendMailToRecipient : std::exception
{
    FailedToSendMailToRecipient(
            const std::string& _failed_recipient,
            const std::set<std::string>& _skipped_recipients)
        : failed_recipient(_failed_recipient),
          skipped_recipients(_skipped_recipients)
    {}
    ~FailedToSendMailToRecipient() throw() {}
    const char* what()const throw() { return "failed to send mail to recipient"; }
    const std::string failed_recipient;
    const std::set<std::string> skipped_recipients;
};

unsigned long long send_joined_addresses_email(
    boost::shared_ptr<Fred::Mailer::Manager> mailer,
    const EmailData& data)
{
    std::set<std::string> trimmed_recipient_email_addresses;
    BOOST_FOREACH(const std::string& email, data.recipient_email_addresses)
    {
        trimmed_recipient_email_addresses.insert(boost::trim_copy(email));
    }

    std::ostringstream recipients;
    for (std::set<std::string>::const_iterator address_ptr = trimmed_recipient_email_addresses.begin();
         address_ptr != trimmed_recipient_email_addresses.end(); ++address_ptr)
    {
        recipients << *address_ptr << ' ';
    }
    try
    {
        return mailer->sendEmail(
                "",
                recipients.str(),
                "",
                data.template_name,
                data.template_parameters,
                Fred::Mailer::Handles(),
                Fred::Mailer::Attachments());
    }
    catch (const Fred::Mailer::NOT_SEND&)
    {
        throw FailedToSendMailToRecipient(recipients.str(), std::set<std::string>());
    }
}

unsigned long long send_authinfo(
    unsigned long long public_request_id,
    const std::string& handle,
    Fred::Object_Type::Enum object_type,
    boost::shared_ptr<Fred::Mailer::Manager> manager)
{
    Fred::OperationContextCreator ctx;
    Fred::Mailer::Parameters email_template_params;
    {
        const Database::Result dbres = ctx.get_conn().exec_params(
                "SELECT create_time FROM public_request "
                "WHERE id=$1::bigint",
                Database::query_param_list(public_request_id));
        if (dbres.size() < 1)
        {
            throw NoPublicRequest();
        }
        if (1 < dbres.size())
        {
            throw std::runtime_error("too many public requests for given id");
        }
        email_template_params.insert(Fred::Mailer::Parameters::value_type("reqid", boost::lexical_cast<std::string>(public_request_id)));
        email_template_params.insert(Fred::Mailer::Parameters::value_type("reqdate", static_cast<std::string>(dbres[0][0])));
        email_template_params.insert(Fred::Mailer::Parameters::value_type("handle", handle));
    }

    std::string sql;
    const std::string object_type_handle = Conversion::Enums::to_db_handle(object_type);
    switch (object_type)
    {
        case Fred::Object_Type::contact:
            sql = "SELECT o.authinfopw,c.email "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN contact c ON c.id=o.id "
                  "WHERE obr.name=$1::TEXT AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "c.email IS NOT NULL";
            email_template_params.insert(Fred::Mailer::Parameters::value_type("type", "1"));
            break;
        case Fred::Object_Type::nsset:
            sql = "SELECT o.authinfopw,c.email "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN nsset n ON n.id=o.id "
                  "JOIN nsset_contact_map ncm ON ncm.nssetid=n.id "
                  "JOIN contact c ON c.id=ncm.contactid "
                  "WHERE obr.name=$1::TEXT AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "c.email IS NOT NULL";
            email_template_params.insert(Fred::Mailer::Parameters::value_type("type", "2"));
            break;
        case Fred::Object_Type::domain:
            sql = "SELECT o.authinfopw,c.email "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN domain d ON d.id=o.id "
                  "JOIN contact c ON c.id=d.registrant "
                  "WHERE obr.name=$1::TEXT AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "c.email IS NOT NULL "
              "UNION "
                  "SELECT o.authinfopw,c.email "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN domain d ON d.id=o.id "
                  "JOIN domain_contact_map dcm ON dcm.domainid=d.id "
                  "JOIN contact c ON c.id=dcm.contactid AND c.id!=d.registrant "
                  "WHERE obr.name=$1::TEXT AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "c.email IS NOT NULL AND "
                        "dcm.role=1";
            email_template_params.insert(Fred::Mailer::Parameters::value_type("type", "3"));
            break;
        case Fred::Object_Type::keyset:
            sql = "SELECT o.authinfopw,c.email "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN keyset k ON k.id=o.id "
                  "JOIN keyset_contact_map kcm ON kcm.keysetid=k.id "
                  "JOIN contact c ON c.id=kcm.contactid "
                  "WHERE obr.name=$1::TEXT AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "c.email IS NOT NULL";
            email_template_params.insert(Fred::Mailer::Parameters::value_type("type", "4"));
            break;
    }
    const Database::Result dbres = ctx.get_conn().exec_params(
            sql,
            Database::query_param_list(handle)(object_type_handle));
    if (dbres.size() < 1)
    {
        throw PublicRequestImpl::NoContactEmail();
    }

    email_template_params.insert(Fred::Mailer::Parameters::value_type("authinfo", static_cast<std::string>(dbres[0][0])));
    std::set<std::string> recipients;
    for (unsigned idx = 0; idx < dbres.size(); ++idx)
    {
        recipients.insert(static_cast<std::string>(dbres[idx][1]));
    }
    const EmailData data(recipients, "sendauthinfo_pif", email_template_params);
    return send_joined_addresses_email(manager, data);
}

Fred::Object_Type::Enum to_fred_object_type(PublicRequestImpl::ObjectType::Enum value)
{
    switch (value)
    {
        case PublicRequestImpl::ObjectType::contact: return Fred::Object_Type::contact;
        case PublicRequestImpl::ObjectType::nsset: return Fred::Object_Type::nsset;
        case PublicRequestImpl::ObjectType::domain: return Fred::Object_Type::domain;
        case PublicRequestImpl::ObjectType::keyset: return Fred::Object_Type::keyset;
    }
    throw std::logic_error("unexpected PublicRequestImpl::ObjectType::Enum value");
}

}//namespace Registry::{anonymous}

unsigned long long PublicRequestImpl::create_authinfo_request_registry_email(
    ObjectType::Enum object_type,
    const std::string& object_handle,
    const Optional<unsigned long long>& log_request_id,
    boost::shared_ptr<Fred::Mailer::Manager> manager) // potentially put as member
{
    try
    {
        unsigned long long object_id;
        unsigned long long public_request_id;
        const PublicRequestType::AuthinfoAuto public_request_type;
        {
            Fred::OperationContextCreator ctx;
            object_id = Fred::get_present_object_id(ctx, to_fred_object_type(object_type), object_handle);
            Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
            public_request_id = Fred::CreatePublicRequest(
                    "create_authinfo_request_registry_email call",
                    Optional<std::string>(),
                    Optional<unsigned long long>()).exec(locked_object, public_request_type, log_request_id);
            ctx.commit_transaction();
        }
        try
        {
            const unsigned long long email_id =
                    send_authinfo(public_request_id, object_handle, to_fred_object_type(object_type), manager);
            try
            {
                Fred::OperationContextCreator ctx;
                Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
                Fred::UpdatePublicRequest(
                        Fred::PublicRequest::Status::answered,
                        Optional< Nullable< std::string > >(),
                        Optional< Nullable< std::string > >(),
                        email_id,
                        Optional< Nullable< Fred::RegistrarId > >())
                    .exec(locked_object, public_request_type, log_request_id);
                ctx.commit_transaction();
            }
            catch (...)
            {
                LOGGER(PACKAGE).info(boost::format("Request %1% update failed, but email %2% sent") % public_request_id % email_id);
                //no throw as main part is completed
            }
            return public_request_id;
        }
        catch (...)
        {
            Fred::OperationContextCreator ctx;
            Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
            Fred::UpdatePublicRequest(
                    Fred::PublicRequest::Status::invalidated,
                    Optional< Nullable< std::string > >(),
                    Optional< Nullable< std::string > >(),
                    Optional< Nullable< unsigned long long > >(),
                    Optional< Nullable< Fred::RegistrarId > >())
                .exec(locked_object, public_request_type, log_request_id);
            ctx.commit_transaction();
            throw;
        }
    }
    catch (const NoPublicRequest& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

unsigned long long PublicRequestImpl::create_authinfo_request_non_registry_email(
    ObjectType::Enum object_type,
    const std::string& object_handle,
    const Optional<unsigned long long>& log_request_id,
    ConfirmationMethod::Enum confirmation_method,
    const std::string& specified_email)
{
    try
    {
        Fred::OperationContextCreator ctx;
        Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(
                ctx,
                Fred::get_present_object_id(ctx, to_fred_object_type(object_type), object_handle));
        const Fred::CreatePublicRequest create_public_request_op(
                "create_authinfo_request_non_registry_email call",
                specified_email,
                Optional<unsigned long long>());
        switch (confirmation_method)
        {
            case ConfirmationMethod::email_with_qualified_certificate:
            {
                const unsigned long long request_id =
                        create_public_request_op.exec(locked_object, PublicRequestType::AuthinfoEmail(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case ConfirmationMethod::letter_with_authenticated_signature:
            {
                const unsigned long long request_id =
                        create_public_request_op.exec(locked_object, PublicRequestType::AuthinfoPost(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
        }
        throw std::runtime_error("unexpected confirmation method");
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const Fred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).error(boost::diagnostic_information(e));
            throw InvalidContactEmail();
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

namespace {

template <typename request>
const Fred::PublicRequestTypeIface& get_public_request_type(PublicRequestImpl::ConfirmationMethod::Enum confirmation_method)
{
    switch (confirmation_method)
    {
        case PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate:
        {
            static const typename request::ByEmail public_request_type;
            return public_request_type.iface();
        }
        case PublicRequestImpl::ConfirmationMethod::letter_with_authenticated_signature:
        {
            static const typename request::ByPost public_request_type;
            return public_request_type.iface();
        }
    }
    throw std::runtime_error("unexpected confirmation method");
}

}//namespace Registry::{anonymous}

unsigned long long PublicRequestImpl::create_block_unblock_request(
    ObjectType::Enum object_type,
    const std::string& object_handle,
    const Optional<unsigned long long>& log_request_id,
    ConfirmationMethod::Enum confirmation_method,
    LockRequestType::Enum lock_request_type)
{
    try
    {
        Fred::OperationContextCreator ctx;
        const unsigned long long object_id = Fred::get_present_object_id(ctx, to_fred_object_type(object_type), object_handle);
        Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        const Fred::ObjectStatesInfo states(Fred::GetObjectStates(object_id).exec(ctx));
        switch (lock_request_type)
        {
            case LockRequestType::block_transfer:
            {
                if (states.presents(Fred::Object_State::server_transfer_prohibited))
                {
                    throw ObjectAlreadyBlocked();
                }
                const unsigned long long request_id =
                        Fred::CreatePublicRequest().exec(
                                locked_object,
                                get_public_request_type<PublicRequestType::Block::Transfer>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::block_transfer_and_update:
            {
                if (states.presents(Fred::Object_State::server_transfer_prohibited) &&
                    states.presents(Fred::Object_State::server_update_prohibited))
                {
                    throw ObjectAlreadyBlocked();
                }
                const unsigned long long request_id =
                        Fred::CreatePublicRequest().exec(
                                locked_object,
                                get_public_request_type<PublicRequestType::Block::Changes>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::unblock_transfer:
            {
                if (states.presents(Fred::Object_State::server_update_prohibited))
                {
                    throw HasDifferentBlock();
                }
                if (states.absents(Fred::Object_State::server_transfer_prohibited))
                {
                    throw ObjectNotBlocked();
                }
                const unsigned long long request_id =
                        Fred::CreatePublicRequest().exec(
                                locked_object,
                                get_public_request_type<PublicRequestType::Unblock::Transfer>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::unblock_transfer_and_update:
            {
                if (states.absents(Fred::Object_State::server_update_prohibited))
                {
                    if (states.presents(Fred::Object_State::server_transfer_prohibited))
                    {
                        throw HasDifferentBlock();
                    }
                    throw ObjectNotBlocked();
                }
                const unsigned long long request_id =
                        Fred::CreatePublicRequest().exec(
                                locked_object,
                                get_public_request_type<PublicRequestType::Unblock::Changes>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
        }
        throw std::runtime_error("unexpected lock request type");
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const Fred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).error(boost::diagnostic_information(e));
            throw InvalidContactEmail();
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

namespace {

short public_request_type_to_post_type(const std::string& public_request_type)
{
    if (public_request_type == PublicRequestType::AuthinfoPost().iface().get_public_request_type())
    {
        return 1;
    }
    if (public_request_type == PublicRequestType::Block::Transfer::ByPost().iface().get_public_request_type())
    {
        return 2;
    }
    if (public_request_type == PublicRequestType::Unblock::Transfer::ByPost().iface().get_public_request_type())
    {
        return 3;
    }
    if (public_request_type == PublicRequestType::Block::Changes::ByPost().iface().get_public_request_type())
    {
        return 4;
    }
    if (public_request_type == PublicRequestType::Unblock::Changes::ByPost().iface().get_public_request_type())
    {
        return 5;
    }
    throw PublicRequestImpl::InvalidPublicRequestType();
}

std::string language_to_lang_code(PublicRequestImpl::Language::Enum lang)
{
    switch (lang)
    {
        case PublicRequestImpl::Language::cs: return "cs";
        case PublicRequestImpl::Language::en: return "en";
    }
    throw std::invalid_argument("language code not found");
}

}//namespace Registry::{anonymous}

PublicRequestImpl::Buffer PublicRequestImpl::create_public_request_pdf(
    unsigned long long public_request_id,
    Language::Enum lang,
    boost::shared_ptr<Fred::Document::Manager> manager)
{
    const std::string lang_code = language_to_lang_code(lang);

    Fred::OperationContextCreator ctx;
    std::string create_time, email_to_answer;
    unsigned long long post_type;
    try
    {
        Fred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
        Fred::PublicRequestInfo request_info = Fred::InfoPublicRequest().exec(ctx, locked_request);
        post_type = public_request_type_to_post_type(request_info.get_type());
        create_time = stringify(request_info.get_create_time().date());
        email_to_answer = request_info.get_email_to_answer().get_value_or_default();
    }
    catch (const Fred::PublicRequestLockGuardById::Exception&)
    {
        throw ObjectNotFound();
    }

    const Database::Result dbres = ctx.get_conn().exec_params(
            "SELECT oreg.type,oreg.name "
            "FROM public_request pr "
            "JOIN public_request_objects_map prom ON prom.request_id=pr.id "
            "JOIN enum_public_request_type eprt ON eprt.id=pr.request_type "
            "JOIN object_registry oreg ON oreg.id=prom.object_id "
            "WHERE pr.id=$1::BIGINT",
            Database::query_param_list(public_request_id));
    if (dbres.size() != 1)
    {
        if (dbres.size() == 0)
        {
            throw ObjectNotFound();
        }
        throw std::runtime_error("too many objects associated with this public request");
    }
    const unsigned type_id = static_cast<unsigned>(dbres[0][0]);
    const std::string handle = static_cast<std::string>(dbres[0][1]);
    std::ostringstream pdf_content;
    const std::auto_ptr<Fred::Document::Generator> docgen_ptr(
            manager.get()->createOutputGenerator(
                Fred::Document::GT_PUBLIC_REQUEST_PDF,
                pdf_content,
                lang_code));
    docgen_ptr->getInput() << "<?xml version='1.0' encoding='utf-8'?>"
        << "<enum_whois>"
        << "<public_request>"
            << "<type>" << post_type << "</type>"
            << "<handle type='" << type_id << "'>"
            << handle
            << "</handle>"
            << "<date>" << create_time << "</date>"
            << "<id>" << public_request_id << "</id>"
            << "<replymail>" << email_to_answer << "</replymail>"
        << "</public_request>"
        << "</enum_whois>";
    docgen_ptr->closeInput();

    return Buffer(pdf_content.str());
}

boost::shared_ptr<Fred::Mailer::Manager> PublicRequestImpl::get_default_mailer_manager()
{
    return boost::shared_ptr<Fred::Mailer::Manager>(
                   new MailerManager(CorbaContainer::get_instance()->getNS()));
}

boost::shared_ptr<Fred::Document::Manager> PublicRequestImpl::get_default_document_manager()
{
    const HandleRegistryArgs* const args = CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
    return boost::shared_ptr<Fred::Document::Manager>(
            Fred::Document::Manager::create(
                    args->docgen_path,
                    args->docgen_template_path,
                    args->fileclient_path,
                    CorbaContainer::get_instance()->getNS()->getHostName()));
}

PublicRequestImpl::Buffer::Buffer(const std::string& s)
    : value(s)
{ }

}//namespace Registry
