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
 *  mojeid implementation
 */

#include "src/mojeid/mojeid.h"
#include "src/mojeid/mojeid_impl_internal.h"
#include "src/mojeid/safe_data_storage.h"
#include "src/mojeid/mojeid_public_request.h"
#include "src/mojeid/messages/generate.h"
#include "src/corba/mojeid/mojeid_corba_conversion.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/transfer_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/info_contact_diff.h"
#include "src/fredlib/contact/ssntype.h"
#include "src/fredlib/documents.h"
#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/public_request/create_public_request_auth.h"
#include "src/fredlib/public_request/info_public_request_auth.h"
#include "src/fredlib/public_request/update_public_request.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/public_request/get_active_public_request.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/notifier/enqueue_notification.h"
#include "src/fredlib/poll/create_transfer_contact_poll_message.h"
#include "util/random.h"
#include "util/xmlgen.h"
#include "util/log/context.h"
#include "util/cfg/handle_corbanameservice_args.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/cfg/handle_registry_args.h"
#include "util/cfg/config_handler_decl.h"
#include "util/types/birthdate.h"

#include <algorithm>
#include <map>
#include <iomanip>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/set.hpp>

namespace Registry {
namespace MojeID {

namespace {

std::string create_ctx_name(const std::string &_name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
}

std::string create_ctx_function_name(const char *fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const MojeIDImpl &_impl, const std::string &_op_name)
    :   ctx_server_(create_ctx_name(_impl.get_server_name())),
        ctx_operation_(_op_name)
    {
    }
private:
    Logging::Context ctx_server_;
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) LogContext CTX_VAR((IMPL_OBJ), create_ctx_function_name(__FUNCTION__))

std::string get_mojeid_registrar_handle()
{
    const std::string handle =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->registrar_handle;
    if (!handle.empty()) {
        return handle;
    }
    throw std::runtime_error("missing configuration for dedicated registrar");
}

::size_t get_mojeid_registrar_id(const std::string &registrar_handle)
{
    Fred::OperationContextCreator ctx;
    Database::Result dbres = ctx.get_conn().exec_params(
        "SELECT id FROM registrar WHERE handle=$1::TEXT", Database::query_param_list(registrar_handle));
    if (0 < dbres.size()) {
        ctx.commit_transaction();
        return static_cast< ::size_t >(dbres[0][0]);
    }
    throw std::runtime_error("missing dedicated registrar");
}

void set_create_contact_arguments(
    const MojeIDImplData::CreateContact &_contact,
    Fred::CreateContact &_arguments)
{
    _arguments.set_name(_contact.first_name + " " + _contact.last_name);
    Fred::Contact::PlaceAddress permanent;
    from_into(_contact.permanent, permanent);
    _arguments.set_place(permanent);
    _arguments.set_email(_contact.email);
    _arguments.set_telephone(_contact.telephone);
    if (!_contact.organization.isnull()) {
        _arguments.set_organization(_contact.organization.get_value());
    }
    if (!_contact.notify_email.isnull()) {
        _arguments.set_notifyemail(_contact.notify_email.get_value());
    }
    if (!_contact.fax.isnull()) {
        _arguments.set_fax(_contact.fax.get_value());
    }
    {
        Fred::ContactAddressList addresses;
        if (!_contact.mailing.isnull()) {
            from_into(_contact.mailing.get_value(), addresses[Fred::ContactAddressType::MAILING]);
        }
        if (!_contact.billing.isnull()) {
            from_into(_contact.billing.get_value(), addresses[Fred::ContactAddressType::BILLING]);
        }
        if (!_contact.shipping.isnull()) {
            from_into(_contact.shipping.get_value(), addresses[Fred::ContactAddressType::SHIPPING]);
        }
        if (!_contact.shipping2.isnull()) {
            from_into(_contact.shipping2.get_value(), addresses[Fred::ContactAddressType::SHIPPING_2]);
        }
        if (!_contact.shipping3.isnull()) {
            from_into(_contact.shipping3.get_value(), addresses[Fred::ContactAddressType::SHIPPING_3]);
        }
        if (!addresses.empty()) {
            _arguments.set_addresses(addresses);
        }
    }

    if (_contact.organization.isnull()) {
        if (!_contact.birth_date.isnull()) {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(Fred::SSNType::birthday));
            _arguments.set_ssn(_contact.birth_date.get_value().value);
        }
    }
    else {
        if (!_contact.vat_id_num.isnull()) {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(Fred::SSNType::ico));
            _arguments.set_ssn(_contact.vat_id_num.get_value());
        }
        else if (!_contact.id_card_num.isnull()) {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(Fred::SSNType::op));
            _arguments.set_ssn(_contact.id_card_num.get_value());
        }
        else if (!_contact.passport_num.isnull()) {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(Fred::SSNType::pass));
            _arguments.set_ssn(_contact.passport_num.get_value());
        }
        else if (!_contact.ssn_id_num.isnull()) {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(Fred::SSNType::mpsv));
            _arguments.set_ssn(_contact.ssn_id_num.get_value());
        }
    }
}

void check_sent_letters_limit(Fred::OperationContext &_ctx,
                              MojeIDImpl::ContactId _contact_id,
                              unsigned _max_sent_letters,
                              unsigned _watched_period_in_days)
{
    const Database::Result result = _ctx.get_conn().exec_params(
        "WITH comm_type_letter AS (SELECT id FROM comm_type WHERE type='letter'),"
             "message_types AS (SELECT id FROM message_type WHERE type IN ('mojeid_pin3',"
                                                                          "'mojeid_card')),"
             "send_states_ignore AS (SELECT id FROM enum_send_status WHERE status_name='no_processing') "
        "SELECT ma.moddate+($3::TEXT||'DAYS')::INTERVAL "
        "FROM message_archive ma "
        "JOIN message_contact_history_map mc ON mc.message_archive_id=ma.id "
        "WHERE ma.message_type_id IN (SELECT id FROM message_types) AND "
              "ma.comm_type_id=(SELECT id FROM comm_type_letter) AND "
              "ma.status_id NOT IN (SELECT id FROM send_states_ignore) AND "
              "(NOW()-($3::TEXT||'DAYS')::INTERVAL)::DATE<ma.moddate::DATE AND "
              "mc.contact_object_registry_id=$1::BIGINT "
        "ORDER BY 1 DESC OFFSET ($2::INTEGER-1) LIMIT 1",
        Database::query_param_list(_contact_id)               // used as $1::BIGINT
                                  (_max_sent_letters)         // used as $2::INTEGER
                                  (_watched_period_in_days)); // used as $3::TEXT
    if (0 < result.size()) {
        MojeIDImplData::MessageLimitExceeded e;
        e.limit_expire_datetime = boost::posix_time::time_from_string(static_cast< std::string >(result[0][0]));
        throw e;
    }
}

namespace check_limits {

class sent_letters:public ::MojeID::Messages::Generate::message_checker
{
public:
    sent_letters(unsigned _max_sent_letters,
                 unsigned _watched_period_in_days)
    :   max_sent_letters_(_max_sent_letters),
        watched_period_in_days_(_watched_period_in_days)
    { }
    sent_letters(const HandleMojeIDArgs *_server_conf_ptr = CfgArgs::instance()->
                                                            get_handler_ptr_by_type< HandleMojeIDArgs >())
    :   max_sent_letters_(_server_conf_ptr->letter_limit_count),
        watched_period_in_days_(_server_conf_ptr->letter_limit_interval)
    { }
    void operator()(Fred::OperationContext &_ctx, Fred::ObjectId _object_id)const
    {
        check_sent_letters_limit(_ctx,
                                 _object_id,
                                 max_sent_letters_,
                                 watched_period_in_days_);
    }
private:
    const unsigned max_sent_letters_;
    const unsigned watched_period_in_days_;
};

}//namespace Registry::MojeID::check_limits

template < typename T >
bool differs(const Nullable< T > &a, const Nullable< T > &b)
{
    return a.get_value_or_default() != b.get_value_or_default();
}

template < typename T >
bool differs(const Optional< T > &a, const Optional< T > &b)
{
    return (a.isset() != b.isset()) ||
           (a.isset() && (a.get_value() != b.get_value()));
}

template < typename T >
bool differs(const T &a, const T &b)
{
    return a != b;
}

Nullable< boost::gregorian::date > convert_as_birthdate(const Nullable< std::string > &_birth_date)
{
    if (!_birth_date.isnull()) {
        return Nullable< boost::gregorian::date >(birthdate_from_string_to_date(_birth_date.get_value()));
    }
    return Nullable< boost::gregorian::date >();
}

bool validated_data_changed(const Fred::InfoContactData &_c1, const Fred::InfoContactData &_c2)
{
    if (differs(_c1.name, _c2.name)) {
        return true;
    }

    if (differs(_c1.organization, _c2.organization)) {
        return true;
    }

    const Fred::InfoContactData::Address a1 = _c1.get_permanent_address();
    const Fred::InfoContactData::Address a2 = _c2.get_permanent_address();
    if (differs(a1.street1,         a2.street1)         ||
        differs(a1.street2,         a2.street2)         ||
        differs(a1.street3,         a2.street3)         ||
        differs(a1.city,            a2.city)            ||
        differs(a1.stateorprovince, a2.stateorprovince) ||
        differs(a1.country,         a2.country)         ||
        differs(a1.postalcode,      a2.postalcode)) {
        return true;
    }

    if (differs(_c1.ssntype, _c2.ssntype)) {
        return true;
    }

    if (differs(_c1.ssn, _c2.ssn)) {
        if (_c1.ssntype.get_value_or_default() != Conversion::Enums::to_db_handle(Fred::SSNType::birthday)) {
            return true;
        }
        const Nullable< boost::gregorian::date > bd1 = convert_as_birthdate(_c1.ssn);
        const Nullable< boost::gregorian::date > bd2 = convert_as_birthdate(_c2.ssn);
        if (differs(bd1, bd2)) {
            return true;
        }
    }

    return false;
}

bool notification_enabled()
{
    return CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->notify_commands;
}

void notify(Fred::OperationContext&        _ctx,
        const Notification::notified_event _event,
        unsigned long long                 _done_by_registrar,
        unsigned long long                 _object_historyid_post_change,
        LogRequestId                       _log_request_id)
{
    if (notification_enabled()) {
        Notification::enqueue_notification(_ctx, _event, _done_by_registrar, _object_historyid_post_change,
                                           Util::make_svtrid(_log_request_id));
    }
}

struct MessageType
{
    enum Enum//message_type table
    {
        domain_expiration,
        mojeid_pin2,
        mojeid_pin3,
        mojeid_sms_change,
        monitoring,
        contact_verification_pin2,
        contact_verification_pin3,
        mojeid_pin3_reminder,
        contact_check_notice,
        contact_check_thank_you,
        mojeid_card
    };
};

struct CommType
{
    enum Enum//comm_type table
    {
        email,
        letter,
        sms,
        registered_letter
    };
};

struct SendStatus
{
    enum Enum//enum_send_status table
    {
        ready,
        waiting_confirmation,
        no_processing,
        send_failed,
        sent,
        being_sent,
        undelivered
    };
};

struct PubReqType
{
    enum Enum//subset of enum_public_request_type table
    {
        contact_conditional_identification,
        conditionally_identified_contact_transfer,
        identified_contact_transfer,
        prevalidated_unidentified_contact_transfer,
        prevalidated_contact_transfer
    };
};

}//Registry::MojeID::{anonymous}
}//Registry::MojeID
}//Registry

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Registry::MojeID::MessageType::Enum value)
{
    switch (value)
    {
        case Registry::MojeID::MessageType::domain_expiration:         return "domain_expiration";
        case Registry::MojeID::MessageType::mojeid_pin2:               return "mojeid_pin2";
        case Registry::MojeID::MessageType::mojeid_pin3:               return "mojeid_pin3";
        case Registry::MojeID::MessageType::mojeid_sms_change:         return "mojeid_sms_change";
        case Registry::MojeID::MessageType::monitoring:                return "monitoring";
        case Registry::MojeID::MessageType::contact_verification_pin2: return "contact_verification_pin2";
        case Registry::MojeID::MessageType::contact_verification_pin3: return "contact_verification_pin3";
        case Registry::MojeID::MessageType::mojeid_pin3_reminder:      return "mojeid_pin3_reminder";
        case Registry::MojeID::MessageType::contact_check_notice:      return "contact_check_notice";
        case Registry::MojeID::MessageType::contact_check_thank_you:   return "contact_check_thank_you";
        case Registry::MojeID::MessageType::mojeid_card:               return "mojeid_card";
    }
    throw std::invalid_argument("value doesn't exist in Registry::MojeID::MessageType::{anonymous}::Enum");
}

template < >
inline Registry::MojeID::MessageType::Enum from_db_handle< Registry::MojeID::MessageType >(const std::string &db_handle)
{
    if (to_db_handle(Registry::MojeID::MessageType::domain_expiration) == db_handle) { return Registry::MojeID::MessageType::domain_expiration; }
    if (to_db_handle(Registry::MojeID::MessageType::mojeid_pin2) == db_handle) { return Registry::MojeID::MessageType::mojeid_pin2; }
    if (to_db_handle(Registry::MojeID::MessageType::mojeid_pin3) == db_handle) { return Registry::MojeID::MessageType::mojeid_pin3; }
    if (to_db_handle(Registry::MojeID::MessageType::mojeid_sms_change) == db_handle) { return Registry::MojeID::MessageType::mojeid_sms_change; }
    if (to_db_handle(Registry::MojeID::MessageType::monitoring) == db_handle) { return Registry::MojeID::MessageType::monitoring; }
    if (to_db_handle(Registry::MojeID::MessageType::contact_verification_pin2) == db_handle) { return Registry::MojeID::MessageType::contact_verification_pin2; }
    if (to_db_handle(Registry::MojeID::MessageType::contact_verification_pin3) == db_handle) { return Registry::MojeID::MessageType::contact_verification_pin3; }
    if (to_db_handle(Registry::MojeID::MessageType::mojeid_pin3_reminder) == db_handle) { return Registry::MojeID::MessageType::mojeid_pin3_reminder; }
    if (to_db_handle(Registry::MojeID::MessageType::contact_check_notice) == db_handle) { return Registry::MojeID::MessageType::contact_check_notice; }
    if (to_db_handle(Registry::MojeID::MessageType::contact_check_thank_you) == db_handle) { return Registry::MojeID::MessageType::contact_check_thank_you; }
    if (to_db_handle(Registry::MojeID::MessageType::mojeid_card) == db_handle) { return Registry::MojeID::MessageType::mojeid_card; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Registry::MojeID::MessageType::{anonymous}::Enum");
}

inline std::string to_db_handle(Registry::MojeID::CommType::Enum value)
{
    switch (value)
    {
        case Registry::MojeID::CommType::email:             return "email";
        case Registry::MojeID::CommType::letter:            return "letter";
        case Registry::MojeID::CommType::sms:               return "sms";
        case Registry::MojeID::CommType::registered_letter: return "registered_letter";
    }
    throw std::invalid_argument("value doesn't exist in Registry::MojeID::CommType::{anonymous}::Enum");
}

template < >
inline Registry::MojeID::CommType::Enum from_db_handle< Registry::MojeID::CommType >(const std::string &db_handle)
{
    if (to_db_handle(Registry::MojeID::CommType::email) == db_handle) { return Registry::MojeID::CommType::email; }
    if (to_db_handle(Registry::MojeID::CommType::letter) == db_handle) { return Registry::MojeID::CommType::letter; }
    if (to_db_handle(Registry::MojeID::CommType::sms) == db_handle) { return Registry::MojeID::CommType::sms; }
    if (to_db_handle(Registry::MojeID::CommType::registered_letter) == db_handle) { return Registry::MojeID::CommType::registered_letter; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Registry::MojeID::CommType::{anonymous}::Enum");
}

inline std::string to_db_handle(Registry::MojeID::SendStatus::Enum value)
{
    switch (value)
    {
        case Registry::MojeID::SendStatus::ready:                return "ready";
        case Registry::MojeID::SendStatus::waiting_confirmation: return "waiting_confirmation";
        case Registry::MojeID::SendStatus::no_processing:        return "no_processing";
        case Registry::MojeID::SendStatus::send_failed:          return "send_failed";
        case Registry::MojeID::SendStatus::sent:                 return "sent";
        case Registry::MojeID::SendStatus::being_sent:           return "being_sent";
        case Registry::MojeID::SendStatus::undelivered:          return "undelivered";
    }
    throw std::invalid_argument("value doesn't exist in Registry::MojeID::SendStatus::{anonymous}::Enum");
}

template < >
inline Registry::MojeID::SendStatus::Enum from_db_handle< Registry::MojeID::SendStatus >(const std::string &db_handle)
{
    if (to_db_handle(Registry::MojeID::SendStatus::ready) == db_handle) { return Registry::MojeID::SendStatus::ready; }
    if (to_db_handle(Registry::MojeID::SendStatus::waiting_confirmation) == db_handle) { return Registry::MojeID::SendStatus::waiting_confirmation; }
    if (to_db_handle(Registry::MojeID::SendStatus::no_processing) == db_handle) { return Registry::MojeID::SendStatus::no_processing; }
    if (to_db_handle(Registry::MojeID::SendStatus::send_failed) == db_handle) { return Registry::MojeID::SendStatus::send_failed; }
    if (to_db_handle(Registry::MojeID::SendStatus::sent) == db_handle) { return Registry::MojeID::SendStatus::sent; }
    if (to_db_handle(Registry::MojeID::SendStatus::being_sent) == db_handle) { return Registry::MojeID::SendStatus::being_sent; }
    if (to_db_handle(Registry::MojeID::SendStatus::undelivered) == db_handle) { return Registry::MojeID::SendStatus::undelivered; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Registry::MojeID::SendStatus::{anonymous}::Enum");
}

inline std::string to_db_handle(Registry::MojeID::PubReqType::Enum value)
{
    switch (value)
    {
        case Registry::MojeID::PubReqType::contact_conditional_identification:
            return Fred::MojeID::PublicRequest::ContactConditionalIdentification().get_public_request_type();
        case Registry::MojeID::PubReqType::conditionally_identified_contact_transfer:
            return Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer().get_public_request_type();
        case Registry::MojeID::PubReqType::identified_contact_transfer:
            return Fred::MojeID::PublicRequest::IdentifiedContactTransfer().get_public_request_type();
        case Registry::MojeID::PubReqType::prevalidated_contact_transfer:
            return Fred::MojeID::PublicRequest::PrevalidatedContactTransfer().get_public_request_type();
        case Registry::MojeID::PubReqType::prevalidated_unidentified_contact_transfer:
            return Fred::MojeID::PublicRequest::PrevalidatedUnidentifiedContactTransfer().get_public_request_type();
    }
    throw std::invalid_argument("value doesn't exist in Registry::MojeID::PubReqType::{anonymous}::Enum");
}

template < >
inline Registry::MojeID::PubReqType::Enum from_db_handle< Registry::MojeID::PubReqType >(const std::string &db_handle)
{
    if (to_db_handle(Registry::MojeID::PubReqType::contact_conditional_identification) == db_handle) { return Registry::MojeID::PubReqType::contact_conditional_identification; }
    if (to_db_handle(Registry::MojeID::PubReqType::conditionally_identified_contact_transfer) == db_handle) { return Registry::MojeID::PubReqType::conditionally_identified_contact_transfer; }
    if (to_db_handle(Registry::MojeID::PubReqType::identified_contact_transfer) == db_handle) { return Registry::MojeID::PubReqType::identified_contact_transfer; }
    if (to_db_handle(Registry::MojeID::PubReqType::prevalidated_contact_transfer) == db_handle) { return Registry::MojeID::PubReqType::prevalidated_contact_transfer; }
    if (to_db_handle(Registry::MojeID::PubReqType::prevalidated_unidentified_contact_transfer) == db_handle) { return Registry::MojeID::PubReqType::prevalidated_unidentified_contact_transfer; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Registry::MojeID::PubReqType::{anonymous}::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

namespace Registry {
namespace MojeID {

namespace {

template < MessageType::Enum MT, CommType::Enum CT >
::size_t cancel_message_sending(Fred::OperationContext &_ctx, MojeIDImpl::ContactId _contact_id)
{
    const Database::Result result = _ctx.get_conn().exec_params(
        "UPDATE message_archive ma "
        "SET moddate=NOW(),"
            "status_id=(SELECT id FROM enum_send_status WHERE status_name=$4::TEXT) "
        "FROM message_contact_history_map mchm "
        "JOIN letter_archive la ON la.id=mchm.message_archive_id "
        "WHERE mchm.message_archive_id=ma.id AND "
              "mchm.contact_object_registry_id=$1::BIGINT AND "
              "ma.status_id IN (SELECT id FROM enum_send_status "
                               "WHERE status_name IN ($5::TEXT,$6::TEXT)) AND "
              "ma.comm_type_id=(SELECT id FROM comm_type WHERE type=$2::TEXT) AND "
              "ma.message_type_id=(SELECT id FROM message_type WHERE type=$3::TEXT) "
        "RETURNING ma.id",
        Database::query_param_list(_contact_id)                                               //$1::BIGINT
                                  (Conversion::Enums::to_db_handle(CT))                       //$2::TEXT
                                  (Conversion::Enums::to_db_handle(MT))                       //$3::TEXT
                                  (Conversion::Enums::to_db_handle(SendStatus::no_processing))//$4::TEXT
                                  (Conversion::Enums::to_db_handle(SendStatus::send_failed))  //$5::TEXT
                                  (Conversion::Enums::to_db_handle(SendStatus::ready)));      //$6::TEXT
    return result.size();
}

bool identified_data_changed(const Fred::InfoContactData &_c1, const Fred::InfoContactData &_c2)
{
    if (differs(_c1.name, _c2.name)) {
        return true;
    }

    const Fred::InfoContactData::Address a1 = _c1.get_address< Fred::ContactAddressType::MAILING >();
    const Fred::InfoContactData::Address a2 = _c2.get_address< Fred::ContactAddressType::MAILING >();
    if (differs(a1.name, a2.name)) {
        const std::string name1 = a1.name.isset() ? a1.name.get_value() : _c1.name.get_value_or_default();
        const std::string name2 = a2.name.isset() ? a2.name.get_value() : _c2.name.get_value_or_default();
        if (differs(name1, name2)) {
            return true;
        }
    }
    if (differs(a1.street1,         a2.street1)         ||
        differs(a1.street2,         a2.street2)         ||
        differs(a1.street3,         a2.street3)         ||
        differs(a1.city,            a2.city)            ||
        differs(a1.stateorprovince, a2.stateorprovince) ||
        differs(a1.country,         a2.country)         ||
        differs(a1.postalcode,      a2.postalcode)) {
        return true;
    }

    return false;
}

typedef data_storage< std::string, MojeIDImpl::ContactId >::safe prepare_transaction_storage;
typedef prepare_transaction_storage::object_type::data_not_found prepare_transaction_data_not_found;

}//Registry::MojeID::{anonymous}

MojeIDImpl::MojeIDImpl(const std::string &_server_name)
:   server_name_(_server_name),
    mojeid_registrar_handle_(get_mojeid_registrar_handle()),
    mojeid_registrar_id_(get_mojeid_registrar_id(mojeid_registrar_handle_))
{
    LogContext log_ctx(*this, "init");
}//MojeIDImpl::MojeIDImpl

MojeIDImpl::~MojeIDImpl()
{
}

const std::string& MojeIDImpl::get_server_name()const
{
    return server_name_;
}

void MojeIDImpl::get_unregistrable_contact_handles(
        MojeIDImplData::ContactHandleList &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Database::Result dbres = ctx.get_conn().exec(
            "WITH static_data AS ("
                "SELECT eot.id AS type_id,"
                       "NOW()-(ep.val||'MONTH')::INTERVAL AS contact_protected_since "
                "FROM enum_object_type eot,enum_parameters ep "
                "WHERE eot.name='contact' AND "
                      "ep.name='handle_registration_protection_period') "
            "SELECT name "
            "FROM object_registry "
            "WHERE type=(SELECT type_id FROM static_data) AND "
                  "COALESCE((SELECT contact_protected_since FROM static_data)<erdate,TRUE) AND "
                  "LOWER(name)~'^[a-z0-9](-?[a-z0-9])*$'");
        _result.clear();
        _result.reserve(dbres.size());
        for (::size_t idx = 0; idx < dbres.size(); ++idx) {
            _result.push_back(static_cast< std::string >(dbres[idx][0]));
        }
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

namespace {

Optional< LogRequestId > get_optional_log_request_id(LogRequestId _log_request_id)
{
    if (0 < _log_request_id) {
        return _log_request_id;
    }
    return Optional< LogRequestId >();
}

}

MojeIDImpl::ContactId MojeIDImpl::create_contact_prepare(
        const MojeIDImplData::CreateContact &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);

        Fred::InfoContactData info_contact_data;
        from_into(_contact, info_contact_data);
        {
            const MojeIDImplInternal::CheckCreateContactPrepare check_contact_data(
                Fred::make_args(info_contact_data),
                Fred::make_args(info_contact_data, ctx));

            if (!check_contact_data.success()) {
                MojeIDImplInternal::raise(check_contact_data);
            }
        }

        Fred::CreateContact op_create_contact(_contact.username, mojeid_registrar_handle_);
        set_create_contact_arguments(_contact, op_create_contact);
        if (0 < _log_request_id) {
            op_create_contact.set_logd_request_id(_log_request_id);
        }
        const Fred::CreateContact::Result new_contact = op_create_contact.exec(ctx);
        Fred::CreatePublicRequestAuth op_create_pub_req;
        op_create_pub_req.set_registrar_id(mojeid_registrar_id_);
        Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, new_contact.create_object_result.object_id);
        {
            const Fred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(
                locked_contact, Fred::MojeID::PublicRequest::ContactConditionalIdentification().iface(),
                get_optional_log_request_id(_log_request_id));
            _ident = result.identification;
            notify(ctx, Notification::created,
                   mojeid_registrar_id_, new_contact.create_object_result.history_id, _log_request_id);
        }
        prepare_transaction_storage()->store(_trans_id, new_contact.create_object_result.object_id);
        ctx.commit_transaction();
        return new_contact.create_object_result.object_id;
    }
    catch (const MojeIDImplData::RegistrationValidationResult &e) {
        LOGGER(PACKAGE).info("request failed (incorrect input data)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

namespace {

Fred::CreatePublicRequestAuth::Result action_transfer_contact_prepare(
    const Fred::PublicRequestAuthTypeIface &_iface,
    const std::string &_trans_id,
    const Fred::InfoContactData &_contact,
    const Fred::LockedPublicRequestsOfObjectForUpdate &_locked_contact,
    unsigned long long _registrar_id,
    LogRequestId _log_request_id)
{
    Fred::CreatePublicRequestAuth op_create_pub_req;
    if (!_contact.notifyemail.isnull()) {
        op_create_pub_req.set_email_to_answer(_contact.notifyemail.get_value());
    }
    op_create_pub_req.set_registrar_id(_registrar_id);
    const Fred::CreatePublicRequestAuth::Result result =
        op_create_pub_req.exec(_locked_contact, _iface, get_optional_log_request_id(_log_request_id));
    prepare_transaction_storage()->store(_trans_id, _contact.id);
    return result;
}

}

void MojeIDImpl::transfer_contact_prepare(
        const std::string &_handle,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        MojeIDImplData::InfoContact &_contact,
        std::string &_ident)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const Fred::InfoContactData contact = Fred::InfoContactByHandle(_handle).exec(ctx).info_contact_data;
        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, contact.id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(contact.id).exec(ctx));
        {
            const MojeIDImplInternal::CheckTransferContactPrepareStates check_result(states);
            if (!check_result.success()) {
                MojeIDImplInternal::raise(check_result);
            }
        }
        {
            const MojeIDImplInternal::CheckMojeIDRegistration check_result(
                Fred::make_args(contact), Fred::make_args(contact, ctx));
            if (!check_result.success()) {
                MojeIDImplInternal::raise(check_result);
            }
        }

        Fred::CreatePublicRequestAuth::Result pub_req_result;
        if (states.absents(Fred::Object::State::conditionally_identified_contact) &&
            states.absents(Fred::Object::State::identified_contact) &&
            states.absents(Fred::Object::State::validated_contact))
        {
            pub_req_result = action_transfer_contact_prepare(
                Fred::MojeID::PublicRequest::ContactConditionalIdentification(),
                _trans_id, contact, locked_contact, mojeid_registrar_id_, _log_request_id);
        }
        else if (states.presents(Fred::Object::State::conditionally_identified_contact) &&
                 states.absents(Fred::Object::State::identified_contact) &&
                 states.absents(Fred::Object::State::validated_contact))
        {
            pub_req_result = action_transfer_contact_prepare(
                Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer(),
                _trans_id, contact, locked_contact, mojeid_registrar_id_, _log_request_id);
        }
        else if (states.presents(Fred::Object::State::conditionally_identified_contact) &&
                 states.presents(Fred::Object::State::identified_contact) &&
                 states.absents(Fred::Object::State::validated_contact))
        {
            pub_req_result = action_transfer_contact_prepare(
                Fred::MojeID::PublicRequest::IdentifiedContactTransfer(),
                _trans_id, contact, locked_contact, mojeid_registrar_id_, _log_request_id);
        }

        from_into(contact, _contact);
        ctx.commit_transaction();
        _ident = pub_req_result.identification;
        return;
    }
    catch (const MojeIDImplData::AlreadyMojeidContact&) {
        LOGGER(PACKAGE).info("request failed (incorrect input data - AlreadyMojeidContact)");
        throw;
    }
    catch (const MojeIDImplData::ObjectAdminBlocked&) {
        LOGGER(PACKAGE).info("request failed (incorrect input data - ObjectAdminBlocked)");
        throw;
    }
    catch (const MojeIDImplData::ObjectUserBlocked&) {
        LOGGER(PACKAGE).info("request failed (incorrect input data - ObjectUserBlocked)");
        throw;
    }
    catch (const MojeIDImplData::RegistrationValidationResult&) {
        LOGGER(PACKAGE).info("request failed (incorrect input data - RegistrationValidationResult)");
        throw;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).info("request failed (incorrect input data)");
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

namespace {

template < Fred::ContactAddressType::Value ADDRESS_TYPE, typename UPDATE_CONTACT >
void update_address(const Fred::ContactAddressList &_old_addresses,
                    const Fred::ContactAddressList &_new_addresses,
                    Fred::UpdateContact< UPDATE_CONTACT > &_update_op)
{
    Fred::ContactAddressList::const_iterator old_ptr = _old_addresses.find(ADDRESS_TYPE);
    const bool old_presents = old_ptr != _old_addresses.end();
    Fred::ContactAddressList::const_iterator new_ptr = _new_addresses.find(ADDRESS_TYPE);
    const bool new_presents = new_ptr != _new_addresses.end();
    if (new_presents) {
        if (!old_presents || (old_ptr->second != new_ptr->second)) {
            _update_op.template set_address< ADDRESS_TYPE >(new_ptr->second);
        }
        return;
    }
    if (old_presents) {
        _update_op.template reset_address< ADDRESS_TYPE >();
    }
}

template < Fred::ContactAddressType::Value ADDRESS_TYPE, typename UPDATE_CONTACT >
void update_address(const Fred::InfoContactDiff &_data_changes,
                    Fred::UpdateContact< UPDATE_CONTACT > &_update_op)
{
    update_address< ADDRESS_TYPE >(_data_changes.addresses.get_value().first,
                                   _data_changes.addresses.get_value().second,
                                   _update_op);
}

template < typename UPDATE_CONTACT >
void set_update_contact_op(const Fred::InfoContactDiff &_data_changes,
                           Fred::UpdateContact< UPDATE_CONTACT > &_update_op)
{
    if (_data_changes.name.isset()) {
        _update_op.set_name(_data_changes.name.get_value().second);
    }
    if (_data_changes.organization.isset()) {
        _update_op.set_organization(_data_changes.organization.get_value().second);
    }
    if (_data_changes.vat.isset()) {
        _update_op.set_vat(_data_changes.vat.get_value().second);
    }
    if (_data_changes.personal_id.isset()) {
        _update_op.set_personal_id(_data_changes.personal_id.get_value().second);
    }
    if (_data_changes.place.isset()) {
        _update_op.set_place(_data_changes.place.get_value().second);
    }
    if (_data_changes.addresses.isset()) {
        update_address< Fred::ContactAddressType::MAILING >   (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::BILLING >   (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING >  (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING_2 >(_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING_3 >(_data_changes, _update_op);
    }
    if (_data_changes.email.isset()) {
        _update_op.set_email(_data_changes.email.get_value().second);
    }
    if (_data_changes.notifyemail.isset()) {
        _update_op.set_notifyemail(_data_changes.notifyemail.get_value().second);
    }
    if (_data_changes.telephone.isset()) {
        _update_op.set_telephone(_data_changes.telephone.get_value().second);
    }
    if (_data_changes.fax.isset()) {
        _update_op.set_fax(_data_changes.fax.get_value().second);
    }
}

}

void MojeIDImpl::update_contact_prepare(
        ContactId _contact_id,
        const MojeIDImplData::UpdateContact &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::InfoContactData new_data;
        from_into(_new_data, new_data);
        new_data.id = _contact_id;
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(new_data.id).exec(ctx));
        if (states.absents(Fred::Object::State::mojeid_contact)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        const Fred::InfoContactData current_data = Fred::InfoContactById(new_data.id).exec(ctx).info_contact_data;
        const Fred::InfoContactDiff data_changes = Fred::diff_contact_data(current_data, new_data);
        if (!(data_changes.name.isset()         ||
              data_changes.organization.isset() ||
              data_changes.vat.isset()          ||
              data_changes.personal_id.isset()  ||
              data_changes.place.isset()        ||
              data_changes.addresses.isset()    ||
              data_changes.email.isset()        ||
              data_changes.notifyemail.isset()  ||
              data_changes.telephone.isset()    ||
              data_changes.fax.isset())) {
            ctx.commit_transaction();
            return;
        }
        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, new_data.id);
        Fred::StatusList to_cancel;
        bool drop_validation = false;
        if (states.presents(Fred::Object::State::validated_contact)) {
            drop_validation = validated_data_changed(current_data, new_data);
            if (drop_validation) {
                to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::validated_contact));
            }
        }
        const bool drop_identification = identified_data_changed(current_data, new_data);
        if (drop_identification || differs(current_data.email, new_data.email)) {
            cancel_message_sending< MessageType::mojeid_card, CommType::letter >(ctx, new_data.id);
            cancel_message_sending< MessageType::mojeid_pin3, CommType::letter >(ctx, new_data.id);
        }
        if (drop_identification) {
            bool reidentification_needed = states.presents(Fred::Object::State::identified_contact);
            if (reidentification_needed) {
                to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::identified_contact));
            }
            else {
                const Database::Result dbres = ctx.get_conn().exec_params(
                    "SELECT 1 "
                    "FROM object_registry obr "
                    "JOIN object_state os ON os.object_id=obr.id AND "
                                            "os.state_id=(SELECT id FROM enum_object_states WHERE name=$2::TEXT) "
                    "WHERE obr.id=$1::BIGINT AND "
                          "os.valid_to IS NULL AND "
                          "EXISTS(SELECT * FROM object_state "
                                 "WHERE object_id=obr.id AND "
                                       "state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                       "(os.valid_from<=valid_from OR "
                                       " os.valid_from<valid_to))",
                    Database::query_param_list
                        (new_data.id)                                                               //$1::BIGINT
                        (Conversion::Enums::to_db_handle(Fred::Object::State::mojeid_contact))      //$2::TEXT
                        (Conversion::Enums::to_db_handle(Fred::Object::State::identified_contact)));//$3::TEXT
                const bool contact_was_identified_in_the_past = 0 < dbres.size();
                if (contact_was_identified_in_the_past) {
                    reidentification_needed = true;
                }
            }
            const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->
                                                                get_handler_ptr_by_type< HandleMojeIDArgs >();
            check_sent_letters_limit(ctx,
                                     new_data.id,
                                     server_conf_ptr->letter_limit_count,
                                     server_conf_ptr->letter_limit_interval);
            Fred::CreatePublicRequestAuth create_public_request_op;
            create_public_request_op.set_reason("data changed");
            create_public_request_op.set_registrar_id(mojeid_registrar_id_);
            create_public_request_op.exec(
                locked_contact,
                reidentification_needed ? Fred::MojeID::PublicRequest::ContactReidentification().iface()
                                        : Fred::MojeID::PublicRequest::ContactIdentification().iface(),
                get_optional_log_request_id(_log_request_id));
        }
        {
            const MojeIDImplInternal::CheckUpdateContactPrepare check_contact_data(new_data);
            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
        }
        const bool manual_verification_done = states.presents(Fred::Object::State::contact_failed_manual_verification) ||
                                              states.presents(Fred::Object::State::contact_passed_manual_verification);
        if (manual_verification_done) {
            const bool cancel_manual_verification = data_changes.name.isset()         ||
                                                    data_changes.organization.isset() ||
                                                    data_changes.personal_id.isset()  ||
                                                    data_changes.place.isset()        ||
                                                    data_changes.email.isset()        ||
                                                    data_changes.notifyemail.isset()  ||
                                                    data_changes.telephone.isset()    ||
                                                    data_changes.fax.isset();
            if (cancel_manual_verification) {
                if (states.presents(Fred::Object::State::contact_failed_manual_verification)) {
                    to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::contact_failed_manual_verification));
                }
                if (states.presents(Fred::Object::State::contact_passed_manual_verification)) {
                    to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::contact_passed_manual_verification));
                }
            }
        }
        const bool object_states_changed = !to_cancel.empty();
        if (object_states_changed) {
            Fred::CancelObjectStateRequestId(new_data.id, to_cancel).exec(ctx);
        }
        Fred::UpdateContactById update_contact_op(new_data.id, mojeid_registrar_handle_);
        set_update_contact_op(data_changes, update_contact_op);
        const bool is_identified = states.presents(Fred::Object::State::identified_contact) && !drop_identification;
        const bool is_validated  = states.presents(Fred::Object::State::validated_contact) && !drop_validation;
        const bool addr_can_be_hidden = (is_identified || is_validated) &&
                                        new_data.organization.get_value_or_default().empty();
        if (!addr_can_be_hidden) {
            update_contact_op.set_discloseaddress(true);
        }
        if (0 < _log_request_id) {
            update_contact_op.set_logd_request_id(_log_request_id);
        }
        const unsigned long long history_id = update_contact_op.exec(ctx);

        notify(ctx, Notification::updated, mojeid_registrar_id_, history_id, _log_request_id);

        if (object_states_changed) {
            prepare_transaction_storage()->store(_trans_id, new_data.id);
        }

        ctx.commit_transaction();
        return;
    }
    catch (const Fred::InfoContactById::Exception &e) {
        if (e.is_set_unknown_object_id()) {
            LOGGER(PACKAGE).info("request failed (InfoContactById::Exception - unknown_object_id)");
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).info("request failed (InfoContactById::Exception)");
        throw;
    }
    catch (const MojeIDImplData::ObjectDoesntExist &e) {
        LOGGER(PACKAGE).info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch(const MojeIDImplData::MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.as_string());
        throw;
    }
    catch(const MojeIDImplInternal::CheckUpdateContactPrepare &e) {
        LOGGER(PACKAGE).info("request failed (CheckUpdateContactPrepare)");
        MojeIDImplInternal::raise(e);
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

MojeIDImplData::InfoContact MojeIDImpl::update_transfer_contact_prepare(
        const std::string &_username,
        const MojeIDImplData::UpdateTransferContact &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::InfoContactData new_data;
        from_into(_new_data, new_data);
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        //check contact is registered
        const Fred::InfoContactData current_data = Fred::InfoContactByHandle(_username).exec(ctx).info_contact_data;
        new_data.id     = current_data.id;
        new_data.handle = current_data.handle;
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(new_data.id).exec(ctx));
        {
            const MojeIDImplInternal::CheckTransferContactPrepareStates check_result(states);
            if (!check_result.success()) {
                MojeIDImplInternal::raise(check_result);
            }
        }
        check_limits::sent_letters()(ctx, current_data.id);
        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, new_data.id);
        bool drop_identification      = false;
        bool drop_cond_identification = false;
        unsigned long long history_id;
        {
            if (states.presents(Fred::Object::State::identified_contact)) {
                const Fred::InfoContactData &c1 = current_data;
                const Fred::InfoContactData &c2 = new_data;
                drop_identification = c1.name.get_value_or_default() != c2.name.get_value_or_default();
                if (!drop_identification) {
                    const Fred::InfoContactData::Address a1 = c1.get_address< Fred::ContactAddressType::MAILING >();
                    const Fred::InfoContactData::Address a2 = c2.get_address< Fred::ContactAddressType::MAILING >();
                    drop_identification =
                        (a1.name.get_value_or_default()            != a2.name.get_value_or_default())            ||
                        (a1.organization.get_value_or_default()    != a2.organization.get_value_or_default())    ||
                        (a1.company_name.get_value_or_default()    != a2.company_name.get_value_or_default())    ||
                        (a1.street1                                != a2.street1)                                ||
                        (a1.street2.get_value_or_default()         != a2.street2.get_value_or_default())         ||
                        (a1.street3.get_value_or_default()         != a2.street3.get_value_or_default())         ||
                        (a1.city                                   != a2.city)                                   ||
                        (a1.stateorprovince.get_value_or_default() != a2.stateorprovince.get_value_or_default()) ||
                        (a1.postalcode                             != a2.postalcode)                             ||
                        (a1.country                                != a2.country);
                }
            }
            if (states.presents(Fred::Object::State::conditionally_identified_contact) ||
                (!drop_identification && states.presents(Fred::Object::State::identified_contact)))
            {
                const Fred::InfoContactData &c1 = current_data;
                const Fred::InfoContactData &c2 = new_data;
                drop_cond_identification =
                    (c1.telephone.get_value_or_default() != c2.telephone.get_value_or_default()) ||
                    (c1.email.get_value_or_default()     != c2.email.get_value_or_default());
                drop_identification |= drop_cond_identification;
            }
            if (drop_cond_identification || drop_identification) {
                Fred::StatusList to_cancel;
                //drop conditionally identified flag if e-mail or mobile changed
                if (drop_cond_identification &&
                    states.presents(Fred::Object::State::conditionally_identified_contact))
                {
                    to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::conditionally_identified_contact));
                }
                //drop identified flag if name, mailing address, e-mail or mobile changed
                if (drop_identification &&
                    states.presents(Fred::Object::State::identified_contact))
                {
                    to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::identified_contact));
                }
                if (!to_cancel.empty()) {
                    try {
                        Fred::CancelObjectStateRequestId(current_data.id, to_cancel).exec(ctx);
                    }
                    catch (const Fred::CancelObjectStateRequestId::Exception &e) {
                        if (e.is_set_object_id_not_found()) {
                            throw MojeIDImplData::ObjectDoesntExist();
                        }
                        if (e.is_set_state_not_found()) {
                            LOGGER(PACKAGE).info("unable clear state " + e.get_state_not_found());
                        }
                        else {
                            throw;
                        }
                    }
                }
            }

            {
                const MojeIDImplInternal::CheckUpdateTransferContactPrepare result_of_check(new_data);

                if (!result_of_check.success()) {
                    MojeIDImplInternal::raise(result_of_check);
                }
            }
            if (current_data.sponsoring_registrar_handle != mojeid_registrar_handle_) {
                Fred::TransferContact transfer_contact_op(current_data.id,
                                                          mojeid_registrar_handle_,
                                                          current_data.authinfopw,
                                                          0 < _log_request_id ? _log_request_id
                                                                              : Nullable< LogRequestId >());
                //transfer contact to 'REG-MOJEID' sponsoring registrar
                const unsigned long long history_id = transfer_contact_op.exec(ctx);
                notify(ctx, Notification::transferred,
                       mojeid_registrar_id_, history_id, _log_request_id);
                Fred::Poll::CreateTransferContactPollMessage(history_id).exec(ctx);
            }
            //perform changes
            Fred::UpdateContactById update_contact_op(new_data.id, mojeid_registrar_handle_);
            set_update_contact_op(Fred::diff_contact_data(current_data, new_data), update_contact_op);
            if (0 < _log_request_id) {
                update_contact_op.set_logd_request_id(_log_request_id);
            }
            history_id = update_contact_op.exec(ctx);
        }
        const bool is_cond_identified = states.presents(Fred::Object::State::conditionally_identified_contact) &&
                                        !drop_cond_identification;
        Fred::CreatePublicRequestAuth op_create_pub_req;
        if (!current_data.notifyemail.get_value_or_default().empty()) {
            op_create_pub_req.set_email_to_answer(current_data.notifyemail.get_value());
        }
        op_create_pub_req.set_registrar_id(mojeid_registrar_id_);
        const Fred::CreatePublicRequestAuth::Result result =
            op_create_pub_req.exec(
                locked_contact,
                //for 'conditionallyIdentifiedContact' or 'identifiedContact' create 'mojeid_prevalidated_contact_transfer' public request
                is_cond_identified ? Fred::MojeID::PublicRequest::PrevalidatedContactTransfer().iface()
                //in other cases create 'mojeid_prevalidated_unidentified_contact_transfer' public request
                                   : Fred::MojeID::PublicRequest::PrevalidatedUnidentifiedContactTransfer().iface(),
                get_optional_log_request_id(_log_request_id));

        notify(ctx, Notification::updated, mojeid_registrar_id_, history_id, _log_request_id);
        //second phase commit will change contact states
        prepare_transaction_storage()->store(_trans_id, current_data.id);

        MojeIDImplData::InfoContact changed_data;
        from_into(Fred::InfoContactById(current_data.id).exec(ctx).info_contact_data, changed_data);
        ctx.commit_transaction();
        _ident = result.identification;
        return changed_data;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        //check contact is registered, throw OBJECT_NOT_EXISTS if isn't
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).info("request failed (InfoContactByHandle::Exception) - unknown_contact_handle");
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIDImplData::RegistrationValidationResult&) {
        LOGGER(PACKAGE).info("request failed (RegistrationValidationResult)");
        throw;
    }
    catch (const MojeIDImplData::AlreadyMojeidContact&) {
        LOGGER(PACKAGE).info("request failed (AlreadyMojeidContact)");
        throw;
    }
    catch (const MojeIDImplData::ObjectAdminBlocked&) {
        LOGGER(PACKAGE).info("request failed (ObjectAdminBlocked)");
        throw;
    }
    catch (const MojeIDImplData::ObjectUserBlocked&) {
        LOGGER(PACKAGE).info("request failed (ObjectUserBlocked)");
        throw;
    }
    catch(const MojeIDImplData::MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.as_string());
        throw;
    }
    catch(const MojeIDImplData::ObjectDoesntExist &e) {
        LOGGER(PACKAGE).info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

namespace {

Fred::UpdatePublicRequest::Result set_status(
    const Fred::LockedPublicRequestForUpdate &_locked_request,
    const Fred::PublicRequestTypeIface &_request_type,
    Fred::PublicRequest::Status::Enum _status,
    const std::string &_reason,
    MojeIDImpl::LogRequestId _log_request_id)
{
    Fred::UpdatePublicRequest op_update_public_request;
    op_update_public_request.set_status(_status);
    if (!_reason.empty()) {
        op_update_public_request.set_reason(_reason);
    }
    return op_update_public_request.exec(_locked_request, _request_type, get_optional_log_request_id(_log_request_id));
}

Fred::UpdatePublicRequest::Result answer(
    const Fred::LockedPublicRequestForUpdate &_locked_request,
    const Fred::PublicRequestTypeIface &_request_type,
    const std::string &_reason,
    MojeIDImpl::LogRequestId _log_request_id)
{
    return set_status(_locked_request, _request_type, Fred::PublicRequest::Status::answered, _reason, _log_request_id);
}

Fred::UpdatePublicRequest::Result invalidate(
    const Fred::LockedPublicRequestForUpdate &_locked_request,
    const Fred::PublicRequestTypeIface &_request_type,
    const std::string &_reason,
    MojeIDImpl::LogRequestId _log_request_id)
{
    return set_status(_locked_request, _request_type, Fred::PublicRequest::Status::invalidated, _reason, _log_request_id);
}

//ticket #15587 hack
void invalid_birthday_looks_like_no_birthday(MojeIDImplData::InfoContact &_data)
{
    if (!_data.birth_date.isnull()) {
        const boost::gregorian::date invalid_date(boost::gregorian::not_a_date_time);
        const std::string invalid_date_str = boost::gregorian::to_iso_extended_string(invalid_date);
        if (_data.birth_date.get_value().value == invalid_date_str) { //make believe that invalid birthday
            _data.birth_date = Nullable< MojeIDImplData::Date >();    //is no birthday
        }
    }
}

}//namespace Registry::MojeID::{anonymous}

void MojeIDImpl::info_contact(
        const std::string &_username,
        MojeIDImplData::InfoContact &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        from_into(Fred::InfoContactByHandle(_username).exec(ctx).info_contact_data, _result);
        invalid_birthday_looks_like_no_birthday(_result);//ticket #15587 hack
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).info("request failed (ObjectDoesntExist)");
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error("request failed (Fred::InfoContactByHandle failure)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeIDImpl::get_contact_info_publish_flags(
        ContactId _contact_id,
        MojeIDImplData::InfoContactPublishFlags &_flags)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Fred::InfoContactData data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));

        if (states.presents(Fred::Object::State::linked)) {
            _flags.first_name   = data.disclosename;
            _flags.last_name    = data.disclosename;
            _flags.organization = data.discloseorganization;
            _flags.vat_reg_num  = data.discloseident;
            _flags.birth_date   = data.discloseident;
            _flags.id_card_num  = data.discloseident;
            _flags.passport_num = data.discloseident;
            _flags.ssn_id_num   = data.discloseident;
            _flags.vat_id_num   = data.discloseident;
            _flags.email        = data.discloseemail;
            _flags.notify_email = data.disclosenotifyemail;
            _flags.telephone    = data.disclosetelephone;
            _flags.fax          = data.disclosefax;
            _flags.permanent    = data.discloseaddress;
            _flags.mailing      = false;
            _flags.billing      = false;
            _flags.shipping     = false;
            _flags.shipping2    = false;
            _flags.shipping3    = false;
        }
        else {
            _flags.first_name   = data.disclosename;
            _flags.last_name    = data.disclosename;
            _flags.organization = false;
            _flags.vat_reg_num  = false;
            _flags.birth_date   = false;
            _flags.id_card_num  = false;
            _flags.passport_num = false;
            _flags.ssn_id_num   = false;
            _flags.vat_id_num   = false;
            _flags.email        = false;
            _flags.notify_email = false;
            _flags.telephone    = false;
            _flags.fax          = false;
            _flags.permanent    = false;
            _flags.mailing      = false;
            _flags.billing      = false;
            _flags.shipping     = false;
            _flags.shipping2    = false;
            _flags.shipping3    = false;
        }
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::InfoContactById::Exception &e) {
        if (e.is_set_unknown_object_id()) {
            LOGGER(PACKAGE).info("request failed (incorrect input data)");
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

MojeIDImpl::ContactId MojeIDImpl::process_registration_request(
        const std::string &_ident_request_id,
        const std::string &_password,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Fred::PublicRequestLockGuardByIdentification locked_request(ctx, _ident_request_id);
        const Fred::PublicRequestAuthInfo pub_req_info(ctx, locked_request);
        if (pub_req_info.get_object_id().isnull()) {
            invalidate(locked_request,
                       Fred::FakePublicRequestForInvalidating(pub_req_info.get_type()).iface(),
                       "no object associated with this public request",
                       _log_request_id);
            throw MojeIDImplData::IdentificationRequestDoesntExist();
        }
        const Fred::ObjectId contact_id = pub_req_info.get_object_id().get_value();
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(contact_id).exec(ctx));
        const PubReqType::Enum pub_req_type(Conversion::Enums::from_db_handle< PubReqType >(pub_req_info.get_type()));
        try {
            switch (pub_req_info.get_status()) {
            case Fred::PublicRequest::Status::active:
                break;
            case Fred::PublicRequest::Status::answered:
                throw MojeIDImplData::IdentificationAlreadyProcessed();
            case Fred::PublicRequest::Status::invalidated:
                throw MojeIDImplData::IdentificationAlreadyInvalidated();
            }

            Fred::StatusList to_set;
            switch (pub_req_type) {
            case PubReqType::contact_conditional_identification:
                to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::conditionally_identified_contact));
                break;
            case PubReqType::prevalidated_unidentified_contact_transfer:
                to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::conditionally_identified_contact));
                if (states.absents(Fred::Object::State::validated_contact)) {
                    to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::validated_contact));
                }
                break;
            case PubReqType::conditionally_identified_contact_transfer:
            case PubReqType::identified_contact_transfer:
                break;
            case PubReqType::prevalidated_contact_transfer:
                if (states.absents(Fred::Object::State::validated_contact)) {
                    to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::validated_contact));
                }
                break;
            default:
                throw std::runtime_error("unexpected public request type " + pub_req_type);
            }

            const Database::Result dbres = ctx.get_conn().exec_params(
                "SELECT EXISTS(SELECT 1 FROM public_request "
                              "WHERE id=$1::BIGINT AND "
                                    "create_time<(SELECT GREATEST(update,trdate) FROM object "
                                                 "WHERE id=$2::BIGINT)"
                             ") AS object_changed",
                Database::query_param_list(static_cast< const Fred::LockedPublicRequest& >
                                          (locked_request).get_id())//$1::BIGINT
                                          (contact_id));            //$2::BIGINT

            if (dbres.size() != 1) {
                throw std::runtime_error("something wrong happened, database looks to be crazy, this query has to return exactly one row");
            }

            const bool contact_changed = static_cast< bool >(dbres[0][0]);
            if (contact_changed) {
                invalidate(locked_request,
                           Fred::FakePublicRequestForInvalidating(pub_req_info.get_type()).iface(),
                           "contact data changed after the public request had been created",
                           _log_request_id);
                throw MojeIDImplData::ContactChanged();
            }

            if (!pub_req_info.check_password(_password)) {
                throw MojeIDImplData::IdentificationFailed();
            }

            to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::server_delete_prohibited));
            to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::server_transfer_prohibited));
            to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::server_update_prohibited));
            to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::mojeid_contact));
            Fred::CreateObjectStateRequestId(contact_id, to_set).exec(ctx);
            Fred::PerformObjectStateRequest(contact_id).exec(ctx);

            const Fred::InfoContactData contact = Fred::InfoContactById(contact_id).exec(ctx).info_contact_data;
            {
                const MojeIDImplInternal::CheckProcessRegistrationValidation check_result(contact);
                if (!check_result.success()) {
                    MojeIDImplInternal::raise(check_result);
                }
            }
            if (contact.sponsoring_registrar_handle != mojeid_registrar_handle_) {
                Fred::TransferContact transfer_contact_op(contact.id,
                                                          mojeid_registrar_handle_,
                                                          contact.authinfopw,
                                                          0 < _log_request_id ? _log_request_id
                                                                              : Nullable< LogRequestId >());
                //transfer contact to 'REG-MOJEID' sponsoring registrar
                const unsigned long long history_id = transfer_contact_op.exec(ctx);
                notify(ctx, Notification::transferred,
                       mojeid_registrar_id_, history_id, _log_request_id);
                Fred::Poll::CreateTransferContactPollMessage(history_id).exec(ctx);
            }
            answer(locked_request,
                   pub_req_type == PubReqType::contact_conditional_identification
                   ? Fred::MojeID::PublicRequest::ContactConditionalIdentification().iface()
                   : pub_req_type == PubReqType::prevalidated_unidentified_contact_transfer
                     ? Fred::MojeID::PublicRequest::PrevalidatedUnidentifiedContactTransfer().iface()
                     : pub_req_type == PubReqType::conditionally_identified_contact_transfer
                       ? Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer().iface()
                       : pub_req_type == PubReqType::identified_contact_transfer
                         ? Fred::MojeID::PublicRequest::IdentifiedContactTransfer().iface()
                         : Fred::MojeID::PublicRequest::PrevalidatedContactTransfer().iface(),
                   "successfully processed",
                   _log_request_id);

            if (Fred::Object::StatesInfo(Fred::GetObjectStates(contact_id).exec(ctx))
                    .absents(Fred::Object::State::identified_contact)) {
                Fred::CreatePublicRequestAuth op_create_pub_req;
                op_create_pub_req.set_registrar_id(mojeid_registrar_id_);
                Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, contact_id);
                const Fred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(
                    locked_contact, Fred::MojeID::PublicRequest::ContactIdentification().iface(),
                    get_optional_log_request_id(_log_request_id));
            }

            ctx.commit_transaction();

            return contact_id;
        }
        catch (const MojeIDImplData::IdentificationFailed&) {
            ctx.commit_transaction();
            throw;
        }
        catch (const MojeIDImplData::ContactChanged&) {
            ctx.commit_transaction();
            throw;
        }
    }
    catch (const MojeIDImplData::IdentificationRequestDoesntExist&) {
        throw;
    }
    catch (const MojeIDImplData::IdentificationFailed&) {
        throw;
    }
    catch (const MojeIDImplData::ContactChanged&) {
        throw;
    }
    catch (const MojeIDImplData::ProcessRegistrationValidationResult&) {
        throw;
    }
    catch (const Fred::PublicRequestLockGuardByIdentification::Exception &e) {
        if (e.is_set_public_request_doesnt_exist()) {
            LOGGER(PACKAGE).info(boost::format("request failed (%1%)") % e.what());
            throw MojeIDImplData::IdentificationRequestDoesntExist();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw std::runtime_error(e.what());
    }
    catch (const Fred::InfoContactById::Exception &e) {
        if (e.is_set_unknown_object_id()) {
            LOGGER(PACKAGE).info(boost::format("request failed (%1%)") % e.what());
            throw MojeIDImplData::IdentificationFailed();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeIDImpl::process_identification_request(
        ContactId _contact_id,
        const std::string &_password,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        Fred::PublicRequestId public_request_id;
        bool reidentification;
        try {
            public_request_id = Fred::GetActivePublicRequest(
                Fred::MojeID::PublicRequest::ContactIdentification())
                .exec(ctx, locked_contact, _log_request_id);
            reidentification = false;
        }
        catch (const Fred::GetActivePublicRequest::Exception &e) {
            if (!e.is_set_no_request_found()) {
                throw;
            }
            try {
                public_request_id = Fred::GetActivePublicRequest(
                    Fred::MojeID::PublicRequest::ContactReidentification())
                    .exec(ctx, locked_contact, _log_request_id);
                reidentification = true;
            }
            catch (const Fred::GetActivePublicRequest::Exception &e) {
                if (e.is_set_no_request_found()) {
                    throw MojeIDImplData::IdentificationRequestDoesntExist();
                }
                throw;
            }
        }
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (states.absents(Fred::Object::State::mojeid_contact)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        if (states.absents(Fred::Object::State::conditionally_identified_contact)) {
            throw std::runtime_error("state conditionallyIdentifiedContact missing");
        }
        if (states.presents(Fred::Object::State::identified_contact)) {
            throw MojeIDImplData::IdentificationAlreadyProcessed();
        }
        if (states.presents(Fred::Object::State::server_blocked)) {
            throw MojeIDImplData::ObjectAdminBlocked();
        }
        if (states.absents(Fred::Object::State::server_transfer_prohibited) ||
            states.absents(Fred::Object::State::server_update_prohibited)   ||
            states.absents(Fred::Object::State::server_delete_prohibited))
        {
            throw std::runtime_error("contact not protected against changes");
        }

        Fred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
        if (!Fred::PublicRequestAuthInfo(ctx, locked_request).check_password(_password)) {
            throw MojeIDImplData::IdentificationFailed();
        }
        Fred::StatusList to_set;
        to_set.insert(Conversion::Enums::to_db_handle(Fred::Object::State::identified_contact));
        Fred::CreateObjectStateRequestId(_contact_id, to_set).exec(ctx);
        Fred::PerformObjectStateRequest(_contact_id).exec(ctx);
        answer(locked_request,
               reidentification ? Fred::MojeID::PublicRequest::ContactReidentification().iface()
                                : Fred::MojeID::PublicRequest::ContactIdentification().iface(),
               "successfully processed",
               _log_request_id);
        ctx.commit_transaction();
    }
    catch (const MojeIDImplData::IdentificationRequestDoesntExist&) {
        LOGGER(PACKAGE).info("request failed (IdentificationRequestDoesntExist)");
        throw;
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        LOGGER(PACKAGE).info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIDImplData::IdentificationAlreadyProcessed&) {
        LOGGER(PACKAGE).info("request failed (IdentificationAlreadyProcessed)");
        throw;
    }
    catch (const MojeIDImplData::IdentificationFailed&) {
        LOGGER(PACKAGE).warning("request failed (IdentificationFailed)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeIDImpl::commit_prepared_transaction(const std::string &_trans_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::commit_transaction(_trans_id);
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }

    try {
        const ContactId contact_id = prepare_transaction_storage()->get(_trans_id);
        Fred::OperationContextCreator ctx;
        Fred::PerformObjectStateRequest(contact_id).exec(ctx);
        ctx.commit_transaction();
        prepare_transaction_storage()->release(_trans_id);
    }
    catch (const prepare_transaction_data_not_found&) {
        LOGGER(PACKAGE).info("no saved transaction data for " + _trans_id + " identifier)");
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }

    try {
        const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >();
        if (server_conf_ptr->auto_sms_generation) {
            this->generate_sms_messages();
        }
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }

    try {
        const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >();
        if (server_conf_ptr->auto_email_generation) {
            this->generate_email_messages();
        }
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeIDImpl::rollback_prepared_transaction(const std::string &_trans_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::rollback_transaction(_trans_id);
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
    try {
        prepare_transaction_storage()->release(_trans_id);
    }
    catch (const prepare_transaction_data_not_found&) {
        LOGGER(PACKAGE).info("no saved transaction data for " + _trans_id + " identifier)");
    }
}

namespace {

std::string birthdate_into_czech_date(const std::string &_birthdate)
{
    const boost::gregorian::date d = birthdate_from_string_to_date(_birthdate);
    std::ostringstream out;
    if (!d.is_special()) {
        const boost::gregorian::date::ymd_type ymd = d.year_month_day();
        out << std::setw(2) << std::setfill('0') << std::right << ymd.day.as_number() << "."   //dd.
            << std::setw(2) << std::setfill('0') << std::right << ymd.month.as_number() << "." //dd.mm.
            << std::setw(0)                                    << ymd.year;                    //dd.mm.yyyy
    }
    return out.str();
}

}

MojeIDImplData::Buffer MojeIDImpl::get_validation_pdf(ContactId _contact_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;

        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);

        Database::Result res = ctx.get_conn().exec_params(
            "SELECT pr.id,c.name,c.organization,c.ssn,"
                   "(SELECT type FROM enum_ssntype WHERE id=c.ssntype),"
                   "CONCAT_WS(', ',"
                       "NULLIF(BTRIM(c.street1),''),NULLIF(BTRIM(c.street2),''),NULLIF(BTRIM(c.street3),''),"
                       "BTRIM(NULLIF(BTRIM(c.postalcode),'')||E'\\u2007'||NULLIF(BTRIM(c.city),'')),"
                       "CASE WHEN c.country='CZ' THEN NULL "
                            "ELSE (SELECT country_cs FROM enum_country WHERE id=c.country) END),"
                   "(SELECT name FROM object_registry WHERE id=c.id) "
            "FROM public_request pr,"
                 "contact c "
            "WHERE pr.resolve_time IS NULL AND "
                  "pr.status=(SELECT id FROM enum_public_request_status WHERE name=$1::TEXT) AND "
                  "pr.request_type=(SELECT id FROM enum_public_request_type WHERE name=$2::TEXT) AND "
                  "c.id=$3::BIGINT AND "
                  "EXISTS(SELECT 1 FROM public_request_objects_map WHERE request_id=pr.id AND object_id=c.id)",
            Database::query_param_list
                (Conversion::Enums::to_db_handle(Fred::PublicRequest::Status::active))
                (Fred::MojeID::PublicRequest::ContactValidation().get_public_request_type())
                (_contact_id));
        if (res.size() <= 0) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        const HandleRegistryArgs *const reg_conf =
            CfgArgs::instance()->get_handler_ptr_by_type< HandleRegistryArgs >();
        HandleCorbaNameServiceArgs *const cn_conf =
            CfgArgs::instance()->get_handler_ptr_by_type< HandleCorbaNameServiceArgs >();
        std::auto_ptr< Fred::Document::Manager > doc_manager(
            Fred::Document::Manager::create(
                reg_conf->docgen_path,
                reg_conf->docgen_template_path,
                reg_conf->fileclient_path,
                //doc_manager config dependence
                cn_conf->get_nameservice_host_port()));
        const std::string czech_language = "cs";
        std::ostringstream pdf_document;
        std::auto_ptr< Fred::Document::Generator > doc_gen(
            doc_manager->createOutputGenerator(Fred::Document::GT_CONTACT_VALIDATION_REQUEST_PIN3,
                                               pdf_document,
                                               czech_language));
        const std::string request_id   = static_cast< std::string >(res[0][0]);
        const std::string name         = static_cast< std::string >(res[0][1]);
        const std::string organization = static_cast< std::string >(res[0][2]);
        const std::string ssn_value    = static_cast< std::string >(res[0][3]);
        const Fred::SSNType::Enum ssn_type = Conversion::Enums::from_db_handle< Fred::SSNType >(
                                         static_cast< std::string >(res[0][4]));
        const std::string address      = static_cast< std::string >(res[0][5]);
        const std::string handle       = static_cast< std::string >(res[0][6]);
        const bool is_ssn_ico =      ssn_type == Fred::SSNType::ico;
        const std::string birthday = ssn_type == Fred::SSNType::birthday
            ? birthdate_into_czech_date(ssn_value)
            : "";
        std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");
        Util::XmlTagPair("mojeid_valid", Util::vector_of< Util::XmlCallback >
            (Util::XmlTagPair("request_id",   Util::XmlUnparsedCData(request_id)))
            (Util::XmlTagPair("handle",       Util::XmlUnparsedCData(handle)))
            (Util::XmlTagPair("name",         Util::XmlUnparsedCData(name)))
            (Util::XmlTagPair("organization", Util::XmlUnparsedCData(organization)))
            (Util::XmlTagPair("ic",           Util::XmlUnparsedCData(is_ssn_ico ? ssn_value : "")))
            (Util::XmlTagPair("birth_date",   Util::XmlUnparsedCData(birthday)))
            (Util::XmlTagPair("address",      Util::XmlUnparsedCData(address)))
        )(letter_xml);

        doc_gen->getInput() << letter_xml;
        doc_gen->closeInput();
        MojeIDImplData::Buffer content;
        content.value = pdf_document.str();
        return content;
    }
    catch (const Fred::PublicRequestsOfObjectLockGuardByObjectId::Exception &e) {
        if (e.is_set_object_doesnt_exist()) {
            LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        LOGGER(PACKAGE).warning("request doesn't exist (ObjectDoesntExist)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}//MojeIDImpl::get_validation_pdf

void MojeIDImpl::create_validation_request(
        ContactId _contact_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        try {
            Fred::GetActivePublicRequest(Fred::MojeID::PublicRequest::ContactValidation())
                .exec(ctx, locked_contact, _log_request_id);
            throw MojeIDImplData::ValidationRequestExists();
        }
        catch (const Fred::GetActivePublicRequest::Exception &e) {
            if (!e.is_set_no_request_found()) {
                throw;
            }
        }
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (states.presents(Fred::Object::State::validated_contact)) {
            throw MojeIDImplData::ValidationAlreadyProcessed();
        }
        if (states.absents(Fred::Object::State::mojeid_contact)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        const Fred::InfoContactData contact_data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        {
            const MojeIDImplInternal::CheckCreateValidationRequest check_create_validation_request(contact_data);
            if (!check_create_validation_request.success()) {
                MojeIDImplInternal::raise(check_create_validation_request);
            }
        }
        Fred::CreatePublicRequest().set_registrar_id(mojeid_registrar_id_)
                                   .exec(locked_contact,
                                         Fred::MojeID::PublicRequest::ContactValidation().iface(),
                                         get_optional_log_request_id(_log_request_id));
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::PublicRequestsOfObjectLockGuardByObjectId::Exception &e) {
        if (e.is_set_object_doesnt_exist()) {
            LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        LOGGER(PACKAGE).warning("contact doesn't exist (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIDImplData::ValidationRequestExists&) {
        LOGGER(PACKAGE).warning("unable to create new request (ValidationRequestExists)");
        throw;
    }
    catch (const MojeIDImplData::ValidationAlreadyProcessed&) {
        LOGGER(PACKAGE).warning("contact already validated (ValidationAlreadyProcessed)");
        throw;
    }
    catch (const MojeIDImplData::CreateValidationRequestValidationResult&) {
        LOGGER(PACKAGE).warning("request failed (CreateValidationRequestValidationResult)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

namespace {

typedef bool IsNotNull;

IsNotNull add_state(const Database::Value &_valid_from, Fred::Object::State::Enum _state,
                    Registry::MojeIDImplData::ContactStateInfo &_data)
{
    if (_valid_from.isnull()) {
        return false;
    }
    const std::string db_timestamp = static_cast< std::string >(_valid_from);//2014-12-11 09:28:45.741828
    boost::posix_time::ptime valid_from = boost::posix_time::time_from_string(db_timestamp);
    switch (_state) {
        case Fred::Object::State::identified_contact:
            _data.identification_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        case Fred::Object::State::validated_contact:
            _data.validation_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        case Fred::Object::State::mojeid_contact:
            _data.mojeid_activation_datetime = valid_from;
            break;
        case Fred::Object::State::linked:
            _data.linked_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        default:
            break;
    }
    return true;
}

}

void MojeIDImpl::get_contacts_state_changes(
    unsigned long _last_hours,
    MojeIDImplData::ContactStateInfoList &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        Database::query_param_list params(mojeid_registrar_handle_);                                   //$1::TEXT
        params(_last_hours);                                                                           //$2::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::conditionally_identified_contact));//$3::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::identified_contact));              //$4::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::validated_contact));               //$5::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::mojeid_contact));                  //$6::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::linked));                          //$7::TEXT
        const Database::Result rcontacts = ctx.get_conn().exec_params(// observe interval <now - last_hours, now)
            "WITH cic AS (SELECT id FROM enum_object_states WHERE name=$3::TEXT),"
                  "ic AS (SELECT id FROM enum_object_states WHERE name=$4::TEXT),"
                  "vc AS (SELECT id FROM enum_object_states WHERE name=$5::TEXT),"
                  "mc AS (SELECT id FROM enum_object_states WHERE name=$6::TEXT),"
                  "lc AS (SELECT id FROM enum_object_states WHERE name=$7::TEXT),"
                 "obs AS (SELECT id FROM enum_object_states WHERE name IN "//observed states
                                                                 "($3::TEXT,$4::TEXT,$5::TEXT,$6::TEXT,$7::TEXT)),"
                  "cc AS (SELECT DISTINCT c.id "//contacts whose observed states start
                         "FROM contact c "      //or stop in the course of $2 hours
                         "JOIN object_state os ON os.object_id=c.id "
                         "JOIN obs ON os.state_id=obs.id "
                         "WHERE (NOW()-($2::TEXT||'HOUR')::INTERVAL)<=os.valid_from OR (os.valid_to IS NOT NULL AND "
                               "(NOW()-($2::TEXT||'HOUR')::INTERVAL)<=os.valid_to)) "
            "SELECT cc.id,"                                                           // [0] - contact id
                   "(SELECT valid_from FROM object_state JOIN cic ON state_id=cic.id "// [1] - cic from
                    "WHERE object_id=cc.id AND valid_to IS NULL),"
                   "(SELECT valid_from FROM object_state JOIN ic ON state_id=ic.id "  // [2] - ic from
                    "WHERE object_id=cc.id AND valid_to IS NULL),"
                   "(SELECT valid_from FROM object_state JOIN vc ON state_id=vc.id "  // [3] - vc from
                    "WHERE object_id=cc.id AND valid_to IS NULL),"
                   "(SELECT valid_from FROM object_state JOIN mc ON state_id=mc.id "  // [4] - mc from
                    "WHERE object_id=cc.id AND valid_to IS NULL), "
                   "(SELECT valid_from FROM object_state JOIN lc ON state_id=lc.id "  // [5] - lc from
                    "WHERE object_id=cc.id AND valid_to IS NULL) "
            "FROM cc "
            "JOIN object_state os ON os.object_id=cc.id AND os.valid_to IS NULL "
            "JOIN mc ON mc.id=os.state_id "
            "JOIN object o ON o.id=cc.id "
            "JOIN registrar r ON r.id=o.clid AND r.handle=$1::TEXT", params);

        _result.clear();
        _result.reserve(rcontacts.size());
        for (::size_t idx = 0; idx < rcontacts.size(); ++idx) {
            Registry::MojeIDImplData::ContactStateInfo data;
            data.contact_id = static_cast< ContactId >(rcontacts[idx][0]);
            if (!add_state(rcontacts[idx][1], Fred::Object::State::conditionally_identified_contact, data)) {
                std::ostringstream msg;
                msg << "contact " << data.contact_id << " hasn't "
                    << Conversion::Enums::to_db_handle(Fred::Object::State::conditionally_identified_contact) << " state";
                LOGGER(PACKAGE).error(msg.str());
                continue;
            }
            add_state(rcontacts[idx][2], Fred::Object::State::identified_contact, data);
            add_state(rcontacts[idx][3], Fred::Object::State::validated_contact, data);
            if (!add_state(rcontacts[idx][4], Fred::Object::State::mojeid_contact, data)) {
                std::ostringstream msg;
                msg << "contact " << data.contact_id << " doesn't have "
                    << Conversion::Enums::to_db_handle(Fred::Object::State::mojeid_contact) << " state";
                throw std::runtime_error(msg.str());
            }
            add_state(rcontacts[idx][5], Fred::Object::State::linked, data);
            _result.push_back(data);
        }
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeIDImpl::get_contact_state(
    ContactId _contact_id,
    MojeIDImplData::ContactStateInfo &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        Database::query_param_list params(mojeid_registrar_handle_);                                   //$1::TEXT
        params(_contact_id);                                                                           //$2::BIGINT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::mojeid_contact));                  //$3::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::conditionally_identified_contact));//$4::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::identified_contact));              //$5::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::validated_contact));               //$6::TEXT
        params(Conversion::Enums::to_db_handle(Fred::Object::State::linked));                          //$7::TEXT
        const Database::Result rcontact = ctx.get_conn().exec_params(
            "SELECT r.id IS NULL,"                         // 0
                   "(SELECT valid_from FROM object_state " // 1
                    "WHERE object_id=o.id AND valid_to IS NULL AND "
                          "state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT)),"
                   "(SELECT valid_from FROM object_state " // 2
                    "WHERE object_id=o.id AND valid_to IS NULL AND "
                          "state_id=(SELECT id FROM enum_object_states WHERE name=$4::TEXT)),"
                   "(SELECT valid_from FROM object_state " // 3
                    "WHERE object_id=o.id AND valid_to IS NULL AND "
                          "state_id=(SELECT id FROM enum_object_states WHERE name=$5::TEXT)),"
                   "(SELECT valid_from FROM object_state " // 4
                    "WHERE object_id=o.id AND valid_to IS NULL AND "
                          "state_id=(SELECT id FROM enum_object_states WHERE name=$6::TEXT)),"
                   "(SELECT valid_from FROM object_state " // 5
                    "WHERE object_id=o.id AND valid_to IS NULL AND "
                          "state_id=(SELECT id FROM enum_object_states WHERE name=$7::TEXT)) "
            "FROM contact c "
            "JOIN object o ON o.id=c.id "
            "LEFT JOIN registrar r ON r.id=o.clid AND r.handle=$1::TEXT "
            "WHERE c.id=$2::BIGINT", params);

        if (rcontact.size() == 0) {
            throw MojeIDImplData::ObjectDoesntExist();
        }

        if (1 < rcontact.size()) {
            std::ostringstream msg;
            msg << "contact " << _contact_id << " returns multiple (" << rcontact.size() << ") records";
            throw std::runtime_error(msg.str());
        }

        if (static_cast< bool >(rcontact[0][0])) { // contact's registrar missing
            throw MojeIDImplData::ObjectDoesntExist();
        }

        _result.contact_id = _contact_id;
        if (!add_state(rcontact[0][1], Fred::Object::State::mojeid_contact, _result)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        if (!add_state(rcontact[0][2], Fred::Object::State::conditionally_identified_contact, _result)) {
            std::ostringstream msg;
            msg << "contact " << _contact_id << " doesn't have "
                << Conversion::Enums::to_db_handle(Fred::Object::State::conditionally_identified_contact) << " state";
            throw std::runtime_error(msg.str());
        }
        add_state(rcontact[0][3], Fred::Object::State::identified_contact, _result);
        add_state(rcontact[0][4], Fred::Object::State::validated_contact, _result);
        add_state(rcontact[0][5], Fred::Object::State::linked, _result);
    }//try
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        LOGGER(PACKAGE).warning("ObjectDoesntExist");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeIDImpl::cancel_account_prepare(
        ContactId _contact_id,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));

        if (states.absents(Fred::Object::State::mojeid_contact)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }

        if (!(states.presents(Fred::Object::State::validated_contact)  ||
              states.presents(Fred::Object::State::identified_contact) ||
              states.presents(Fred::Object::State::conditionally_identified_contact))
           ) {
            throw std::runtime_error("bad mojeID contact");
        }

        if (states.absents(Fred::Object::State::linked)) {
            Fred::DeleteContactById(_contact_id).exec(ctx);
            ctx.commit_transaction();
            return;
        }

        Fred::StatusList to_cancel;
        to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::mojeid_contact));
        if (states.presents(Fred::Object::State::server_update_prohibited)) {
            to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::server_update_prohibited));
        }
        if (states.presents(Fred::Object::State::server_transfer_prohibited)) {
            to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::server_transfer_prohibited));
        }
        if (states.presents(Fred::Object::State::server_delete_prohibited)) {
            to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::server_delete_prohibited));
        }
        if (states.presents(Fred::Object::State::validated_contact)) {
            to_cancel.insert(Conversion::Enums::to_db_handle(Fred::Object::State::validated_contact));
        }
        Fred::CancelObjectStateRequestId(_contact_id, to_cancel).exec(ctx);

        {
            Fred::UpdateContactById update_contact_op(_contact_id, mojeid_registrar_handle_);
            update_contact_op.unset_domain_expiration_letter_flag()
                             .reset_address< Fred::ContactAddressType::MAILING >()
                             .reset_address< Fred::ContactAddressType::BILLING >()
                             .reset_address< Fred::ContactAddressType::SHIPPING >()
                             .reset_address< Fred::ContactAddressType::SHIPPING_2 >()
                             .reset_address< Fred::ContactAddressType::SHIPPING_3 >();
            if (0 < _log_request_id) {
                update_contact_op.set_logd_request_id(_log_request_id);
            }
            update_contact_op.exec(ctx);
        }

        Fred::UpdatePublicRequest().set_status(Fred::PublicRequest::Status::invalidated)
                                   .set_reason("cancel_account_prepare call")
                                   .set_registrar_id(ctx, mojeid_registrar_handle_)
                                   .exec(locked_contact,
                                         Fred::MojeID::PublicRequest::ContactValidation(),
                                         get_optional_log_request_id(_log_request_id));
        prepare_transaction_storage()->store(_trans_id, _contact_id);
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::PublicRequestsOfObjectLockGuardByObjectId::Exception &e) {
        if (e.is_set_object_doesnt_exist()) {
            LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        LOGGER(PACKAGE).warning("contact doesn't exist (ObjectDoesntExist)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeIDImpl::send_new_pin3(
    ContactId _contact_id,
    LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Fred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, _contact_id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (states.presents(Fred::Object::State::identified_contact)) {
            // nothing to send if contact is identified
            // IdentificationRequestDoesntExist isn't error in frontend
            throw MojeIDImplData::IdentificationRequestDoesntExist();
        }
        if (states.absents(Fred::Object::State::mojeid_contact)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        bool has_identification_request = false;
        try {
            const Fred::MojeID::PublicRequest::ContactIdentification type;
            Fred::GetActivePublicRequest get_active_public_request_op(type.iface());
            while (true) {
                const Fred::PublicRequestId request_id = get_active_public_request_op.exec(ctx, locked_object);
                Fred::UpdatePublicRequest update_public_request_op;
                Fred::PublicRequestLockGuardById locked_request(ctx, request_id);
                update_public_request_op.set_status(Fred::PublicRequest::Status::invalidated);
                update_public_request_op.set_reason("new pin3 generated");
                update_public_request_op.set_registrar_id(ctx, mojeid_registrar_handle_);
                update_public_request_op.exec(locked_request, type.iface(), get_optional_log_request_id(_log_request_id));
                has_identification_request = true;
            }
        }
        catch (const Fred::GetActivePublicRequest::Exception &e) {
            if (!e.is_set_no_request_found()) {
                throw;
            }
        }

        bool has_reidentification_request = false;
        try {
            const Fred::MojeID::PublicRequest::ContactReidentification type;
            Fred::GetActivePublicRequest get_active_public_request_op(type.iface());
            while (true) {
                const Fred::PublicRequestId request_id = get_active_public_request_op.exec(ctx, locked_object);
                Fred::UpdatePublicRequest update_public_request_op;
                Fred::PublicRequestLockGuardById locked_request(ctx, request_id);
                update_public_request_op.set_status(Fred::PublicRequest::Status::invalidated);
                update_public_request_op.set_reason("new pin3 generated");
                update_public_request_op.set_registrar_id(ctx, mojeid_registrar_handle_);
                update_public_request_op.exec(locked_request, type.iface(), get_optional_log_request_id(_log_request_id));
                has_reidentification_request = true;
            }
        }
        catch (const Fred::GetActivePublicRequest::Exception &e) {
            if (!e.is_set_no_request_found()) {
                throw;
            }
        }

        if (!has_identification_request && !has_reidentification_request) {
            throw MojeIDImplData::IdentificationRequestDoesntExist();
        }

        const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >();
        check_sent_letters_limit(ctx,
                                 _contact_id,
                                 server_conf_ptr->letter_limit_count,
                                 server_conf_ptr->letter_limit_interval);

        Fred::CreatePublicRequestAuth create_public_request_op;
        create_public_request_op.set_registrar_id(mojeid_registrar_id_);
        create_public_request_op.set_reason("send_new_pin3 call");
        const Fred::CreatePublicRequestAuth::Result result =
            create_public_request_op.exec(
                locked_object,
                has_reidentification_request
                ? Fred::MojeID::PublicRequest::ContactReidentification().iface()
                : Fred::MojeID::PublicRequest::ContactIdentification().iface(),
                get_optional_log_request_id(_log_request_id));
        ctx.commit_transaction();
        return;
    }
    catch(const MojeIDImplData::ObjectDoesntExist &e) {
        LOGGER(PACKAGE).info("ObjectDoesntExist");
        throw;
    }
    catch(const MojeIDImplData::MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.as_string());
        throw;
    }
    catch(const Fred::PublicRequestsOfObjectLockGuardByObjectId::Exception &e) {
        if (e.is_set_object_doesnt_exist()) {
            LOGGER(PACKAGE).info(e.what());
            throw MojeIDImplData::ObjectDoesntExist();
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch(const MojeIDImplData::IdentificationRequestDoesntExist&) {
        LOGGER(PACKAGE).info("IdentificationRequestDoesntExist");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

void MojeIDImpl::send_mojeid_card(
    ContactId _contact_id,
    LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (states.absents(Fred::Object::State::mojeid_contact)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->
                                                            get_handler_ptr_by_type< HandleMojeIDArgs >();
        const Fred::InfoContactData data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        const Fred::Messages::ManagerPtr manager_ptr = Fred::Messages::create_manager();
        MojeIDImpl::send_mojeid_card(
            ctx,
            manager_ptr.get(),
            data,
            server_conf_ptr->letter_limit_count,
            server_conf_ptr->letter_limit_interval,
            _log_request_id,
            Optional< boost::posix_time::ptime >(),
            states.presents(Fred::Object::State::validated_contact));
        ctx.commit_transaction();
    }
    catch(const MojeIDImplData::ObjectDoesntExist&) {
        LOGGER(PACKAGE).info("ObjectDoesntExist");
        throw;
    }
    catch(const MojeIDImplData::MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.as_string());
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

void MojeIDImpl::generate_sms_messages()const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        typedef ::MojeID::Messages::CommChannel CommChannel;
        ::MojeID::Messages::DefaultMultimanager multimanager;
        ::MojeID::Messages::Generate::Into< CommChannel::sms >::for_new_requests(ctx, multimanager);
        ctx.commit_transaction();
        return;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

void MojeIDImpl::enable_sms_messages_generation(bool enable)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        ::MojeID::Messages::Generate::enable< ::MojeID::Messages::CommChannel::sms >(ctx, enable);
        ctx.commit_transaction();
        return;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

void MojeIDImpl::generate_letter_messages()const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        typedef ::MojeID::Messages::CommChannel CommChannel;
        ::MojeID::Messages::DefaultMultimanager multimanager;
        ::MojeID::Messages::Generate::Into< CommChannel::letter >::for_new_requests(
            ctx, multimanager, check_limits::sent_letters());
        ctx.commit_transaction();
        return;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

void MojeIDImpl::enable_letter_messages_generation(bool enable)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        ::MojeID::Messages::Generate::enable< ::MojeID::Messages::CommChannel::letter >(ctx, enable);
        ctx.commit_transaction();
        return;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

void MojeIDImpl::generate_email_messages()const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        typedef ::MojeID::Messages::CommChannel CommChannel;
        const std::string link_hostname_part = CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->hostname;
        ::MojeID::Messages::DefaultMultimanager multimanager;
        ::MojeID::Messages::Generate::Into< CommChannel::email >::for_new_requests(
            ctx,
            multimanager,
            ::MojeID::Messages::Generate::message_checker_always_success(),
            link_hostname_part);
        ctx.commit_transaction();
        return;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

void MojeIDImpl::enable_email_messages_generation(bool enable)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        ::MojeID::Messages::Generate::enable< ::MojeID::Messages::CommChannel::email >(ctx, enable);
        ctx.commit_transaction();
        return;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

MojeIDImpl::MessageId MojeIDImpl::send_mojeid_card(
    Fred::OperationContext &_ctx,
    Fred::Messages::Manager *_msg_manager_ptr,
    const Fred::InfoContactData &_data,
    unsigned _limit_count,
    unsigned _limit_interval,
    LogRequestId _log_request_id,
    const Optional< boost::posix_time::ptime > &_letter_time,
    const Optional< bool > &_validated_contact)
{
    cancel_message_sending< MessageType::mojeid_card, CommType::letter >(_ctx, _data.id);
    check_sent_letters_limit(_ctx, _data.id, _limit_count, _limit_interval);
    std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

    const std::string name = _data.name.get_value_or_default();
    const std::string::size_type name_delimiter_pos = name.find_last_of(' ');
    const std::string firstname = name_delimiter_pos != std::string::npos
                                  ? name.substr(0, name_delimiter_pos)
                                  : name;
    const std::string lastname = name_delimiter_pos != std::string::npos
                                 ? name.substr(name_delimiter_pos + 1)
                                 : std::string();
    static const char female_suffix[] = "á"; // utf-8 encoded
    enum { FEMALE_SUFFIX_LEN = sizeof(female_suffix) - 1,
           STR_EQUAL = 0 };
    const std::string sex = (FEMALE_SUFFIX_LEN <= name.length()) &&
                            (std::strcmp(name.c_str() + name.length() - FEMALE_SUFFIX_LEN, female_suffix) == STR_EQUAL)
                            ? "female"
                            : "male";

    const Fred::InfoContactData::Address addr = _data.get_address< Fred::ContactAddressType::MAILING >();
    Fred::Messages::PostalAddress pa;
    pa.name    = name;
    pa.org     = _data.organization.get_value_or_default();
    pa.street1 = addr.street1;
    pa.city    = addr.city;
    pa.state   = addr.stateorprovince.get_value_or_default();
    pa.code    = addr.postalcode;
    pa.country = addr.country;

    Database::query_param_list params(pa.country);
    std::string sql = "SELECT (SELECT country_cs FROM enum_country WHERE id=$1::TEXT OR country=$1::TEXT),"
                             "(SELECT country FROM enum_country WHERE id=$1::TEXT)";
    if (!_validated_contact.isset()) {
        params(_data.id)
              (Conversion::Enums::to_db_handle(Fred::Object::State::validated_contact));
        sql.append(",EXISTS(SELECT * FROM object_state "
                           "WHERE object_id=$2::BIGINT AND "
                                 "state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                 "valid_to IS NULL)");
    }
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    const std::string addr_country = dbres[0][0].isnull()
                                     ? pa.country
                                     : static_cast< std::string >(dbres[0][0]);
    if (!dbres[0][1].isnull()) {
        pa.country = static_cast< std::string >(dbres[0][1]);
    }
    const std::string contact_handle = _data.handle;
    const boost::gregorian::date letter_date = _letter_time.isset()
                                               ? _letter_time.get_value().date()
                                               : boost::gregorian::day_clock::local_day();
    const std::string contact_state = (_validated_contact.isset() && _validated_contact.get_value()) ||
                                      (!_validated_contact.isset() && static_cast< bool >(dbres[0][2]))
                                      ? "validated"
                                      : "";

    Util::XmlTagPair("contact_auth", Util::vector_of<Util::XmlCallback>
        (Util::XmlTagPair("user", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("actual_date", Util::XmlUnparsedCData(boost::gregorian::to_iso_extended_string(letter_date))))
            (Util::XmlTagPair("name", Util::XmlUnparsedCData(pa.name)))
            (Util::XmlTagPair("organization", Util::XmlUnparsedCData(pa.org)))
            (Util::XmlTagPair("street", Util::XmlUnparsedCData(pa.street1)))
            (Util::XmlTagPair("city", Util::XmlUnparsedCData(pa.city)))
            (Util::XmlTagPair("stateorprovince", Util::XmlUnparsedCData(pa.state)))
            (Util::XmlTagPair("postal_code", Util::XmlUnparsedCData(pa.code)))
            (Util::XmlTagPair("country", Util::XmlUnparsedCData(addr_country)))
            (Util::XmlTagPair("account", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("username", Util::XmlUnparsedCData(contact_handle)))
                (Util::XmlTagPair("first_name", Util::XmlUnparsedCData(firstname)))
                (Util::XmlTagPair("last_name", Util::XmlUnparsedCData(lastname)))
                (Util::XmlTagPair("sex", Util::XmlUnparsedCData(sex)))
                (Util::XmlTagPair("email", Util::XmlUnparsedCData(_data.email.get_value_or_default())))
                (Util::XmlTagPair("mobile", Util::XmlUnparsedCData(_data.telephone.get_value_or_default())))
                (Util::XmlTagPair("state", Util::XmlUnparsedCData(contact_state)))
            ))
        ))
    )(letter_xml);

    std::stringstream xmldata;
    xmldata << letter_xml;

    const HandleRegistryArgs *const rconf =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleRegistryArgs >();
    std::auto_ptr< Fred::Document::Manager > doc_manager_ptr =
        Fred::Document::Manager::create(
            rconf->docgen_path,
            rconf->docgen_template_path,
            rconf->fileclient_path,
            CfgArgs::instance()->get_handler_ptr_by_type< HandleCorbaNameServiceArgs >()
                ->get_nameservice_host_port());
    enum { FILETYPE_MOJEID_CARD = 10 };
    const unsigned long long file_id = doc_manager_ptr->generateDocumentAndSave(
        Fred::Document::GT_MOJEID_CARD,
        xmldata,
        "mojeid_card-" + boost::lexical_cast< std::string >(_data.id) + "-" +
                         boost::lexical_cast< std::string >(::time(NULL)) + ".pdf",
        FILETYPE_MOJEID_CARD, "");

    static const char *const comm_type = "letter";
    static const char *const message_type = "mojeid_card";
    const MessageId message_id =
        _msg_manager_ptr->save_letter_to_send(
            contact_handle.c_str(),
            pa,
            file_id,
            message_type,
            _data.id,
            _data.historyid,
            comm_type,
            true);
    return message_id;
}

}//namespace Registry::MojeID
}//namespace Registry
