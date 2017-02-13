#include "src/corba/public_request/server_i.h"
#include "src/public_request/public_request.h"
#include "src/fredlib/object/get_present_object_id.h"
#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/documents.h"
#include "src/fredlib/send_authinfo.h"
#include "src/corba/util/corba_conversions_string.h"
#include "util/optional_value.h"
#include "util/corba_conversion.h"

#include <map>
#include <stdexcept>

namespace Registry {
namespace PublicRequest {
namespace {

PublicRequestImpl::ObjectType::Enum unwrap_objecttype_pr_to_objecttype(ObjectType_PR object_type)
{
    switch (object_type)
    {
        case contact: return PublicRequestImpl::ObjectType::contact;
        case nsset: return PublicRequestImpl::ObjectType::nsset;
        case domain: return PublicRequestImpl::ObjectType::domain;
        case keyset: return PublicRequestImpl::ObjectType::keyset;
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

}//namespace Registry::PublicRequest::{anonymous}

Server_i::Server_i()
    : pimpl_(new PublicRequestImpl)
{
}

::CORBA::ULongLong Server_i::create_authinfo_request_registry_email(
    ObjectType_PR object_type,
    const char* object_handle,
    NullableULongLong* log_request_id)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_authinfo_request_registry_email(
                unwrap_objecttype_pr_to_objecttype(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
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

PublicRequestImpl::ConfirmationMethod::Enum unwrap_confirmationmethod_to_confirmationmethod(
        ConfirmationMethod confirmation_method)
{
    switch (confirmation_method)
    {
        case EMAIL_WITH_QUALIFIED_CERTIFICATE :
            return PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate;
        case LETTER_WITH_AUTHENTICATED_SIGNATURE :
            return PublicRequestImpl::ConfirmationMethod::letter_with_authenticated_signature;
    }
    throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ConfirmationMethod");
}

}//namespace Registry::PublicRequest::{anonymous}

CORBA::ULongLong Server_i::create_authinfo_request_non_registry_email(
    ObjectType_PR object_type,
    const char* object_handle,
    NullableULongLong* log_request_id,
    ConfirmationMethod confirmation_method,
    const char* specified_email)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_authinfo_request_non_registry_email(
                unwrap_objecttype_pr_to_objecttype(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                unwrap_confirmationmethod_to_confirmationmethod(confirmation_method),
                Corba::unwrap_string_from_const_char_ptr(specified_email));
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

PublicRequestImpl::LockRequestType::Enum unwrap_lockrequesttype_to_lockrequesttype(LockRequestType lock_request_type)
{
    switch (lock_request_type)
    {
        case BLOCK_TRANSFER:
            return PublicRequestImpl::LockRequestType::block_transfer;
        case BLOCK_TRANSFER_AND_UPDATE:
            return PublicRequestImpl::LockRequestType::block_transfer_and_update;
        case UNBLOCK_TRANSFER:
            return PublicRequestImpl::LockRequestType::unblock_transfer;
        case UNBLOCK_TRANSFER_AND_UPDATE:
            return PublicRequestImpl::LockRequestType::unblock_transfer_and_update;
    }
    throw std::invalid_argument("value doesn't exist in LockRequestType");
}

}//namespace Registry::PublicRequest::{anonymous}

CORBA::ULongLong Server_i::create_block_unblock_request(
    ObjectType_PR object_type,
    const char* object_handle,
    NullableULongLong* log_request_id,
    ConfirmationMethod confirmation_method,
    LockRequestType lock_request_type)
{
    try
    {
        const unsigned long long public_request_id = pimpl_->create_block_unblock_request(
                unwrap_objecttype_pr_to_objecttype(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                unwrap_confirmationmethod_to_confirmationmethod(confirmation_method),
                unwrap_lockrequesttype_to_lockrequesttype(lock_request_type));
        CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw OBJECT_NOT_FOUND();
    }
    catch (const PublicRequestImpl::InvalidContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw INVALID_EMAIL();
    }
    catch (const PublicRequestImpl::HasDifferentBlock& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw HAS_DIFFERENT_BLOCK();
    }
    catch (const PublicRequestImpl::ObjectAlreadyBlocked& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw OBJECT_ALREADY_BLOCKED();
    }
    catch (const PublicRequestImpl::ObjectNotBlocked& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw OBJECT_NOT_BLOCKED();
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

PublicRequestImpl::Language::Enum unwrap_language_to_language(Language lang)
{
    switch (lang)
    {
        case CS: return PublicRequestImpl::Language::cs;
        case EN: return PublicRequestImpl::Language::en;
    }
    throw std::invalid_argument("language code not found");
}

void wrap_buffer(const PublicRequestImpl::Buffer& src, Buffer_var& dst)
{
    dst = new Buffer();
    dst->value.length(src.value.size());
    if (!src.value.empty())
    {
        std::memcpy(dst->value.get_buffer(), src.value.c_str(), src.value.size());
    }
}

}//namespace Registry::PublicRequest::{anonymous}

Buffer* Server_i::create_public_request_pdf(CORBA::ULongLong public_request_id, Language lang)
{
    try
    {
        const Registry::PublicRequestImpl::Buffer pdf_content =
                pimpl_->create_public_request_pdf(
                        public_request_id,
                        unwrap_language_to_language(lang),
                        PublicRequestImpl::get_default_document_manager());
        Registry::PublicRequest::Buffer_var result;
        wrap_buffer(pdf_content, result);
        return result._retn();
    }
    catch (const PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw OBJECT_NOT_FOUND();
    }
    catch (const PublicRequestImpl::InvalidPublicRequestType& e)
    {
        LOGGER(PACKAGE).error(e.what());
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

}//namespace Registry::PublicRequest
}//namespace Registry
