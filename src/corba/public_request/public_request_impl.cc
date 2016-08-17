#include "src/corba/public_request/public_request_impl.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/public_request/public_request.h"
#include "src/fredlib/object/get_present_object_id.h"

#include "util/optional_value.h"

namespace Registry
{
namespace PublicRequest
{

Fred::Object_Type::Enum unwrap_object_type(ObjectType_PR object_type)
{
    switch (object_type)
    {
        case Registry::PublicRequest::contact :
            return Fred::Object_Type::contact;
        case Registry::PublicRequest::nsset :
            return Fred::Object_Type::nsset;
        case Registry::PublicRequest::domain :
            return Fred::Object_Type::domain;
        case Registry::PublicRequest::keyset :
            return Fred::Object_Type::keyset;
        default :
            throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ObjectType_PR");
    }
}

inline Optional<unsigned long long> unwrap_ulonglong_optional_from_nullable(NullableULongLong* nullable)
{
    return (nullable) ? Optional<unsigned long long>(nullable->_value()) : Optional<unsigned long long>();
}

::CORBA::ULongLong Server_i::create_authinfo_request_registry_email(
    ObjectType_PR object_type,
    const char* object_handle,
    const char* reason,
    NullableULongLong* log_request_id)
{
    try
    {
        return pimpl_->create_authinfo_request_registry_email(
                unwrap_object_type(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                Corba::unwrap_string_from_const_char_ptr(reason),
                unwrap_ulonglong_optional_from_nullable(log_request_id));
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_EXISTS();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

Registry::PublicRequestImpl::ConfirmationMethod unwrap_confirmation_method(
        ConfirmationMethod confirmation_method)
{
    switch (confirmation_method)
    {
        case EMAIL_WITH_QUALIFIED_CERTIFICATE :
            return Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE;
        case LETTER_WITH_AUTHENTICATED_SIGNATURE :
            return Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE;
        default:
            throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ConfirmationMethod");
    }
}

::CORBA::ULongLong Server_i::create_authinfo_request_non_registry_email(
    ObjectType_PR object_type,
    const char* object_handle,
    const char* reason,
    NullableULongLong* log_request_id,
    ConfirmationMethod confirmation_method,
    const char* specified_email)
{
    try
    {
        return pimpl_->create_authinfo_request_non_registry_email(
                unwrap_object_type(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                Corba::unwrap_string_from_const_char_ptr(reason),
                unwrap_ulonglong_optional_from_nullable(log_request_id),
                unwrap_confirmation_method(confirmation_method),
                Corba::unwrap_string_from_const_char_ptr(specified_email));
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_EXISTS();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

Registry::PublicRequestImpl::ObjectBlockType unwrap_object_block_type(ObjectBlockType object_block_type)
{
    switch (object_block_type)
    {
        case BLOCK_TRANSFER:
            return Registry::PublicRequestImpl::BLOCK_TRANSFER;
        case BLOCK_TRANSFER_AND_UPDATE:
            return Registry::PublicRequestImpl::BLOCK_TRANSFER_AND_UPDATE;
        case UNBLOCK_TRANSFER:
            return Registry::PublicRequestImpl::UNBLOCK_TRANSFER;
        case UNBLOCK_TRANSFER_AND_UPDATE:
            return Registry::PublicRequestImpl::UNBLOCK_TRANSFER_AND_UPDATE;
        default:
            throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ObjectBlockType");
    }
}

::CORBA::ULongLong Server_i::create_block_unblock_request(
    ObjectType_PR object_type,
    const char* object_handle,
    NullableULongLong* log_request_id,
    ConfirmationMethod confirmation_method,
    ObjectBlockType object_block_type)
{
    try
    {
        return pimpl_->create_block_unblock_request(
                unwrap_object_type(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_ulonglong_optional_from_nullable(log_request_id),
                unwrap_confirmation_method(confirmation_method),
                unwrap_object_block_type(object_block_type));
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_EXISTS();
    }
    catch (const Registry::PublicRequestImpl::ObjectAlreadyBlocked& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_ALREADY_BLOCKED();
    }
    catch (const Registry::PublicRequestImpl::ObjectNotBlocked& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_BLOCKED();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
} // create_block_unblock_request

} // Registry
} // PublicRequest
