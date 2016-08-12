#include "src/public_request/public_request.h"
#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/object/get_present_object_id.h"

namespace Registry
{
namespace PublicRequestImpl
{

unsigned long long PublicRequest::create_authinfo_request_registry_email(
    Fred::Object_Type::Enum object_type,
    const std::string& object_handle,
    const std::string& reason,
    Optional<unsigned long long>& log_request_id)
{
    try
    {
        Fred::OperationContextCreator ctx;
        Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(
                ctx,
                Fred::get_present_object_id(ctx, object_type, object_handle));
        return Fred::CreatePublicRequest(reason, Optional< std::string >(), Optional<unsigned long long>())
            .exec(locked_object, AuthinfoAuto(), log_request_id);
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
    Optional<unsigned long long>& log_request_id,
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

//    raises (OBJECT_NOT_FOUND, INTERNAL_SERVER_ERROR, 
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
        Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(
                ctx,
                Fred::get_present_object_id(ctx, object_type, object_handle));
        unsigned long long request_id;
        Fred::CreatePublicRequest c_p_r = Fred::CreatePublicRequest(
                Optional<std::string>(),
                Optional<std::string>(),
                Optional<unsigned long long>());
        //TODO OBJECT_ALREADY_BLOCKED, OBJECT_NOT_BLOCKED
        if (confirmation_method == EMAIL_WITH_QUALIFIED_CERTIFICATE)
        {
            if (lock_request_type == BLOCK_TRANSFER)
            {
                request_id = c_p_r.exec(locked_object, BlockTransferEmail(), log_request_id);
            }
            else if (lock_request_type == BLOCK_TRANSFER_AND_UPDATE)
            {
                request_id = c_p_r.exec(locked_object, BlockChangesEmail(), log_request_id);
            }
            else if (lock_request_type == UNBLOCK_TRANSFER)
            {
                request_id = c_p_r.exec(locked_object, UnblockTransferEmail(), log_request_id);
            }
            else if (lock_request_type == UNBLOCK_TRANSFER_AND_UPDATE)
            {
                request_id = c_p_r.exec(locked_object, UnblockChangesEmail(), log_request_id);
            }
        }
        else if (confirmation_method == LETTER_WITH_AUTHENTICATED_SIGNATURE)
        {
            if (lock_request_type == BLOCK_TRANSFER)
            {
                request_id = c_p_r.exec(locked_object, BlockTransferPost(), log_request_id);
            }
            else if (lock_request_type == BLOCK_TRANSFER_AND_UPDATE)
            {
                request_id = c_p_r.exec(locked_object, BlockChangesPost(), log_request_id);
            }
            else if (lock_request_type == UNBLOCK_TRANSFER)
            {
                request_id = c_p_r.exec(locked_object, UnblockTransferPost(), log_request_id);
            }
            else if (lock_request_type == UNBLOCK_TRANSFER_AND_UPDATE)
            {
                request_id = c_p_r.exec(locked_object, UnblockChangesPost(), log_request_id);
            }
        }
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

} // namespace Registry
} // namespace PublicRequestImpl
