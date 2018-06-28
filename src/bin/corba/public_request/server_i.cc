#include "src/bin/corba/public_request/server_i.hh"
#include "src/backend/buffer.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/bin/corba/util/corba_conversions_buffer.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/libfred/documents.hh"
#include "src/libfred/mailer.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/util/corba_conversion.hh"
#include "src/util/optional_value.hh"

#include <map>
#include <stdexcept>

namespace CorbaConversion {
namespace PublicRequest {

namespace {

Fred::Backend::PublicRequest::PublicRequestImpl::ObjectType::Enum unwrap_objecttype_pr_to_objecttype(Registry::PublicRequest::ObjectType_PR::Type object_type)
{
    switch (object_type)
    {
        case Registry::PublicRequest::ObjectType_PR::contact:
            return Fred::Backend::PublicRequest::PublicRequestImpl::ObjectType::contact;
        case Registry::PublicRequest::ObjectType_PR::nsset:
            return Fred::Backend::PublicRequest::PublicRequestImpl::ObjectType::nsset;
        case Registry::PublicRequest::ObjectType_PR::domain:
            return Fred::Backend::PublicRequest::PublicRequestImpl::ObjectType::domain;
        case Registry::PublicRequest::ObjectType_PR::keyset:
            return Fred::Backend::PublicRequest::PublicRequestImpl::ObjectType::keyset;
    }
    throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ObjectType_PR");
}

Optional<unsigned long long> unwrap_nullableulonglong_to_optional_unsigned_long_long(Registry::NullableULongLong* src_ptr)
{
    if (src_ptr == NULL)
    {
        return Optional<unsigned long long>();
    }
    unsigned long long dst;
    CorbaConversion::unwrap_int(src_ptr->_value(), dst);
    return dst;
}

} // namespace CorbaConversion::PublicRequest::{anonymous}

Server_i::Server_i(const std::string& _server_name)
    : pimpl_(new Fred::Backend::PublicRequest::PublicRequestImpl(_server_name))
{
}

Server_i::~Server_i()
{
}

::CORBA::ULongLong Server_i::create_authinfo_request_registry_email(
        Registry::PublicRequest::ObjectType_PR::Type object_type,
        const char* object_handle,
        Registry::NullableULongLong* log_request_id)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_authinfo_request_registry_email(
                unwrap_objecttype_pr_to_objecttype(object_type),
                LibFred::Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id));
        ::CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::NoContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectTransferProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_TRANSFER_PROHIBITED();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

namespace {

Fred::Backend::PublicRequest::PublicRequestImpl::ConfirmedBy::Enum unwrap_confirmedby_to_confirmedby(Registry::PublicRequest::ConfirmedBy::Type confirmation_method)
{
    switch (confirmation_method)
    {
        case Registry::PublicRequest::ConfirmedBy::signed_email:
            return Fred::Backend::PublicRequest::PublicRequestImpl::ConfirmedBy::email;
        case Registry::PublicRequest::ConfirmedBy::notarized_letter:
            return Fred::Backend::PublicRequest::PublicRequestImpl::ConfirmedBy::letter;
    }
    throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ConfirmedBy");
}

} // namespace CorbaConversion::PublicRequest::{anonymous}

CORBA::ULongLong Server_i::create_authinfo_request_non_registry_email(
        Registry::PublicRequest::ObjectType_PR::Type object_type,
        const char* object_handle,
        Registry::NullableULongLong* log_request_id,
        Registry::PublicRequest::ConfirmedBy::Type confirmation_method,
        const char* specified_email)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_authinfo_request_non_registry_email(
                unwrap_objecttype_pr_to_objecttype(object_type),
                LibFred::Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                unwrap_confirmedby_to_confirmedby(confirmation_method),
                LibFred::Corba::unwrap_string_from_const_char_ptr(specified_email));
        CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectTransferProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_TRANSFER_PROHIBITED();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::InvalidContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

namespace {

Fred::Backend::PublicRequest::PublicRequestImpl::LockRequestType::Enum unwrap_lockrequesttype_to_lockrequesttype(Registry::PublicRequest::LockRequestType::Type lock_request_type)
{
    switch (lock_request_type)
    {
        case Registry::PublicRequest::LockRequestType::block_transfer:
            return Fred::Backend::PublicRequest::PublicRequestImpl::LockRequestType::block_transfer;
        case Registry::PublicRequest::LockRequestType::block_transfer_and_update:
            return Fred::Backend::PublicRequest::PublicRequestImpl::LockRequestType::block_transfer_and_update;
        case Registry::PublicRequest::LockRequestType::unblock_transfer:
            return Fred::Backend::PublicRequest::PublicRequestImpl::LockRequestType::unblock_transfer;
        case Registry::PublicRequest::LockRequestType::unblock_transfer_and_update:
            return Fred::Backend::PublicRequest::PublicRequestImpl::LockRequestType::unblock_transfer_and_update;
    }
    throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::LockRequestType");
}

} // namespace CorbaConversion::PublicRequest::{anonymous}

CORBA::ULongLong Server_i::create_block_unblock_request(
        Registry::PublicRequest::ObjectType_PR::Type object_type,
        const char* object_handle,
        Registry::NullableULongLong* log_request_id,
        Registry::PublicRequest::ConfirmedBy::Type confirmation_method,
        Registry::PublicRequest::LockRequestType::Type lock_request_type)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_block_unblock_request(
                unwrap_objecttype_pr_to_objecttype(object_type),
                LibFred::Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                unwrap_confirmedby_to_confirmedby(confirmation_method),
                unwrap_lockrequesttype_to_lockrequesttype(lock_request_type));
        CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::InvalidContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::HasDifferentBlock& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::HAS_DIFFERENT_BLOCK();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectAlreadyBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_ALREADY_BLOCKED();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_BLOCKED();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::OperationProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OPERATION_PROHIBITED();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

CORBA::ULongLong Server_i::create_personal_info_request_registry_email(
        const char* contact_handle,
        Registry::NullableULongLong* log_request_id)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_personal_info_request_registry_email(
                LibFred::Corba::unwrap_string_from_const_char_ptr(contact_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id));
        ::CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::NoContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("create_personal_info_request_registry_email failed due to an unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}


CORBA::ULongLong Server_i::create_personal_info_request_non_registry_email(
        const char* contact_handle,
        Registry::NullableULongLong* log_request_id,
        Registry::PublicRequest::ConfirmedBy::Type confirmation_method,
        const char* specified_email)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_personal_info_request_non_registry_email(
                LibFred::Corba::unwrap_string_from_const_char_ptr(contact_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                unwrap_confirmedby_to_confirmedby(confirmation_method),
                LibFred::Corba::unwrap_string_from_const_char_ptr(specified_email));
        CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::InvalidContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("create_personal_info_request_non_registry_email failed due to an unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}


namespace {

Fred::Backend::PublicRequest::PublicRequestImpl::Language::Enum unwrap_language_to_language(Registry::PublicRequest::Language::Type lang)
{
    switch (lang)
    {
        case Registry::PublicRequest::Language::cs:
            return Fred::Backend::PublicRequest::PublicRequestImpl::Language::cs;
        case Registry::PublicRequest::Language::en:
            return Fred::Backend::PublicRequest::PublicRequestImpl::Language::en;
    }
    throw std::invalid_argument("language code not found");
}

} // namespace CorbaConversion::PublicRequest::{anonymous}

Registry::Buffer* Server_i::create_public_request_pdf(CORBA::ULongLong public_request_id, Registry::PublicRequest::Language::Type lang)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                pimpl_->create_public_request_pdf(
                        public_request_id,
                        unwrap_language_to_language(lang),
                        Fred::Backend::PublicRequest::PublicRequestImpl::get_default_document_manager());
        Registry::Buffer_var result = CorbaConversion::Util::wrap_Buffer(pdf_content);
        return result._retn();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::InvalidPublicRequestType& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw Registry::PublicRequest::INVALID_PUBLIC_REQUEST_TYPE();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

} // namespace CorbaConversion::PublicRequest
} // namespace CorbaConversion
