/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/corba/public_request/server_i.hh"
#include "src/backend/buffer.hh"
#include "src/backend/public_request/create_authinfo_request_non_registry_email.hh"
#include "src/backend/public_request/create_authinfo_request_registry_email.hh"
#include "src/backend/public_request/create_block_unblock_request.hh"
#include "src/backend/public_request/create_personal_info_request_non_registry_email.hh"
#include "src/backend/public_request/create_personal_info_request_registry_email.hh"
#include "src/backend/public_request/create_public_request_pdf.hh"
#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/get_default_document_manager.hh"
#include "src/backend/public_request/language.hh"
#include "src/backend/public_request/lock_request_type.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/bin/corba/util/corba_conversions_buffer.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/deprecated/libfred/documents.hh"
#include "libfred/mailer.hh"
#include "libfred/public_request/create_public_request.hh"
#include "src/util/corba_conversion.hh"
#include "util/optional_value.hh"

#include <map>
#include <stdexcept>

namespace CorbaConversion {
namespace PublicRequest {

namespace {

Fred::Backend::PublicRequest::ObjectType unwrap_objecttype_pr_to_objecttype(
        Registry::PublicRequest::ObjectType_PR::Type object_type)
{
    switch (object_type)
    {
        case Registry::PublicRequest::ObjectType_PR::contact:
            return Fred::Backend::PublicRequest::ObjectType::contact;
        case Registry::PublicRequest::ObjectType_PR::nsset:
            return Fred::Backend::PublicRequest::ObjectType::nsset;
        case Registry::PublicRequest::ObjectType_PR::domain:
            return Fred::Backend::PublicRequest::ObjectType::domain;
        case Registry::PublicRequest::ObjectType_PR::keyset:
            return Fred::Backend::PublicRequest::ObjectType::keyset;
    }
    throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::ObjectType_PR");
}

Optional<unsigned long long> unwrap_nullableulonglong_to_optional_unsigned_long_long(
        Registry::NullableULongLong* src_ptr)
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

Server_i::Server_i(const std::string& _server_name [[gnu::unused]])
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
        const unsigned long long public_request_id =
                Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                        unwrap_objecttype_pr_to_objecttype(object_type),
                        LibFred::Corba::unwrap_string_from_const_char_ptr(object_handle),
                        unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id));
        ::CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::NoContactEmail& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::ObjectNotFound& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::ObjectTransferProhibited& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_TRANSFER_PROHIBITED();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER.error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

namespace {

Fred::Backend::PublicRequest::ConfirmedBy unwrap_confirmedby_to_confirmedby(
        Registry::PublicRequest::ConfirmedBy::Type confirmation_method)
{
    switch (confirmation_method)
    {
        case Registry::PublicRequest::ConfirmedBy::signed_email:
            return Fred::Backend::PublicRequest::ConfirmedBy::email;
        case Registry::PublicRequest::ConfirmedBy::notarized_letter:
            return Fred::Backend::PublicRequest::ConfirmedBy::letter;
        case Registry::PublicRequest::ConfirmedBy::government:
            return Fred::Backend::PublicRequest::ConfirmedBy::government;
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
        const unsigned long long public_request_id =
                Fred::Backend::PublicRequest::create_authinfo_request_non_registry_email(
                        unwrap_objecttype_pr_to_objecttype(object_type),
                        LibFred::Corba::unwrap_string_from_const_char_ptr(object_handle),
                        unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                        unwrap_confirmedby_to_confirmedby(confirmation_method),
                        LibFred::Corba::unwrap_string_from_const_char_ptr(specified_email));
        CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::ObjectNotFound& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::ObjectTransferProhibited& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_TRANSFER_PROHIBITED();
    }
    catch (const Fred::Backend::PublicRequest::InvalidContactEmail& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER.error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

namespace {

Fred::Backend::PublicRequest::LockRequestType::Enum unwrap_lockrequesttype_to_lockrequesttype(
        Registry::PublicRequest::LockRequestType::Type lock_request_type)
{
    switch (lock_request_type)
    {
        case Registry::PublicRequest::LockRequestType::block_transfer:
            return Fred::Backend::PublicRequest::LockRequestType::block_transfer;
        case Registry::PublicRequest::LockRequestType::block_transfer_and_update:
            return Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update;
        case Registry::PublicRequest::LockRequestType::unblock_transfer:
            return Fred::Backend::PublicRequest::LockRequestType::unblock_transfer;
        case Registry::PublicRequest::LockRequestType::unblock_transfer_and_update:
            return Fred::Backend::PublicRequest::LockRequestType::unblock_transfer_and_update;
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
        const unsigned long long public_request_id =
                Fred::Backend::PublicRequest::create_block_unblock_request(
                        unwrap_objecttype_pr_to_objecttype(object_type),
                        LibFred::Corba::unwrap_string_from_const_char_ptr(object_handle),
                        unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                        unwrap_confirmedby_to_confirmedby(confirmation_method),
                        unwrap_lockrequesttype_to_lockrequesttype(lock_request_type));
        CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::ObjectNotFound& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::InvalidContactEmail& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::HasDifferentBlock& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::HAS_DIFFERENT_BLOCK();
    }
    catch (const Fred::Backend::PublicRequest::ObjectAlreadyBlocked& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_ALREADY_BLOCKED();
    }
    catch (const Fred::Backend::PublicRequest::ObjectNotBlocked& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_BLOCKED();
    }
    catch (const Fred::Backend::PublicRequest::OperationProhibited& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OPERATION_PROHIBITED();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER.error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

CORBA::ULongLong Server_i::create_personal_info_request_registry_email(
        const char* contact_handle,
        Registry::NullableULongLong* log_request_id)
{
    try
    {
        const unsigned long long public_request_id =
                Fred::Backend::PublicRequest::create_personal_info_request_registry_email(
                        LibFred::Corba::unwrap_string_from_const_char_ptr(contact_handle),
                        unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id));
        ::CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::NoContactEmail& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::ObjectNotFound& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER.error("create_personal_info_request_registry_email failed due to an unexpected exception");
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
        const unsigned long long public_request_id =
                Fred::Backend::PublicRequest::create_personal_info_request_non_registry_email(
                        LibFred::Corba::unwrap_string_from_const_char_ptr(contact_handle),
                        unwrap_nullableulonglong_to_optional_unsigned_long_long(log_request_id),
                        unwrap_confirmedby_to_confirmedby(confirmation_method),
                        LibFred::Corba::unwrap_string_from_const_char_ptr(specified_email));
        CORBA::ULongLong result;
        CorbaConversion::wrap_int(public_request_id, result);
        return result;
    }
    catch (const Fred::Backend::PublicRequest::InvalidContactEmail& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::Backend::PublicRequest::ObjectNotFound& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER.error("create_personal_info_request_non_registry_email failed due to an unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}


namespace {

Fred::Backend::PublicRequest::Language::Enum unwrap_language_to_language(
        Registry::PublicRequest::Language::Type lang)
{
    switch (lang)
    {
        case Registry::PublicRequest::Language::cs:
            return Fred::Backend::PublicRequest::Language::cs;
        case Registry::PublicRequest::Language::en:
            return Fred::Backend::PublicRequest::Language::en;
    }
    throw std::invalid_argument("language code not found");
}

} // namespace CorbaConversion::PublicRequest::{anonymous}

Registry::Buffer* Server_i::create_public_request_pdf(
        CORBA::ULongLong public_request_id,
        Registry::PublicRequest::Language::Type lang)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                Fred::Backend::PublicRequest::create_public_request_pdf(
                        public_request_id,
                        unwrap_language_to_language(lang),
                        Fred::Backend::PublicRequest::get_default_document_manager());
        Registry::Buffer_var result = CorbaConversion::Util::wrap_Buffer(pdf_content);
        return result._retn();
    }
    catch (const Fred::Backend::PublicRequest::ObjectNotFound& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::PublicRequest::InvalidPublicRequestType& e)
    {
        LOGGER.info(e.what());
        throw Registry::PublicRequest::INVALID_PUBLIC_REQUEST_TYPE();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (...)
    {
        LOGGER.error("unexpected exception");
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
}

} // namespace CorbaConversion::PublicRequest
} // namespace CorbaConversion
