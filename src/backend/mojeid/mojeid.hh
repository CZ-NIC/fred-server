/*
 * Copyright (C) 2015-2020  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  header of mojeid implementation
 */

#ifndef MOJEID_HH_56C8449065CF456E85D3F2D0A1B5A37A
#define MOJEID_HH_56C8449065CF456E85D3F2D0A1B5A37A

#include "src/backend/buffer.hh"
#include "src/backend/mojeid/mojeid_checkers.hh"
#include "src/backend/mojeid/mojeid_impl_data_conversion.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "libfred/object/object_state.hh"

#include <stdexcept>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Fred {
namespace Backend {
namespace MojeId {

class ConfiguredRegistrar
{
public:
    explicit ConfiguredRegistrar(const std::string& _handle);
    std::string handle() const;
    unsigned long long id() const;

private:
    std::string handle_;
    unsigned long long id_;
};

class MojeIdImpl
{
public:
    typedef unsigned long long ContactId;
    typedef unsigned long long MessageId;
    typedef unsigned long long LogRequestId;
    MojeIdImpl(const std::string& _server_name);
    ~MojeIdImpl();

    const std::string& get_server_name() const;

    void get_unregistrable_contact_handles(
            MojeIdImplData::ContactHandleList& _result) const;

    ContactId create_contact_prepare(
            const MojeIdImplData::CreateContact& _contact,
            const std::string& _trans_id,
            LogRequestId _log_request_id,
            std::string& _ident) const;

    void transfer_contact_prepare(
            const std::string& _handle,
            const std::string& _trans_id,
            LogRequestId _log_request_id,
            MojeIdImplData::InfoContact& _contact,
            std::string& _ident) const;

    void update_contact_prepare(
            ContactId contact_id,
            const MojeIdImplData::UpdateContact& _new_data,
            const std::string& _trans_id,
            LogRequestId _log_request_id) const;

    void update_validated_contact_prepare(
            ContactId contact_id,
            const MojeIdImplData::ValidatedContactData& verified_data,
            const std::string& _trans_id,
            LogRequestId _log_request_id) const;

    MojeIdImplData::InfoContact update_transfer_contact_prepare(
            const std::string& _username,
            const MojeIdImplData::UpdateTransferContact& _new_data,
            const std::string& _trans_id,
            LogRequestId _log_request_id,
            std::string& _ident) const;

    void info_contact(
            const std::string& _username,
            MojeIdImplData::InfoContact& _result) const;

    void get_contact_info_publish_flags(
            ContactId _contact_id,
            MojeIdImplData::InfoContactPublishFlags& _flags) const;

    void commit_prepared_transaction(const std::string& _trans_id) const;

    void rollback_prepared_transaction(const std::string& _trans_id) const;

    Fred::Backend::Buffer get_validation_pdf(ContactId _contact_id) const;

    void create_validation_request(
            ContactId contact_id,
            LogRequestId log_request_id) const;

    void validate_contact(
            ContactId contact_id,
            LogRequestId log_request_id) const;

    ContactId process_registration_request(
            const std::string& _ident_request_id,
            const std::string& _password,
            LogRequestId _log_request_id) const;

    void process_identification_request(
            ContactId _contact_id,
            const std::string& _password,
            LogRequestId _log_request_id) const;

    void get_contacts_state_changes(
            unsigned long _last_hours,
            MojeIdImplData::ContactStateInfoList& _result) const;

    void get_contact_state(
            ContactId _contact_id,
            MojeIdImplData::ContactStateInfo& _result) const;

    void cancel_account_prepare(
            ContactId _contact_id,
            const std::string& _trans_id,
            LogRequestId _log_request_id) const;

    void send_new_pin3(
            ContactId _contact_id,
            LogRequestId _log_request_id) const;

    void send_mojeid_card(
            ContactId _contact_id,
            LogRequestId _log_request_id) const;

    void generate_sms_messages() const;
    void enable_sms_messages_generation(bool enable) const;

    void generate_letter_messages() const;
    void enable_letter_messages_generation(bool enable) const;

    void generate_email_messages() const;
    void enable_email_messages_generation(bool enable) const;

    static MessageId send_mojeid_card(
            LibFred::OperationContext& _ctx,
            LibFred::Messages::Manager* _msg_manager_ptr,
            const LibFred::InfoContactData& _data,
            unsigned _limit_count,
            unsigned _limit_interval,
            LogRequestId _log_request_id,
            const Optional<boost::posix_time::ptime>& _letter_time = Optional<boost::posix_time::ptime>(),
            const Optional<bool>& _validated_contact = Optional<bool>());

private:
    const std::string server_name_;
    const ConfiguredRegistrar mojeid_registrar_;
    const ConfiguredRegistrar system_registrar_;
};

} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

#endif
