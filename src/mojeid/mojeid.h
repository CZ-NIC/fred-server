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
 *  header of mojeid implementation
 */

#ifndef MOJEID_H_06D795C17DD0FF3D98B375032F99493A//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID_H_06D795C17DD0FF3D98B375032F99493A

#include "src/mojeid/mojeid_checkers.h"
#include "src/mojeid/mojeid_impl_data_conversion.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/messages/messages_impl.h"

#include <vector>
#include <stdexcept>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace Registry {
namespace MojeID {

class MojeIDImpl
{
public:
    typedef unsigned long long ContactId;
    typedef unsigned long long MessageId;
    typedef unsigned long long LogRequestId;
    MojeIDImpl(const std::string &_server_name);
    ~MojeIDImpl();

    const std::string& get_server_name()const;

    void get_unregistrable_contact_handles(
        MojeIDImplData::ContactHandleList &_result)const;

    ContactId create_contact_prepare(
        const MojeIDImplData::CreateContact &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)const;

    void transfer_contact_prepare(
        const std::string &_handle,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        MojeIDImplData::InfoContact &_contact,
        std::string &_ident)const;

    void update_contact_prepare(
        const MojeIDImplData::InfoContact &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const;

    MojeIDImplData::InfoContact update_transfer_contact_prepare(
        const std::string &_username,
        const MojeIDImplData::UpdateTransferContact &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const;

    void info_contact(
        const std::string &_username,
        MojeIDImplData::InfoContact &_result)const;

    void commit_prepared_transaction(const std::string &_trans_id)const;

    void rollback_prepared_transaction(const std::string &_trans_id)const;

    MojeIDImplData::Buffer get_validation_pdf(ContactId _contact_id)const;

    void create_validation_request(
        ContactId contact_id,
        LogRequestId log_request_id)const;

    ContactId process_registration_request(
        const std::string &_ident_request_id,
        const std::string &_password,
        LogRequestId _log_request_id)const;

    void process_identification_request(
        ContactId _contact_id,
        const std::string &_password,
        LogRequestId _log_request_id)const;

    void get_contacts_state_changes(
        unsigned long _last_hours,
        MojeIDImplData::ContactStateInfoList &_result)const;

    void get_contact_state(
        ContactId _contact_id,
        MojeIDImplData::ContactStateInfo &_result)const;

    void cancel_account_prepare(
        ContactId _contact_id,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const;

    void send_new_pin3(
        ContactId _contact_id,
        LogRequestId _log_request_id)const;

    void send_mojeid_card(
        ContactId _contact_id,
        LogRequestId _log_request_id)const;

    void generate_sms_messages()const;
    void enable_sms_messages_generation(bool enable)const;

    void generate_letter_messages()const;
    void enable_letter_messages_generation(bool enable)const;

    void generate_email_messages()const;
    void enable_email_messages_generation(bool enable)const;

    static MessageId send_mojeid_card(
        Fred::OperationContext &_ctx,
        Fred::Messages::Manager *_msg_manager_ptr,
        const Fred::InfoContactData &_data,
        unsigned _limit_count,
        unsigned _limit_interval,
        LogRequestId _log_request_id,
        const Optional< boost::posix_time::ptime > &_letter_time = Optional< boost::posix_time::ptime >(),
        const Optional< bool > &_validated_contact = Optional< bool >());
private:
    const std::string server_name_;
    const std::string mojeid_registrar_handle_;
    const ::size_t mojeid_registrar_id_;
};//class MojeIDImpl

}//namespace Registry::MojeID
}//namespace Registry

#endif//MOJEID_H_06D795C17DD0FF3D98B375032F99493A
