#include "src/corba/public_request/public_request_impl.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/public_request/public_request.h"
#include "src/fredlib/object/get_present_object_id.h"

#include "util/optional_value.h"

namespace Registry
{
namespace PublicRequest
{

Fred::Object_Type::Enum unwrap_object_type(ObjectType object_type)
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
            throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ObjectType");
    }
}
/* inline */ Optional<unsigned long long> unwrap_ulonglong_optional_from_nullable(
    /* ::Registry::PublicRequest:: */NullableULongLong* nullable)
{
    return (nullable) ? Optional<unsigned long long>(nullable->_value()) : Optional<unsigned long long>();
}

::CORBA::ULongLong Server_i::create_authinfo_request_registry_email(
    /* Registry::PublicRequest:: */ObjectType object_type,
    const char* object_handle,
    const char* reason,
    /* ::Registry::PublicRequest:: */NullableULongLong* log_request_id)
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

// Registry::PublicRequestImpl::ConfirmationMethod unwrap_confirmation_method(
//         ConfirmationMethod confirmation_method)
// {
//     switch (confirmation_method)
//     {
//         case EMAIL_WITH_QUALIFIED_CERTIFICATE :
//             return Fred::Object_Type::contact;
//         case LETTER_WITH_AUTHENTICATED_SIGNATURE :
//             return Fred::Object_Type::nsset;
//         default :
//             throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ObjectType");
//     }
// }
// 
::CORBA::ULongLong Server_i::create_authinfo_request_non_registry_email(
    /* Registry::PublicRequest:: */ObjectType object_type,
    const char* object_handle,
    const char* reason,
    /* ::Registry::PublicRequest:: */NullableULongLong* log_request_id,
    /* Registry::PublicRequest:: */ConfirmationMethod confirmation_method,
    const char* specified_email)
{
    try
    {
        return pimpl_->create_authinfo_request_non_registry_email(
                unwrap_object_type(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                Corba::unwrap_string_from_const_char_ptr(reason),
                unwrap_ulonglong_optional_from_nullable(log_request_id),
                static_cast<Registry::PublicRequestImpl::ConfirmationMethod>(confirmation_method), // change to func
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

::CORBA::ULongLong Server_i::create_block_unblock_request(
    /* Registry::PublicRequest:: */ObjectType object_type,
    const char* object_handle,
    /* ::Registry::PublicRequest:: */NullableULongLong* log_request_id,
    /* Registry::PublicRequest:: */ConfirmationMethod confirmation_method,
    /* Registry::PublicRequest:: */ObjectBlockType object_block_type)
{
    try
    {
        return pimpl_->create_block_unblock_request(
                unwrap_object_type(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_ulonglong_optional_from_nullable(log_request_id),
                static_cast<Registry::PublicRequestImpl::ConfirmationMethod>(confirmation_method), // to func
                static_cast<Registry::PublicRequestImpl::ObjectBlockType>(object_block_type)); // to func
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
}

} // Registry
} // PublicRequest
