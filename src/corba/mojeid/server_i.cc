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
 *  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/MojeID.idl
 */

#include "src/corba/mojeid/server_i.h"
#include "src/corba/mojeid/mojeid_corba_conversion.h"
#include "src/mojeid/mojeid.h"

namespace Registry {
namespace MojeID {

typedef Server IDL;

Server_i::Server_i(const std::string &_server_name)
:   impl_ptr_(new MojeIDImpl(_server_name))
{
}

Server_i::~Server_i()
{
}

//   Methods corresponding to IDL attributes and operations
ContactId Server_i::create_contact_prepare(
    const CreateContact &_contact,
    const char *_trans_id,
    LogRequestId _log_request_id,
    ::CORBA::String_out _identification)
{
    try {
        MojeIDImplData::CreateContact contact;
        CorbaConversion::unwrap_CreateContact(_contact, contact);
        std::string ident;
        const ContactId contact_id = impl_ptr_->create_contact_prepare(
            contact,
            _trans_id,
            _log_request_id,
            ident);
        _identification = CorbaConversion::wrap_string(ident)._retn();
        return contact_id;
    }
    catch (const MojeIDImplData::RegistrationValidationResult &e) {
        CorbaConversion::raise_REGISTRATION_VALIDATION_ERROR(e);
        throw IDL::INTERNAL_SERVER_ERROR();//should never be used
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//create_contact_prepare

InfoContact* Server_i::transfer_contact_prepare(
        const char *_handle,
        const char *_trans_id,
        LogRequestId _log_request_id,
        ::CORBA::String_out _identification)
{
    try {
        MojeIDImplData::InfoContact contact;
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
    catch (const MojeIDImplData::AlreadyMojeidContact&) {
        throw IDL::ALREADY_MOJEID_CONTACT();
    }
    catch (const MojeIDImplData::ObjectAdminBlocked&) {
        throw IDL::OBJECT_ADMIN_BLOCKED();
    }
    catch (const MojeIDImplData::ObjectUserBlocked&) {
        throw IDL::OBJECT_USER_BLOCKED();
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (const MojeIDImplData::RegistrationValidationResult &e) {
        CorbaConversion::raise_REGISTRATION_VALIDATION_ERROR(e);
        throw IDL::INTERNAL_SERVER_ERROR();//should never be used
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//transfer_contact_prepare

void Server_i::update_contact_prepare(
        ContactId _contact_id,
        const UpdateContact &_new_data,
        const char *_trans_id,
        LogRequestId _log_request_id)
{
    try {
        MojeIDImplData::UpdateContact new_data;
        CorbaConversion::unwrap_UpdateContact(_new_data, new_data);
        impl_ptr_->update_contact_prepare(_contact_id, new_data, _trans_id, _log_request_id);
        return;
    }
    catch (const MojeIDImplData::UpdateContactPrepareValidationResult &e) {
        CorbaConversion::raise_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR(e);
    }
    catch (const MojeIDImplData::MessageLimitExceeded &e) {
        CorbaConversion::raise_MESSAGE_LIMIT_EXCEEDED(e);
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//update_contact_prepared

InfoContact* Server_i::update_transfer_contact_prepare(
        const char *_username,
        const UpdateTransferContact& _contact_data,
        const char *_trans_id,
        ::CORBA::ULongLong _log_request_id)
{
    try {
        MojeIDImplData::UpdateTransferContact contact_data;
        CorbaConversion::unwrap_UpdateTransferContact(_contact_data, contact_data);
        const MojeIDImplData::InfoContact info_contact =
        impl_ptr_->update_transfer_contact_prepare(_username, contact_data, _trans_id, _log_request_id);
        return CorbaConversion::wrap_InfoContact(info_contact)._retn();
    }
    catch (const MojeIDImplData::ObjectDoesntExist &e) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (const MojeIDImplData::ObjectAdminBlocked &e) {
        throw IDL::OBJECT_ADMIN_BLOCKED();
    }
    catch (const MojeIDImplData::ObjectUserBlocked &e) {
        throw IDL::OBJECT_USER_BLOCKED();
    }
    catch (const MojeIDImplData::RegistrationValidationResult &e) {
        CorbaConversion::raise_REGISTRATION_VALIDATION_ERROR(e);
        throw IDL::INTERNAL_SERVER_ERROR();//should never be used
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

InfoContact* Server_i::info_contact(
        const char *username)
{
    try {
        MojeIDImplData::InfoContact contact;
        impl_ptr_->info_contact(username, contact);
        return CorbaConversion::wrap_InfoContact(contact)._retn();
    }
    catch (const MojeIDImplData::ObjectDoesntExist &e) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

InfoContactPublishFlags Server_i::get_contact_info_publish_flags(
    ContactId contact_id)
{
    try {
        MojeIDImplData::InfoContactPublishFlags flags;
        impl_ptr_->get_contact_info_publish_flags(contact_id, flags);
        InfoContactPublishFlags retval;
        CorbaConversion::wrap_InfoContactPublishFlags(flags, retval);
        return retval;
    }
    catch (const MojeIDImplData::ObjectDoesntExist &e) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

ContactId Server_i::process_registration_request(
        const char *ident_request_id,
        const char *password,
        LogRequestId log_request_id)
{
    try {
        return impl_ptr_->process_registration_request(ident_request_id, password, log_request_id);
    }
    catch (const MojeIDImplData::PublicRequestDoesntExist&) {
        throw IDL::IDENTIFICATION_REQUEST_NOT_EXISTS();
    }
    catch (const MojeIDImplData::IdentificationFailed&) {
        throw IDL::IDENTIFICATION_FAILED();
    }
    catch (const MojeIDImplData::IdentificationAlreadyProcessed&) {
        throw IDL::IDENTIFICATION_ALREADY_PROCESSED();
    }
    catch (const MojeIDImplData::IdentificationAlreadyInvalidated&) {
        throw IDL::IDENTIFICATION_ALREADY_INVALIDATED();
    }
    catch (const MojeIDImplData::ContactChanged&) {
        throw IDL::OBJECT_CHANGED();
    }
    catch (const MojeIDImplData::AlreadyMojeidContact&) {
        throw IDL::ALREADY_MOJEID_CONTACT();
    }
    catch (const MojeIDImplData::ObjectAdminBlocked&) {
        throw IDL::OBJECT_ADMIN_BLOCKED();
    }
    catch (const MojeIDImplData::ObjectUserBlocked&) {
        throw IDL::OBJECT_USER_BLOCKED();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//process_registration_request

void Server_i::process_identification_request(
        ContactId contact_id,
        const char *password,
        LogRequestId log_request_id)
{
    try {
        impl_ptr_->process_identification_request(contact_id, password, log_request_id);
    }
    catch (const MojeIDImplData::PublicRequestDoesntExist&) {
        throw IDL::IDENTIFICATION_REQUEST_NOT_EXISTS();
    }
    catch (const MojeIDImplData::IdentificationFailed&) {
        throw IDL::IDENTIFICATION_FAILED();
    }
    catch (const MojeIDImplData::IdentificationAlreadyProcessed&) {
        throw IDL::IDENTIFICATION_ALREADY_PROCESSED();
    }
    catch (const MojeIDImplData::UpdateContactPrepareValidationResult &e) {
        CorbaConversion::raise_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR(e);
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

Buffer* Server_i::get_validation_pdf(
        ContactId _contact_id)
{
    try {
        const MojeIDImplData::Buffer pdf_content = impl_ptr_->get_validation_pdf(_contact_id);
        return CorbaConversion::wrap_Buffer(pdf_content)._retn();
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_validation_pdf

void Server_i::create_validation_request(
        ContactId _contact_id,
        LogRequestId _log_request_id)
{
    try {
        impl_ptr_->create_validation_request(_contact_id, _log_request_id);
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (const MojeIDImplData::ValidationRequestExists&) {
        throw IDL::OBJECT_EXISTS();
    }
    catch (const MojeIDImplData::ValidationAlreadyProcessed&) {
        throw IDL::VALIDATION_ALREADY_PROCESSED();
    }
    catch (const MojeIDImplData::CreateValidationRequestValidationResult &e) {
        CorbaConversion::raise_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR(e);
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//create_validation_request

ContactStateInfoList* Server_i::get_contacts_state_changes(
        ::CORBA::ULong _last_hours)
{
    try {
        MojeIDImplData::ContactStateInfoList contacts_state_changes;
        impl_ptr_->get_contacts_state_changes(_last_hours, contacts_state_changes);
        return CorbaConversion::wrap_ContactStateInfoList(contacts_state_changes)._retn();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_contacts_state_changes

ContactStateInfo* Server_i::get_contact_state(
        ContactId _contact_id)
{
    try {
        MojeIDImplData::ContactStateInfo data;
        impl_ptr_->get_contact_state(_contact_id, data);
        return CorbaConversion::wrap_ContactStateInfo(data)._retn();
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_contact_state

void Server_i::cancel_account_prepare(
        ContactId _contact_id,
        const char *_trans_id,
        LogRequestId _log_request_id)
{
    try {
        const std::string trans_id = _trans_id;
        impl_ptr_->cancel_account_prepare(_contact_id, trans_id, _log_request_id);
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//cancel_account_prepare

ContactHandleList* Server_i::get_unregistrable_handles()
{
    try {
        MojeIDImplData::ContactHandleList unregistrable_handles;
        impl_ptr_->get_unregistrable_contact_handles(unregistrable_handles);
        return CorbaConversion::wrap_ContactHandleList(unregistrable_handles)._retn();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//get_unregistrable_handles

void Server_i::send_new_pin3(
      ContactId contact_id,
      LogRequestId log_request_id)
{
    try {
        impl_ptr_->send_new_pin3(contact_id, log_request_id);
    }
    catch (const MojeIDImplData::MessageLimitExceeded &e) {
        CorbaConversion::raise_MESSAGE_LIMIT_EXCEEDED(e);
    }
    catch (const MojeIDImplData::PublicRequestDoesntExist&) {
        throw IDL::IDENTIFICATION_REQUEST_NOT_EXISTS();
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        throw IDL::OBJECT_NOT_EXISTS();
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}

void Server_i::send_mojeid_card(
      ContactId contact_id,
      LogRequestId log_request_id)
{
    try {
        impl_ptr_->send_mojeid_card(contact_id, log_request_id);
    }
    catch (const MojeIDImplData::MessageLimitExceeded &e) {
        CorbaConversion::raise_MESSAGE_LIMIT_EXCEEDED(e);
    }
    catch (const MojeIDImplData::PublicRequestDoesntExist&) {
        throw IDL::IDENTIFICATION_REQUEST_NOT_EXISTS();
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
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

}//namespace Registry::MojeID
}//namespace Registry
