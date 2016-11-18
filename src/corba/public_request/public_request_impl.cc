#include "src/public_request/public_request.h"
#include "src/fredlib/object/get_present_object_id.h"
#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/documents.h"
#include "src/corba/PublicRequest.hh"
#include "src/corba/NullableTypes.hh"
#include "src/corba/public_request/public_request_impl.h"
#include "src/corba/mailer_manager.h"
#include "src/corba/util/corba_conversions_string.h"
#include "util/corba_wrapper_decl.h"
#include "util/optional_value.h"
#include "util/cfg/handle_registry_args.h"
#include "util/cfg/config_handler_decl.h"

#include <boost/shared_ptr.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <map>
#include <stdexcept>

namespace Fred { struct NoContactEmail { virtual const char* what() const throw(); }; }

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
    ::CORBA::ULongLong result;
    try
    {
        boost::shared_ptr<Fred::Mailer::Manager> mailer_manager(
                new MailerManager(CorbaContainer::get_instance()->getNS()));
        result = pimpl_->create_authinfo_request_registry_email(
                unwrap_object_type(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                Corba::unwrap_string_from_const_char_ptr(reason),
                unwrap_ulonglong_optional_from_nullable(log_request_id),
                mailer_manager);
    }
    catch (const Fred::NoContactEmail& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INVALID_EMAIL();
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
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
    return result;
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
    ::CORBA::ULongLong result;
    try
    {
        result = pimpl_->create_authinfo_request_non_registry_email(
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
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).error(boost::diagnostic_information(e));
            throw Registry::PublicRequest::INVALID_EMAIL();
        }
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
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
    return result;
}

Registry::PublicRequestImpl::LockRequestType unwrap_lock_request_type(LockRequestType lock_request_type)
{
    switch (lock_request_type)
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
            throw std::invalid_argument("value doesn't exist in Registry::PublicRequest::LockRequestType");
    }
}

::CORBA::ULongLong Server_i::create_block_unblock_request(
    ObjectType_PR object_type,
    const char* object_handle,
    NullableULongLong* log_request_id,
    ConfirmationMethod confirmation_method,
    LockRequestType lock_request_type)
{
    ::CORBA::ULongLong result;
    try
    {
        result = pimpl_->create_block_unblock_request(
                unwrap_object_type(object_type),
                Corba::unwrap_string_from_const_char_ptr(object_handle),
                unwrap_ulonglong_optional_from_nullable(log_request_id),
                unwrap_confirmation_method(confirmation_method),
                unwrap_lock_request_type(lock_request_type));
    }
    catch (const Fred::UnknownObject& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Fred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).error(boost::diagnostic_information(e));
            throw Registry::PublicRequest::INVALID_EMAIL();
        }
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INTERNAL_SERVER_ERROR();
    }
    catch (const Registry::PublicRequestImpl::HasDifferentBlock& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::HAS_DIFFERENT_BLOCK();
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
    return result;
} // create_block_unblock_request

Registry::PublicRequestImpl::Language unwrap_language(Language lang)
{
    switch (lang)
    {
        case CS:
            return Registry::PublicRequestImpl::CS;
        case EN:
            return Registry::PublicRequestImpl::EN;
        default:
            throw std::invalid_argument("language code not found");
    }
}

Buffer* Server_i::create_public_request_pdf(CORBA::ULongLong public_request_id, Language lang)
{
    Registry::PublicRequest::Buffer_var result;
    try
    {
        HandleRegistryArgs* args = CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        boost::shared_ptr<Fred::Document::Manager> manager(
                Fred::Document::Manager::create(
                    args->docgen_path,
                    args->docgen_template_path,
                    args->fileclient_path,
                    CorbaContainer::get_instance()->getNS()->getHostName()));
        const std::string& impl_result =
            pimpl_->create_public_request_pdf(public_request_id, unwrap_language(lang), manager).value;
        result = new Registry::PublicRequest::Buffer();
        result->value.length(impl_result.size());
        if (! impl_result.empty())
        {
            std::memcpy(result->value.get_buffer(), impl_result.c_str(), impl_result.size());
        }
    }
    catch (const Registry::PublicRequestImpl::ObjectNotFound& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::OBJECT_NOT_FOUND();
    }
    catch (const Registry::PublicRequestImpl::InvalidPublicRequestType& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw Registry::PublicRequest::INVALID_PUBLIC_REQUEST_TYPE();
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
    return result._retn();
}

} // Registry
} // PublicRequest
