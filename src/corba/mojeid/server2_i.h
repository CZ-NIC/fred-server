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
*  header of mojeid2 corba wrapper
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/MojeID2.idl
*/
#ifndef SERVER2_I_H_4A7A4BDA1B4C87979D8B1FBE04FD1583//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define SERVER2_I_H_4A7A4BDA1B4C87979D8B1FBE04FD1583

#include "src/corba/MojeID2.hh"
#include "src/corba/mojeid/service_name.h"
#include <memory>
#include <string>
#include <boost/utility.hpp>

namespace Registry {
namespace MojeID {

class MojeID2Impl;//backend implementation class

///mojeid2 corba interface
class Server_i: private boost::noncopyable,
                public POA_Registry::MojeID::Server
{
public:
    // standard constructor
    Server_i(const std::string &_server_name);
    virtual ~Server_i();
    // methods corresponding to defined IDL attributes and operations
    ContactId create_contact_prepare(
        const CreateContact &c,
        const char *trans_id,
        LogRequestId log_request_id,
        ::CORBA::String_out ident);

    InfoContact* transfer_contact_prepare(
        const char *handle,
        const char *trans_id,
        LogRequestId log_request_id,
        ::CORBA::String_out ident);

    void update_contact_prepare(
        const UpdateContact &c,
        const char *trans_id,
        LogRequestId log_request_id);

    InfoContact* update_transfer_contact_prepare(
        const char *username,
        const SetContact& contact_data,
        const char *trans_id,
        LogRequestId request_id);

    InfoContact* info_contact(
        const char *username);

    ContactId process_registration_request(
        const char *ident_request_id,
        const char *password,
        LogRequestId log_request_id);

    void process_identification_request(
        ContactId contact_id,
        const char *password,
        LogRequestId log_request_id);

    void commit_prepared_transaction(
        const char *trans_id);

    void rollback_prepared_transaction(
        const char *trans_id);

    Buffer* get_validation_pdf(
        ContactId contact_id);

    void create_validation_request(
        ContactId contact_id,
        LogRequestId log_request_id);

    ContactStateInfoList* get_contacts_state_changes(
        ::CORBA::ULong last_hours);

    ContactStateInfo* get_contact_state(
        ContactId contact_id);

    void cancel_account_prepare(
        ContactId contact_id,
        const char *trans_id,
        LogRequestId log_request_id);

    ContactHandleList* get_unregistrable_handles();

    void send_new_pin3(
        ContactId contact_id,
        LogRequestId log_request_id);

    void send_mojeid_card(
        ContactId contact_id,
        LogRequestId log_request_id);

    void generate_sms_messages();
    void enable_sms_messages_generation(::CORBA::Boolean enable);

    void generate_email_messages();
    void enable_email_messages_generation(::CORBA::Boolean enable);

    void generate_letter_messages();
    void enable_letter_messages_generation(::CORBA::Boolean enable);
private:
    const std::auto_ptr< MojeID2Impl > impl_ptr_;
};//class Server_i

}//namespace Registry::MojeID
}//namespace Registry

#endif //SERVER2_I_H_4A7A4BDA1B4C87979D8B1FBE04FD1583
