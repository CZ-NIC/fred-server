/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 *  implementational code for mojeid IDL interface
 *  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace/enum/idl/idl/MojeId.idl
 */

#include "src/bin/corba/Buffer.hh"
#include "src/bin/corba/util/corba_conversions_buffer.hh"
#include "src/bin/corba/mojeid/server_i.hh"
#include "src/bin/corba/mojeid/mojeid_corba_conversion.hh"
#include "src/backend/mojeid/mojeid.hh"

namespace Registry {
namespace MojeId {

typedef Registry::MojeID::Server IDL;

Server_i::Server_i(const std::string &_server_name)
:   impl_ptr_(new Fred::Backend::MojeId::MojeIdImpl(_server_name))
{
}

Server_i::~Server_i()
{
}

//   Methods corresponding to IDL attributes and operations
MojeID::ContactId Server_i::create_contact_prepare(
    const MojeID::CreateContact &_contact,
    const char *_trans_id,
    MojeID::LogRequestId _log_request_id,
    ::CORBA::String_out _identification)
{
    try {
        Fred::Backend::MojeIdImplData::CreateContact contact;
        CorbaConversion::unwrap_CreateContact(_contact, contact);
        std::string ident;
        const MojeID::ContactId contact_id = impl_ptr_->create_contact_prepare(
            contact,
            _trans_id,
            _log_request_id,
            ident);
        _identification = CorbaConversion::wrap_string(ident)._retn();
        return contact_id;
    }
    catch (const Fred::Backend::MojeIdImplData::RegistrationValidationResult &e) {
        CorbaConversion::raise_REGISTRATION_VALIDATION_ERROR(e);
        throw IDL::INTERNAL_SERVER_ERROR();//should never be used
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//create_contact_prepare

MojeID::InfoContact* Server_i::transfer_contact_prepare(
        const char *_handle,
        const char *_trans_id,
        MojeID::LogRequestId _log_request_id,
        ::CORBA::String_out _identification)
{
    try {
        Fred::Backend::MojeIdImplData::InfoContact contact;
        std::string ident;
        impl_ptr_->transfer_contact_prepare(
            _handle,
            _trans_id,
            _log_request_id,
            contact,
            ident);
        _identification = CorbaConversion::wrap_string(ident)._retn();
        return CorbaConversion::wrap_InfoContact(contact)._retn();
    }
    catch (const Fred::Backend::MojeIdImplData::AlreadyMojeidContact&) {
        throw IDL::ALREADY_MOJEID_CONTACT();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectAdminBlocked&) {
        throw IDL::OBJECT_ADMIN_BLOCKED();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectUserBlocked&) {
        throw IDL::OBJECT_USER_BLOCKED();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::RegistrationValidationResult &e) {
        CorbaConversion::raise_REGISTRATION_VALIDATION_ERROR(e);
        throw IDL::INTERNAL_SERVER_ERROR();//should never be used
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//transfer_contact_prepare

void Server_i::update_contact_prepare(
        MojeID::ContactId _contact_id,
        const MojeID::UpdateContact &_new_data,
        const char *_trans_id,
        MojeID::LogRequestId _log_request_id)
{
    try {
        Fred::Backend::MojeIdImplData::UpdateContact new_data;
        CorbaConversion::unwrap_UpdateContact(_new_data, new_data);
        impl_ptr_->update_contact_prepare(_contact_id, new_data, _trans_id, _log_request_id);
        return;
    }
    catch (const Fred::Backend::MojeIdImplData::UpdateContactPrepareValidationResult &e) {
        CorbaConversion::raise_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR(e);
    }
    catch (const Fred::Backend::MojeIdImplData::MessageLimitExceeded &e) {
        CorbaConversion::raise_MESSAGE_LIMIT_EXCEEDED(e);
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//update_contact_prepared

MojeID::InfoContact* Server_i::update_transfer_contact_prepare(
        const char *_username,
        const MojeID::UpdateTransferContact& _contact_data,
        const char *_trans_id,
        ::CORBA::ULongLong _log_request_id,
        ::CORBA::String_out _identification)
{
    try {
        Fred::Backend::MojeIdImplData::UpdateTransferContact contact_data;
        CorbaConversion::unwrap_UpdateTransferContact(_contact_data, contact_data);
        std::string ident;
        const Fred::Backend::MojeIdImplData::InfoContact info_contact =
        impl_ptr_->update_transfer_contact_prepare(_username, contact_data, _trans_id, _log_request_id, ident);
        _identification = CorbaConversion::wrap_string(ident)._retn();
        return CorbaConversion::wrap_InfoContact(info_contact)._retn();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist &e) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectAdminBlocked &e) {
        throw IDL::OBJECT_ADMIN_BLOCKED();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectUserBlocked &e) {
        throw IDL::OBJECT_USER_BLOCKED();
    }
    catch (const Fred::Backend::MojeIdImplData::RegistrationValidationResult &e) {
        CorbaConversion::raise_REGISTRATION_VALIDATION_ERROR(e);
        throw IDL::INTERNAL_SERVER_ERROR();//should never be used
    }
    catch (const Fred::Backend::MojeIdImplData::AlreadyMojeidContact&) {
        throw IDL::ALREADY_MOJEID_CONTACT();
    }
    catch(const Fred::Backend::MojeIdImplData::MessageLimitExceeded&) {
        throw IDL::MESSAGE_LIMIT_EXCEEDED();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

MojeID::InfoContact* Server_i::info_contact(
        const char *username)
{
    try {
        Fred::Backend::MojeIdImplData::InfoContact contact;
        impl_ptr_->info_contact(username, contact);
        return CorbaConversion::wrap_InfoContact(contact)._retn();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist &e) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

MojeID::InfoContactPublishFlags Server_i::get_contact_info_publish_flags(
    MojeID::ContactId contact_id)
{
    try {
        Fred::Backend::MojeIdImplData::InfoContactPublishFlags flags;
        impl_ptr_->get_contact_info_publish_flags(contact_id, flags);
        MojeID::InfoContactPublishFlags retval;
        CorbaConversion::wrap_InfoContactPublishFlags(flags, retval);
        return retval;
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist &e) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

MojeID::ContactId Server_i::process_registration_request(
        const char *ident_request_id,
        const char *password,
        MojeID::LogRequestId log_request_id)
{
    try {
        return impl_ptr_->process_registration_request(ident_request_id, password, log_request_id);
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationRequestDoesntExist&) {
        throw IDL::IDENTIFICATION_REQUEST_NOT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationFailed&) {
        throw IDL::IDENTIFICATION_FAILED();
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationAlreadyProcessed&) {
        throw IDL::IDENTIFICATION_ALREADY_PROCESSED();
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationAlreadyInvalidated&) {
        throw IDL::IDENTIFICATION_ALREADY_INVALIDATED();
    }
    catch (const Fred::Backend::MojeIdImplData::ContactChanged&) {
        throw IDL::OBJECT_CHANGED();
    }
    catch (const Fred::Backend::MojeIdImplData::AlreadyMojeidContact&) {
        throw IDL::ALREADY_MOJEID_CONTACT();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectAdminBlocked&) {
        throw IDL::OBJECT_ADMIN_BLOCKED();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectUserBlocked&) {
        throw IDL::OBJECT_USER_BLOCKED();
    }
    catch (const Fred::Backend::MojeIdImplData::ProcessRegistrationValidationResult &e) {
        CorbaConversion::raise_PROCESS_REGISTRATION_VALIDATION_ERROR(e);
        throw IDL::INTERNAL_SERVER_ERROR();//should never be used
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//process_registration_request

void Server_i::process_identification_request(
        MojeID::ContactId contact_id,
        const char *password,
        MojeID::LogRequestId log_request_id)
{
    try {
        impl_ptr_->process_identification_request(contact_id, password, log_request_id);
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationRequestDoesntExist&) {
        throw IDL::IDENTIFICATION_REQUEST_NOT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationFailed&) {
        throw IDL::IDENTIFICATION_FAILED();
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationAlreadyProcessed&) {
        throw IDL::IDENTIFICATION_ALREADY_PROCESSED();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//process_identification_request

void Server_i::commit_prepared_transaction(
        const char *_trans_id)
{
    try {
        impl_ptr_->commit_prepared_transaction(_trans_id);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//commit_prepared_transaction

void Server_i::rollback_prepared_transaction(
        const char *_trans_id)
{
    try {
        impl_ptr_->rollback_prepared_transaction(_trans_id);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//rollback_prepared_transaction

Registry::Buffer* Server_i::get_validation_pdf(
        MojeID::ContactId _contact_id)
{
    try {
        const Fred::Backend::Buffer pdf_content = impl_ptr_->get_validation_pdf(_contact_id);
        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_validation_pdf

void Server_i::create_validation_request(
        MojeID::ContactId _contact_id,
        MojeID::LogRequestId _log_request_id)
{
    try {
        impl_ptr_->create_validation_request(_contact_id, _log_request_id);
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::ValidationRequestExists&) {
        throw IDL::OBJECT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::ValidationAlreadyProcessed&) {
        throw IDL::VALIDATION_ALREADY_PROCESSED();
    }
    catch (const Fred::Backend::MojeIdImplData::CreateValidationRequestValidationResult &e) {
        CorbaConversion::raise_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR(e);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//create_validation_request

void Server_i::validate_contact(
        MojeID::ContactId _contact_id,
        MojeID::LogRequestId _log_request_id)
{
    try {
        impl_ptr_->validate_contact(_contact_id, _log_request_id);
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::ValidationAlreadyProcessed&) {
        throw IDL::VALIDATION_ALREADY_PROCESSED();
    }
    catch (const Fred::Backend::MojeIdImplData::CreateValidationRequestValidationResult &e) {
        CorbaConversion::raise_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR(e);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//validate_contact

MojeID::ContactStateInfoList* Server_i::get_contacts_state_changes(
        ::CORBA::ULong _last_hours)
{
    try {
        Fred::Backend::MojeIdImplData::ContactStateInfoList contacts_state_changes;
        impl_ptr_->get_contacts_state_changes(_last_hours, contacts_state_changes);
        return CorbaConversion::wrap_ContactStateInfoList(contacts_state_changes)._retn();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_contacts_state_changes

MojeID::ContactStateInfo* Server_i::get_contact_state(
        MojeID::ContactId _contact_id)
{
    try {
        Fred::Backend::MojeIdImplData::ContactStateInfo data;
        impl_ptr_->get_contact_state(_contact_id, data);
        return CorbaConversion::wrap_ContactStateInfo(data)._retn();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_contact_state

void Server_i::cancel_account_prepare(
        MojeID::ContactId _contact_id,
        const char *_trans_id,
        MojeID::LogRequestId _log_request_id)
{
    try {
        const std::string trans_id = _trans_id;
        impl_ptr_->cancel_account_prepare(_contact_id, trans_id, _log_request_id);
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//cancel_account_prepare

MojeID::ContactHandleList* Server_i::get_unregistrable_handles()
{
    try {
        Fred::Backend::MojeIdImplData::ContactHandleList unregistrable_handles;
        impl_ptr_->get_unregistrable_contact_handles(unregistrable_handles);
        return CorbaConversion::wrap_ContactHandleList(unregistrable_handles)._retn();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_unregistrable_handles

void Server_i::send_new_pin3(
      MojeID::ContactId contact_id,
      MojeID::LogRequestId log_request_id)
{
    try {
        impl_ptr_->send_new_pin3(contact_id, log_request_id);
    }
    catch (const Fred::Backend::MojeIdImplData::MessageLimitExceeded &e) {
        CorbaConversion::raise_MESSAGE_LIMIT_EXCEEDED(e);
    }
    catch (const Fred::Backend::MojeIdImplData::IdentificationRequestDoesntExist&) {
        throw IDL::IDENTIFICATION_REQUEST_NOT_EXISTS();
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::send_mojeid_card(
      MojeID::ContactId contact_id,
      MojeID::LogRequestId log_request_id)
{
    try {
        impl_ptr_->send_mojeid_card(contact_id, log_request_id);
    }
    catch (const Fred::Backend::MojeIdImplData::MessageLimitExceeded &e) {
        CorbaConversion::raise_MESSAGE_LIMIT_EXCEEDED(e);
    }
    catch (const Fred::Backend::MojeIdImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::generate_sms_messages()
{
    try {
        impl_ptr_->generate_sms_messages();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::enable_sms_messages_generation(::CORBA::Boolean enable)
{
    try {
        impl_ptr_->enable_sms_messages_generation(enable);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::generate_email_messages()
{
    try {
        impl_ptr_->generate_email_messages();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::enable_email_messages_generation(::CORBA::Boolean enable)
{
    try {
        impl_ptr_->enable_email_messages_generation(enable);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::generate_letter_messages()
{
    try {
        impl_ptr_->generate_letter_messages();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::enable_letter_messages_generation(::CORBA::Boolean enable)
{
    try {
        impl_ptr_->enable_letter_messages_generation(enable);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

} // namespace Registry::MojeID
} // namespace Registry
