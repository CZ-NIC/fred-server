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
 *  header of mojeid2 implementation
 */

#ifndef MOJEID2_H_06D795C17DD0FF3D98B375032F99493A//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID2_H_06D795C17DD0FF3D98B375032F99493A

#include "src/mojeid/mojeid2_checkers.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/messages/messages_impl.h"

#include <vector>
#include <stdexcept>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace Registry {
namespace MojeID {

typedef std::vector< std::string > HandleList;

class ContactStateData
{
public:
    typedef unsigned long long          ContactId;
    typedef boost::posix_time::ptime    DateTime;
    typedef Fred::Object::State::Value  State;
    typedef std::map< State, DateTime > StateValidFrom;

    ContactStateData():contact_id_(INVALID_CONTACT_ID) { }
    ContactStateData(ContactId _contact_id):contact_id_(_contact_id) { }

    ContactId get_contact_id()const { return contact_id_; }
    bool presents(State _state)const { return 0 < validity_.count(_state); }
    DateTime get_validity(State _state)const
    {
        StateValidFrom::const_iterator iter = validity_.find(_state);
        if (iter != validity_.end()) {
            return iter->second;
        }
        throw std::runtime_error("state not found");
    }
    StateValidFrom::const_iterator get_sum_state()const;

    ContactStateData& set_contact_id(ContactId _value)
    {
        contact_id_ = _value;
        return *this;
    }
    ContactStateData& set_validity(State _state, const DateTime &_valid_from)
    {
        validity_.insert(std::make_pair(_state, _valid_from));
        return *this;
    }
    ContactStateData& clear()
    {
        validity_.clear();
        return this->set_contact_id(INVALID_CONTACT_ID);
    }
private:
    enum { INVALID_CONTACT_ID = 0 };
    ContactId      contact_id_;
    StateValidFrom validity_;
};

typedef std::vector< ContactStateData > ContactStateDataList;

class MojeID2Impl
{
public:
    typedef unsigned long long ContactId;
    typedef unsigned long long MessageId;
    typedef unsigned long long LogRequestId;
    MojeID2Impl(const std::string &_server_name);
    ~MojeID2Impl();

    const std::string& get_server_name()const;

    HandleList& get_unregistrable_contact_handles(
        HandleList &_result)const;

    ContactId create_contact_prepare(
        const Fred::InfoContactData &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)const;

    Fred::InfoContactData& transfer_contact_prepare(
        const std::string &_handle,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        Fred::InfoContactData &_contact,
        std::string &_ident)const;

    void update_contact_prepare(
        const Fred::InfoContactData &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const;

    void update_transfer_contact_prepare(
        const std::string &_username,
        Fred::InfoContactData &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const;

    Fred::InfoContactData& info_contact(
        const std::string &_username,
        Fred::InfoContactData &_result)const;

    void commit_prepared_transaction(const std::string &_trans_id)const;

    void rollback_prepared_transaction(const std::string &_trans_id)const;

    std::string get_validation_pdf(ContactId _contact_id)const;

    void create_validation_request(
        ContactId contact_id,
        LogRequestId log_request_id)const;

    typedef Fred::Object::Get< Fred::Object::Type::CONTACT > GetContact;

    typedef boost::mpl::list< Fred::check_contact_name,
                              Fred::MojeID::check_contact_birthday,
                              Fred::check_contact_email_presence,
                              Fred::check_contact_email_validity,
                              Fred::check_contact_notifyemail_validity,
                              Fred::check_contact_phone_presence,
                              Fred::check_contact_phone_validity,
                              Fred::check_contact_fax_validity,
                              Fred::check_contact_place_address,
                              Fred::check_contact_addresses_mailing,
                              Fred::check_contact_addresses_billing,
                              Fred::check_contact_addresses_shipping,
                              Fred::check_contact_addresses_shipping2,
                              Fred::check_contact_addresses_shipping3 > check_mojeid_registration;

    typedef boost::mpl::list< Fred::MojeID::check_contact_username_availability,
                              Fred::check_contact_email_availability,
                              Fred::check_contact_phone_availability > check_mojeid_registration_ctx;

    typedef boost::mpl::list< Fred::MojeID::Check::states_before_transfer_into_mojeid > check_transfer_contact_prepare_presence;

    typedef Fred::Check< boost::mpl::list< check_mojeid_registration,
                                        check_mojeid_registration_ctx,
                                        check_transfer_contact_prepare_presence > > CheckMojeIDRegistration;

    typedef Fred::Check< boost::mpl::list< check_mojeid_registration,
                                        check_mojeid_registration_ctx > > CheckCreateContactPrepare;


    typedef boost::mpl::list< Fred::check_contact_name,
                              Fred::check_contact_place_address_mandatory,
                              Fred::check_contact_addresses_mailing,
                              Fred::check_contact_addresses_billing,
                              Fred::check_contact_addresses_shipping,
                              Fred::check_contact_addresses_shipping2,
                              Fred::check_contact_addresses_shipping3,
                              Fred::check_contact_email_presence,
                              Fred::check_contact_email_validity,
                              Fred::check_contact_phone_presence,
                              Fred::check_contact_phone_validity > check_create_contact_prepare;

    typedef boost::mpl::list< Fred::MojeID::check_contact_username_availability,
                              Fred::check_contact_email_availability,
                              Fred::check_contact_phone_availability > check_create_contact_prepare_ctx;

    typedef boost::mpl::list< Fred::check_contact_email_availability,
                              Fred::check_contact_phone_availability > check_process_registration_request_ctx;

    typedef Fred::Check< boost::mpl::list< check_create_contact_prepare,
                    check_process_registration_request_ctx > > CheckProcessRegistrationRequest;



    typedef boost::mpl::list< Fred::check_contact_name,
                              Fred::MojeID::check_contact_birthday,
                              Fred::check_contact_email_presence,
                              Fred::check_contact_email_validity,
                              Fred::check_contact_notifyemail_validity,
                              Fred::check_contact_phone_presence,
                              Fred::check_contact_phone_validity,
                              Fred::check_contact_fax_validity,
                              Fred::check_contact_place_address,
                              Fred::check_contact_addresses_mailing,
                              Fred::check_contact_addresses_billing,
                              Fred::check_contact_addresses_shipping,
                              Fred::check_contact_addresses_shipping2,
                              Fred::check_contact_addresses_shipping3 > check_update_contact_prepare;

    typedef Fred::Check< check_update_contact_prepare > CheckUpdateContactPrepare;

    typedef CheckUpdateContactPrepare UpdateContactPrepareError;

    typedef boost::mpl::list< Fred::check_contact_name,
                              Fred::check_contact_place_address,
                              Fred::check_contact_addresses_mailing,
                              Fred::check_contact_email_presence,
                              Fred::check_contact_email_validity,
                              Fred::check_contact_phone_validity,
                              Fred::check_contact_notifyemail_validity,
                              Fred::check_contact_fax_validity > check_update_transfer;
    typedef Fred::Check< check_update_transfer > CheckUpdateTransfer;
    typedef CheckUpdateTransfer UpdateTransferError;

    typedef check_update_contact_prepare check_process_identification_request;
    typedef Fred::Check< check_process_identification_request > CheckProcessIdentificationRequest;
    typedef CheckProcessIdentificationRequest ProcessIdentificationRequestError;

    typedef boost::mpl::list< Fred::check_contact_name,
                              Fred::check_contact_place_address_mandatory,
                              Fred::check_contact_email_presence,
                              Fred::check_contact_email_validity,
                              Fred::check_contact_phone_validity,
                              Fred::check_contact_notifyemail_validity,
                              Fred::check_contact_fax_validity,
                              Fred::MojeID::check_contact_ssn > check_create_validation_request;
    typedef Fred::Check< check_create_validation_request > CheckCreateValidationRequest;
    typedef CheckCreateValidationRequest CreateValidationRequestError;

    class IdentificationFailed:public std::runtime_error
    {
    public:
        IdentificationFailed(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class IdentificationAlreadyProcessed:public std::runtime_error
    {
    public:
        IdentificationAlreadyProcessed(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class IdentificationAlreadyInvalidated:public std::runtime_error
    {
    public:
        IdentificationAlreadyInvalidated(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class ContactChanged:public std::runtime_error
    {
    public:
        ContactChanged(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class PublicRequestDoesntExist:public std::runtime_error
    {
    public:
        PublicRequestDoesntExist(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class ObjectAdminBlocked:public std::runtime_error
    {
    public:
        ObjectAdminBlocked(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class ObjectUserBlocked:public std::runtime_error
    {
    public:
        ObjectUserBlocked(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class AlreadyMojeidContact:public std::runtime_error
    {
    public:
        AlreadyMojeidContact(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class ObjectDoesntExist:public std::runtime_error
    {
    public:
        ObjectDoesntExist(const std::string &_msg = ""):std::runtime_error(_msg) { }
    };

    class IdentificationRequestDoesntExist:public std::runtime_error
    {
    public:
        IdentificationRequestDoesntExist(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class ValidationRequestExists:public std::runtime_error
    {
    public:
        ValidationRequestExists(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class ValidationAlreadyProcessed:public std::runtime_error
    {
    public:
        ValidationAlreadyProcessed(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class MessageLimitExceeded:public std::runtime_error
    {
    public:
        typedef boost::gregorian::date Date;
        MessageLimitExceeded(const Date &_limit_expire_date,
                             unsigned _limit_count,
                             unsigned _limit_days)
        :   std::runtime_error("no letter can be sent"),
            limit_expire_date(_limit_expire_date),
            limit_count(_limit_count),
            limit_days(_limit_days)
        { }
        const Date limit_expire_date;///< When a new message can be sent
        const unsigned limit_count;  ///< At most a @ref limit_count messages can be sent in a @ref limit_days days
        const unsigned limit_days;   ///< @see limit_count
    };

    ContactId process_registration_request(
        const std::string &_ident_request_id,
        const std::string &_password,
        LogRequestId _log_request_id)const;

    void process_identification_request(
        ContactId _contact_id,
        const std::string &_password,
        LogRequestId _log_request_id)const;

    ContactStateDataList& get_contacts_state_changes(
        unsigned long _last_hours,
        ContactStateDataList &_result)const;

    ContactStateData& get_contact_state(
        ContactId _contact_id,
        ContactStateData &_result)const;

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
};//class MojeID2Impl

}//namespace Registry::MojeID
}//namespace Registry

#endif // MOJEID2_H_06D795C17DD0FF3D98B375032F99493A
