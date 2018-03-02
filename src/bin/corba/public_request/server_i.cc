#include "src/bin/corba/public_request/server_i.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/bin/corba/util/corba_conversions_buffer.hh"
#include "src/backend/buffer.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/mailer.hh"
#include "src/libfred/documents.hh"
#include "src/util/optional_value.hh"
#include "src/util/corba_conversion.hh"

#include <map>
#include <stdexcept>

namespace Registry {
namespace PublicRequest {
namespace {

PublicRequestImpl::ObjectType::Enum unwrap_objecttype_pr_to_objecttype(ObjectType_PR::Type object_type)
{
    switch (object_type)
    {
        case ObjectType_PR::contact: return PublicRequestImpl::ObjectType::contact;
        case ObjectType_PR::nsset: return PublicRequestImpl::ObjectType::nsset;
        case ObjectType_PR::domain: return PublicRequestImpl::ObjectType::domain;
        case ObjectType_PR::keyset: return PublicRequestImpl::ObjectType::keyset;
    }
    throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ObjectType_PR");
}

Optional<unsigned long long> unwrap_nullableulonglong_to_optional_unsigned_long_long(NullableULongLong* src_ptr)
{
    if (src_ptr == NULL)
    {
        return Optional<unsigned long long>();
    }
    unsigned long long dst;
    CorbaConversion::unwrap_int(src_ptr->_value(), dst);
    return dst;
}

} // namespace Registry::PublicRequest::{anonymous}

Server_i::Server_i(const std::string& _server_name)
    : pimpl_(new PublicRequestImpl(_server_name))
{
}

Server_i::~Server_i()
{
}

::CORBA::ULongLong Server_i::create_authinfo_request_registry_email(
    ObjectType_PR::Type object_type,
    const char* object_handle,
    NullableULongLong* log_request_id)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_authinfo_request_registry_email(
                unwrap_objecttype_pr_to_objecttype(object_type),
                LibFred::Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                PublicRequestImpl::get_default_mailer_manager());
        ::CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const PublicRequestImpl::NoContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw INVALID_EMAIL();
    }
    catch (const PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_NOT_FOUND();
    }
    catch (const PublicRequestImpl::ObjectTransferProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_TRANSFER_PROHIBITED();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw INTERNAL_SERVER_ERROR();
    }
}

namespace {

PublicRequestImpl::ConfirmedBy::Enum unwrap_confirmedby_to_confirmedby(ConfirmedBy::Type confirmation_method)
{
    switch (confirmation_method)
    {
        case ConfirmedBy::signed_email:
            return PublicRequestImpl::ConfirmedBy::email;
        case ConfirmedBy::notarized_letter:
            return PublicRequestImpl::ConfirmedBy::letter;
    }
    throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ConfirmedBy");
}

} // namespace Registry::PublicRequest::{anonymous}

CORBA::ULongLong Server_i::create_authinfo_request_non_registry_email(
    ObjectType_PR::Type object_type,
    const char* object_handle,
    NullableULongLong* log_request_id,
    ConfirmedBy::Type confirmation_method,
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
    catch (const PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_NOT_FOUND();
    }
    catch (const PublicRequestImpl::ObjectTransferProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_TRANSFER_PROHIBITED();
    }
    catch (const PublicRequestImpl::InvalidContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw INVALID_EMAIL();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw INTERNAL_SERVER_ERROR();
    }
}

namespace {

PublicRequestImpl::LockRequestType::Enum unwrap_lockrequesttype_to_lockrequesttype(LockRequestType::Type lock_request_type)
{
    switch (lock_request_type)
    {
        case LockRequestType::block_transfer:
            return PublicRequestImpl::LockRequestType::block_transfer;
        case LockRequestType::block_transfer_and_update:
            return PublicRequestImpl::LockRequestType::block_transfer_and_update;
        case LockRequestType::unblock_transfer:
            return PublicRequestImpl::LockRequestType::unblock_transfer;
        case LockRequestType::unblock_transfer_and_update:
            return PublicRequestImpl::LockRequestType::unblock_transfer_and_update;
    }
    throw std::invalid_argument("value doesn't exist in LockRequestType");
}

} // namespace Registry::PublicRequest::{anonymous}

CORBA::ULongLong Server_i::create_block_unblock_request(
    ObjectType_PR::Type object_type,
    const char* object_handle,
    NullableULongLong* log_request_id,
    ConfirmedBy::Type confirmation_method,
    LockRequestType::Type lock_request_type)
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
    catch (const PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_NOT_FOUND();
    }
    catch (const PublicRequestImpl::InvalidContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw INVALID_EMAIL();
    }
    catch (const PublicRequestImpl::HasDifferentBlock& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw HAS_DIFFERENT_BLOCK();
    }
    catch (const PublicRequestImpl::ObjectAlreadyBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_ALREADY_BLOCKED();
    }
    catch (const PublicRequestImpl::ObjectNotBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_NOT_BLOCKED();
    }
    catch (const PublicRequestImpl::OperationProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OPERATION_PROHIBITED();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw INTERNAL_SERVER_ERROR();
    }
}

namespace {

PublicRequestImpl::Language::Enum unwrap_language_to_language(Language::Type lang)
{
    switch (lang)
    {
        case Language::cs: return PublicRequestImpl::Language::cs;
        case Language::en: return PublicRequestImpl::Language::en;
    }
    throw std::invalid_argument("language code not found");
}

} // namespace Registry::PublicRequest::{anonymous}

Buffer* Server_i::create_public_request_pdf(CORBA::ULongLong public_request_id, Language::Type lang)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                pimpl_->create_public_request_pdf(
                        public_request_id,
                        unwrap_language_to_language(lang),
                        PublicRequestImpl::get_default_document_manager());
        Buffer_var result = CorbaConversion::Util::wrap_Buffer(pdf_content);
        return result._retn();
    }
    catch (const PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OBJECT_NOT_FOUND();
    }
    catch (const PublicRequestImpl::InvalidPublicRequestType& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw INVALID_PUBLIC_REQUEST_TYPE();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unexpected exception");
        throw INTERNAL_SERVER_ERROR();
    }
}

} // namespace Registry::PublicRequest
} // namespace Registry
