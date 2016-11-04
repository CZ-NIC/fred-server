#include "src/public_request/public_request.h"
#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/public_request/update_public_request.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object/get_present_object_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/send_authinfo.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/public_request/public_request_type_iface.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/public_request/info_public_request.h"
#include "util/types/stringify.h"
#include "util/db/query_param.h"

#include <boost/format.hpp>

#include <stdexcept>
#include <sstream>
#include <string>

namespace Registry
{
namespace PublicRequestImpl
{

class EnumPublicRequestType : public Fred::PublicRequestTypeIface
{
public:
    const Fred::PublicRequestTypeIface& iface() const { return *this; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        Fred::PublicRequest::Status::Enum _old_status,
        Fred::PublicRequest::Status::Enum _new_status) const
    {
        return PublicRequestTypes();
    }
};

class AuthinfoAuto : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "authinfo_auto_pif"; }

    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoAuto));
        return res;
    }
};

class AuthinfoEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "authinfo_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoEmail));
        return res;
    }
};

class AuthinfoPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "authinfo_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoPost));
        return res;
    }
};

class BlockChangesEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_changes_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockChangesEmail));
        return res;
    }
};

class BlockChangesPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_changes_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockChangesPost));
        return res;
    }
};

class BlockTransferEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_transfer_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockTransferEmail));
        return res;
    }
};

class BlockTransferPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_transfer_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockTransferPost));
        return res;
    }
};

class UnblockChangesEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_changes_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockChangesEmail));
        return res;
    }
};

class UnblockChangesPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_changes_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockChangesPost));
        return res;
    }
};

class UnblockTransferEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_transfer_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockTransferEmail));
        return res;
    }
};

class UnblockTransferPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_transfer_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockTransferPost));
        return res;
    }
};

unsigned long long PublicRequest::create_authinfo_request_registry_email(
    Fred::Object_Type::Enum object_type,
    const std::string& object_handle,
    const std::string& reason,
    const Optional<unsigned long long>& log_request_id,
    boost::shared_ptr<Fred::Mailer::Manager> manager) // potentially put as member
{
    try
    {
        unsigned long long present_obj_id;
        unsigned long long public_request_id;
        {
            Fred::OperationContextCreator ctx;
            present_obj_id = Fred::get_present_object_id(ctx, object_type, object_handle);
            Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, present_obj_id);
            public_request_id = Fred::CreatePublicRequest(
                    reason,
                    Optional<std::string>(),
                    Optional<unsigned long long>())
                .exec(locked_object, AuthinfoAuto(), log_request_id);
            ctx.commit_transaction();
        }
        unsigned long long email_id;
        try
        {
            email_id = Fred::send_authinfo(public_request_id, object_handle, object_type, manager);
        }
        catch (...)
        {
            Fred::OperationContextCreator ctx;
            Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, present_obj_id);
            Fred::UpdatePublicRequest(
                    Fred::PublicRequest::Status::invalidated,
                    Optional< Nullable< std::string > >(),
                    Optional< Nullable< std::string > >(),
                    email_id,
                    Optional< Nullable< Fred::RegistrarId > >())
                .exec(locked_object, AuthinfoAuto(), log_request_id);
            ctx.commit_transaction();
            throw;
        }
        try
        {
            Fred::OperationContextCreator ctx;
            Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, present_obj_id);
            Fred::UpdatePublicRequest(
                    Fred::PublicRequest::Status::answered,
                    Optional< Nullable< std::string > >(),
                    Optional< Nullable< std::string > >(),
                    email_id,
                    Optional< Nullable< Fred::RegistrarId  > >())
                .exec(locked_object, AuthinfoAuto(), log_request_id);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER(PACKAGE).info(boost::format("Request %1% update failed, but email %2% sent") % public_request_id % email_id);
            //no throw as main part is completed
        }
        return public_request_id;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Unknown error");
        throw;
    }
}

unsigned long long PublicRequest::create_authinfo_request_non_registry_email(
    Fred::Object_Type::Enum object_type,
    const std::string& object_handle,
    const std::string& reason,
    const Optional<unsigned long long>& log_request_id,
    ConfirmationMethod confirmation_method,
    const std::string& specified_email)
{
    try
    {
        Fred::OperationContextCreator ctx;
        Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(
            ctx,
            Fred::get_present_object_id(ctx, object_type, object_handle));
        unsigned long long request_id;
        Fred::CreatePublicRequest cpr(reason, specified_email, Optional<unsigned long long>());
        if (confirmation_method == EMAIL_WITH_QUALIFIED_CERTIFICATE)
        {
            request_id = cpr.exec(locked_object, AuthinfoEmail(), log_request_id);
        }
        else if (confirmation_method == LETTER_WITH_AUTHENTICATED_SIGNATURE)
        {
            request_id = cpr.exec(locked_object, AuthinfoPost(), log_request_id);
        }
        ctx.commit_transaction();
        return request_id;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Unknown error");
        throw;
    }
}

template<typename EmailInterface, typename PostInterface>
unsigned long long get_block_id(
    bool states_are_ok,
    ConfirmationMethod confirmation_method,
    const Fred::CreatePublicRequest& request,
    const Fred::PublicRequestsOfObjectLockGuardByObjectId& locked_object,
    const Optional<unsigned long long>& log_request_id)
{
    unsigned long long request_id;
    if (! states_are_ok)
    {
        throw ObjectAlreadyBlocked();
    }
    if (confirmation_method == EMAIL_WITH_QUALIFIED_CERTIFICATE)
    {
        request_id = request.exec(locked_object, EmailInterface(), log_request_id);
    }
    else if (confirmation_method == LETTER_WITH_AUTHENTICATED_SIGNATURE)
    {
        request_id = request.exec(locked_object, PostInterface(), log_request_id);
    }
    else
    {
        throw std::invalid_argument("Registry::PublicRequest::ConfirmationMethod doesn't contain that value");
    }
    return request_id;
}

template<typename EmailInterface, typename PostInterface>
unsigned long long get_unblock_id(
    ConfirmationMethod confirmation_method,
    const Fred::CreatePublicRequest& request,
    const Fred::PublicRequestsOfObjectLockGuardByObjectId& locked_object,
    const Optional<unsigned long long>& log_request_id)
{
    unsigned long long request_id;
    if (confirmation_method == EMAIL_WITH_QUALIFIED_CERTIFICATE)
    {
        request_id = request.exec(locked_object, EmailInterface(), log_request_id);
    }
    else if (confirmation_method == LETTER_WITH_AUTHENTICATED_SIGNATURE)
    {
        request_id = request.exec(locked_object, PostInterface(), log_request_id);
    }
    else
    {
        throw std::invalid_argument("Registry::PublicRequest::ConfirmationMethod doesn't contain that value");
    }
    return request_id;
}

unsigned long long PublicRequest::create_block_unblock_request(
    Fred::Object_Type::Enum object_type,
    const std::string& object_handle,
    const Optional<unsigned long long>& log_request_id,
    ConfirmationMethod confirmation_method,
    LockRequestType lock_request_type)
{
    try
    {
        Fred::OperationContextCreator ctx;
        unsigned long long object_id = Fred::get_present_object_id(ctx, object_type, object_handle);
        Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        const Fred::ObjectStatesInfo states(Fred::GetObjectStates(object_id).exec(ctx));
        unsigned long long request_id;
        Fred::CreatePublicRequest request = Fred::CreatePublicRequest(
                Optional<std::string>(),
                Optional<std::string>(),
                Optional<unsigned long long>());
        if (lock_request_type == BLOCK_TRANSFER)
        {
            request_id = get_block_id<BlockTransferEmail, BlockTransferPost>(
                    states.absents(Fred::Object_State::server_transfer_prohibited),
                    confirmation_method,
                    request,
                    locked_object,
                    log_request_id);
        }
        else if (lock_request_type == BLOCK_TRANSFER_AND_UPDATE)
        {
            request_id = get_block_id<BlockChangesEmail, BlockChangesPost>(
                    (states.absents(Fred::Object_State::server_transfer_prohibited) ||
                     states.absents(Fred::Object_State::server_update_prohibited)),
                    confirmation_method,
                    request,
                    locked_object,
                    log_request_id);
        }
        else if (lock_request_type == UNBLOCK_TRANSFER)
        {
            if (states.presents(Fred::Object_State::server_update_prohibited))
            {
                throw HasDifferentBlock();
            }
            else if (states.absents(Fred::Object_State::server_transfer_prohibited))
            {
                throw ObjectNotBlocked();
            }
            request_id = get_unblock_id<UnblockTransferEmail, UnblockTransferPost>(
                    confirmation_method,
                    request,
                    locked_object,
                    log_request_id);
        }
        else if (lock_request_type == UNBLOCK_TRANSFER_AND_UPDATE)
        {
            if (states.absents(Fred::Object_State::server_update_prohibited))
            {
                if (states.presents(Fred::Object_State::server_transfer_prohibited))
                {
                    throw HasDifferentBlock();
                }
                throw ObjectNotBlocked();
            }
            request_id = get_unblock_id<UnblockChangesEmail, UnblockChangesPost>(
                    confirmation_method,
                    request,
                    locked_object,
                    log_request_id);
        }
        ctx.commit_transaction();
        return request_id;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Unknown error");
        throw;
    }
} // create_block_unblock_request

Buffer PublicRequest::create_public_request_pdf(
    unsigned long long public_request_id,
    Language lang,
    boost::shared_ptr<Fred::Document::Manager> manager)
{
    std::stringstream outstr;
    std::string lang_code;
    switch (lang)
    {
        case CS:
            lang_code = "cs";
            break;
        case EN:
            lang_code = "en";
            break;
        default:
            throw std::invalid_argument("language code not found");
    }
    std::auto_ptr<Fred::Document::Generator> g(
            manager.get()->createOutputGenerator(
                Fred::Document::GT_PUBLIC_REQUEST_PDF,
                outstr,
                lang_code));

    Fred::OperationContextCreator ctx;
    Fred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
    Fred::PublicRequestInfo request_info = Fred::InfoPublicRequest().exec(ctx, locked_request);

    std::map<std::string, short> post_types;
    post_types["authinfo_post_pif"] = 1;
    post_types["block_transfer_post_pif"] = 2;
    post_types["unblock_transfer_post_pif"] = 3;
    post_types["block_changes_post_pif"] = 4;
    post_types["unblock_changes_post_pif"] = 5;
    Database::Result type_name = ctx.get_conn().exec_params(
            "SELECT oreg.type, oreg.name "
            "FROM public_request pr "
                "JOIN public_request_objects_map prom "
                    "ON prom.request_id = pr.id "
                "JOIN enum_public_request_type eprt "
                    "ON eprt.id = pr.request_type "
                "JOIN object_registry oreg "
                    "ON oreg.id = prom.object_id "
            "WHERE pr.id = $1::bigint ",
            Database::query_param_list(public_request_id));
    if (type_name.size() != 1)
    {
        throw ObjectNotFound();
    }
    g->getInput() << "<?xml version='1.0' encoding='utf-8'?>"
        << "<enum_whois>"
        << "<public_request>"
        << "<type>" << post_types.at(request_info.get_type()) << "</type>"
        << "<handle type='"
        << static_cast<unsigned>(type_name[0][0])
        << "'>"
        << static_cast<std::string>(type_name[0][1])
        << "</handle>"
        << "<date>"
        << stringify(request_info.get_create_time().date())
        << "</date>"
        << "<id>"
        << public_request_id
        << "</id>"
        << "<replymail>"
        << request_info.get_email_to_answer().get_value_or("")
        << "</replymail>"
        << "</public_request>"
        << "</enum_whois>";
    g->closeInput();

    Buffer result;
    result.value = outstr.str();
    return result;
} // create_public_request_pdf

} // namespace Registry
} // namespace PublicRequestImpl
