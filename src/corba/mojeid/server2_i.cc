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
 *  implementational code for mojeid2 IDL interface
 *  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/MojeID2.idl
 */

#include "src/corba/mojeid/server2_i.h"
#include "src/corba/mojeid/corba_conversion2.h"
#include "src/mojeid/mojeid2.h"

#include <memory>

namespace Registry {
namespace MojeID {

typedef Server IDL;

Server_i::Server_i(const std::string &_server_name)
:   impl_ptr_(new MojeID2Impl(_server_name))
{
}

Server_i::~Server_i()
{
}

//   Methods corresponding to IDL attributes and operations
::CORBA::ULongLong Server_i::create_contact_prepare(
    const CreateContact &_contact,
    const char *_trans_id,
    ::CORBA::ULongLong _log_request_id,
    ::CORBA::String_out _identification)
{
    try {
        Fred::InfoContactData contact;
        Corba::Conversion::from(_contact).into(contact);
        std::string ident;
        const ContactId contact_id = impl_ptr_->create_contact_prepare(
            contact,
            _trans_id,
            _log_request_id,
            ident);
        Corba::Conversion::into(_identification).from(ident);
        return contact_id;
    }
    catch (const MojeID2Impl::CreateContactPrepareError &e) {
        IDL::CREATE_CONTACT_PREPARE_VALIDATION_ERROR idl_error;
        Corba::Conversion::into(idl_error).from(e);
        throw idl_error;
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//create_contact_prepare

Registry::MojeID::InfoContact* Server_i::transfer_contact_prepare(
        const char *_handle,
        const char *_trans_id,
        ::CORBA::ULongLong _log_request_id,
        ::CORBA::String_out _identification)
{
    try {
        Fred::InfoContactData contact;
        std::string ident;
        impl_ptr_->transfer_contact_prepare(
            _handle,
            _trans_id,
            _log_request_id,
            contact,
            ident);
        Corba::Conversion::into(_identification).from(ident);
        std::auto_ptr< Registry::MojeID::InfoContact > contact_info_ptr(new Registry::MojeID::InfoContact);
        Corba::Conversion::into(*contact_info_ptr).from(contact);
        return contact_info_ptr.release();
    }
    catch (const MojeID2Impl::CreateContactPrepareError &e) {
        IDL::CREATE_CONTACT_PREPARE_VALIDATION_ERROR idl_error;
        Corba::Conversion::into(idl_error).from(e);
        throw idl_error;
    }
    catch (...) {
        throw IDL::INTERNAL_SERVER_ERROR();
    }
}//transfer_contact_prepare

void Server_i::update_contact_prepare(
        const UpdateContact &_contact,
        const char *_trans_id,
        ::CORBA::ULongLong _log_request_id)
{
}//update_contact_prepared

::CORBA::ULongLong Server_i::process_registration_request(
        const char *ident_request_id,
        const char *password,
        ::CORBA::ULongLong log_request_id)
{
    return 0;
}//process_registration_request

void Server_i::process_identification_request(
        ::CORBA::ULongLong contact_id,
        const char *password,
        ::CORBA::ULongLong log_request_id)
{
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
        ::CORBA::ULongLong _contact_id)
{
    return NULL;
}//get_validation_pdf

void Server_i::create_validation_request(
        ::CORBA::ULongLong _contact_id,
        ::CORBA::ULongLong _log_request_id)
{
}//create_validation_request

ContactStateInfoList* Server_i::get_contacts_state_changes(
        ::CORBA::ULong _last_hours)
{
    return NULL;
}//get_contacts_state_changes

ContactStateInfo* Server_i::get_contact_state(
        ::CORBA::ULongLong _contact_id)
{
    return NULL;
}//get_contact_state

void Server_i::cancel_contact_account_prepare(
        ::CORBA::ULongLong _contact_id,
        const char *_trans_id,
        ::CORBA::ULongLong _log_request_id)
{
}//cancel_contact_account_prepare

ContactHandleList* Server_i::get_unregistrable_handles(
        ::CORBA::ULong _chunk_size,
        ::CORBA::ULongLong &_start_from)
{
    try {
        HandleList chunk;
        ContactId last_contact_id = _start_from;
        impl_ptr_->get_unregistrable_contact_handles(_chunk_size, last_contact_id, chunk);
        ContactHandleList_var ret = new ContactHandleList;
        ret->length(0);

        for (::size_t idx = 0; idx < chunk.size(); ++idx) {
            ret->length(idx + 1);
            ret[idx] = chunk[idx].c_str();
        }

        _start_from = last_contact_id;
        return ret._retn();
    }
    catch (...) {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}//get_unregistrable_handles

void Server_i::send_new_pin3(
      ::CORBA::ULongLong contact_id,
      ::CORBA::ULongLong log_request_id)
{
}

void Server_i::send_mojeid_card(
      ::CORBA::ULongLong contact_id,
      ::CORBA::ULongLong log_request_id)
{
}

::CORBA::ULongLong Server_i::get_contact_id(
        const char *_handle)
{
    return 0;
}//get_contact_id

char* Server_i::get_contact_authinfo(
        ::CORBA::ULongLong contact_id)
{
    return NULL;
}

void Server_i::corba_conversion_test(const Test &ct, Test &t, ::CORBA::String_out so, ::CORBA::String_var sv,
                                     ::CORBA::String_member sm)
{
    const char *cs;
    std::string s;
    Nullable< std::string > ns;

    ns = Corba::Conversion::from(ct.var).into< Nullable< std::string > >();
    ns = Corba::Conversion::from(ct.var).into< std::string >();
    ns = Corba::Conversion::from(ct.var).into(ns);
    cs = Corba::Conversion::from(ct.var).into(cs);
    s  = Corba::Conversion::from(ct.var).into(s);
    cs = Corba::Conversion::from(ct.var).into(cs, "NULL");
    s  = Corba::Conversion::from(ct.var).into(s, "NULL");
    cs = Corba::Conversion::from(ct.var).into(cs, "NULL");
    s  = Corba::Conversion::from(ct.var).into(s, "NULL");
    Corba::Conversion::into(t.var).from(cs);
    Corba::Conversion::into(t.var).from(s);
    Corba::Conversion::into(t.var).from(ns);

    ns = Corba::Conversion::from(ct.member).into< std::string >();
    ns = Corba::Conversion::from(ct.member).into(ns);
    cs = Corba::Conversion::from(ct.member).into(cs);
    s  = Corba::Conversion::from(ct.member).into(s);
    cs = Corba::Conversion::from(ct.member).into(cs, "NULL");
    s  = Corba::Conversion::from(ct.member).into(s, "NULL");
    Corba::Conversion::into(t.member).from(cs);
    Corba::Conversion::into(t.member).from(s);
    Corba::Conversion::into(t.member).from(ns);

    Corba::Conversion::into(so).from(cs);
    Corba::Conversion::into(so).from(s);

    cs = Corba::Conversion::from(sv).into< const char* >();
    s  = Corba::Conversion::from(sv).into< std::string >();
    cs = Corba::Conversion::from(sv).into(cs);
    s  = Corba::Conversion::from(sv).into(s);
    Corba::Conversion::into(sv).from(cs);
    Corba::Conversion::into(sv).from(s);

    cs = Corba::Conversion::from(sm).into< const char* >();
    s  = Corba::Conversion::from(sm).into< std::string >();
    cs = Corba::Conversion::from(sm).into(cs);
    s  = Corba::Conversion::from(sm).into(s);
    Corba::Conversion::into(sm).from(cs);
    Corba::Conversion::into(sm).from(s);

    DateTime dt;
    boost::posix_time::ptime pt = Corba::Conversion::from(dt).into< boost::posix_time::ptime >();
    pt = Corba::Conversion::from(dt).into(pt);
    pt = Corba::Conversion::from(dt).into(pt, boost::posix_time::ptime());
    Corba::Conversion::into(dt).from(pt);
}

}//namespace Registry::MojeID
}//namespace Registry
