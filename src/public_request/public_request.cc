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

#include <boost/format.hpp>

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

template<typename Exception, typename EmailInterface, typename PostInterface>
unsigned long long get_id(
    bool states_are_wrong,
    ConfirmationMethod confirmation_method,
    const Fred::CreatePublicRequest& request,
    const Fred::PublicRequestsOfObjectLockGuardByObjectId& locked_object,
    const Optional<unsigned long long>& log_request_id)
{
    unsigned long long request_id;
    if (states_are_wrong)
    {
        throw Exception();
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
            request_id = get_id<ObjectAlreadyBlocked, BlockTransferEmail, BlockTransferPost>(
                    states.presents(Fred::Object_State::server_transfer_prohibited),
                    confirmation_method,
                    request,
                    locked_object,
                    log_request_id);
        }
        else if (lock_request_type == BLOCK_TRANSFER_AND_UPDATE)
        {
            request_id = get_id<ObjectAlreadyBlocked, BlockChangesEmail, BlockChangesPost>(
                    (states.presents(Fred::Object_State::server_transfer_prohibited) ||
                     states.presents(Fred::Object_State::server_update_prohibited)),
                    confirmation_method,
                    request,
                    locked_object,
                    log_request_id);
        }
        else if (lock_request_type == UNBLOCK_TRANSFER)
        {
            request_id = get_id<ObjectNotBlocked, UnblockTransferEmail, UnblockTransferPost>(
                    states.absents(Fred::Object_State::server_transfer_prohibited),
                    confirmation_method,
                    request,
                    locked_object,
                    log_request_id);
        }
        else if (lock_request_type == UNBLOCK_TRANSFER_AND_UPDATE)
        {
            request_id = get_id<ObjectNotBlocked, UnblockChangesEmail, UnblockChangesPost>(
                    (states.absents(Fred::Object_State::server_transfer_prohibited) ||
                     states.absents(Fred::Object_State::server_update_prohibited)),
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

} // namespace Registry
} // namespace PublicRequestImpl
