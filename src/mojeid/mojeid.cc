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
#include "src/fredlib/notifier2/enqueue_notification.h"
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
    try {
        const std::string handle =
            CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->registrar_handle;
        if (!handle.empty()) {
            return handle;
        }
    }
    catch (...) {
    }
    throw std::runtime_error("missing configuration for dedicated registrar");
}

::size_t get_mojeid_registrar_id(const std::string &registrar_handle)
{
    try {
        Fred::OperationContextCreator ctx;
        Database::Result dbres = ctx.get_conn().exec_params(
            "SELECT id FROM registrar WHERE handle=$1::TEXT", Database::query_param_list(registrar_handle));
        if (0 < dbres.size()) {
            ctx.commit_transaction();
            return static_cast< ::size_t >(dbres[0][0]);
        }
    }
    catch (...) {
    }
    throw std::runtime_error("missing dedicated registrar");
}

class set_ssn
{
public:
    template < typename T >
    set_ssn(const Nullable< T > &_ssn, Fred::SSNType::Value _ssn_type, Fred::CreateContact &_out)
    :   out_ptr(&_out)
    {
        this->operator()(_ssn, _ssn_type);
    }
    set_ssn& operator()(const Nullable< std::string > &_ssn, Fred::SSNType::Value _ssn_type)
    {
        if ((out_ptr != NULL) && !_ssn.isnull()) {
            out_ptr->set_ssntype(Conversion::Enums::into< std::string >(_ssn_type));
            out_ptr->set_ssn(_ssn.get_value());
            out_ptr = NULL;
        }
        return *this;
    }
    typedef boost::gregorian::date Date;
    set_ssn& operator()(const Nullable< Date > &_ssn, Fred::SSNType::Value _ssn_type)
    {
        if ((out_ptr != NULL) && !_ssn.isnull()) {
            out_ptr->set_ssntype(Conversion::Enums::into< std::string >(_ssn_type));
            out_ptr->set_ssn(boost::gregorian::to_iso_extended_string(_ssn.get_value()));
            out_ptr = NULL;
        }
        return *this;
    }
private:
    Fred::CreateContact *out_ptr;
};

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

    (_contact.organization.isnull()
     ? set_ssn(_contact.birth_date,   Fred::SSNType::BIRTHDAY, _arguments) //person
     : set_ssn(_contact.vat_id_num,   Fred::SSNType::ICO,      _arguments))//company
              (_contact.id_card_num,  Fred::SSNType::OP)
              (_contact.passport_num, Fred::SSNType::PASS)
              (_contact.ssn_id_num,   Fred::SSNType::MPSV);
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
        "SELECT (ma.moddate+($3::TEXT||'DAYS')::INTERVAL)::DATE "
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
        e.limit_expire_date = boost::gregorian::from_simple_string(static_cast< std::string >(result[0][0]));
        e.limit_count       = _max_sent_letters;
        e.limit_days        = _watched_period_in_days;
        throw e;
    }
}

struct check_limits
{
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
        ~sent_letters() { }
    private:
        const unsigned max_sent_letters_;
        const unsigned watched_period_in_days_;
    };
};

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
        if (_c1.ssntype.get_value_or_default() != Conversion::Enums::into< std::string >(Fred::SSNType::BIRTHDAY)) {
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
    static const bool value = CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->notify_commands;
    return value;
}

static const Fred::PublicRequestId _unused_request_id = 0;

void notify(Fred::OperationContext&        _ctx,
        const Notification::notified_event _event,
        unsigned long long                 _done_by_registrar,
        unsigned long long                 _object_historyid_post_change,
        Fred::PublicRequestId              _request_id = _unused_request_id)
{
    if (notification_enabled()) {
        Notification::enqueue_notification(_ctx, _event, _done_by_registrar, _object_historyid_post_change,
            _request_id == _unused_request_id ? std::string()
                                              : Util::make_svtrid(_request_id));
    }
}

class MessageType
{
public:
    enum Value
    {
        DOMAIN_EXPIRATION,
        MOJEID_PIN2,
        MOJEID_PIN3,
        MOJEID_SMS_CHANGE,
        MONITORING,
        CONTACT_VERIFICATION_PIN2,
        CONTACT_VERIFICATION_PIN3,
        MOJEID_PIN3_REMINDER,
        CONTACT_CHECK_NOTICE,
        CONTACT_CHECK_THANK_YOU,
        MOJEID_CARD
    };
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into_from< Value >::into_enum_from(_str);
    }
};

class CommType
{
public:
    enum Value
    {
        EMAIL,
        LETTER,
        SMS,
        REGISTERED_LETTER
    };
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into_from< Value >::into_enum_from(_str);
    }
};

class SendStatus
{
public:
    enum Value
    {
        READY,
        WAITING_CONFIRMATION,
        NO_PROCESSING,
        SEND_FAILED,
        SENT,
        BEING_SENT,
        UNDELIVERED
    };
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into_from< Value >::into_enum_from(_str);
    }
};

struct PubReqType
{
    enum Value
    {
        CONTACT_CONDITIONAL_IDENTIFICATION,
        CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER,
        IDENTIFIED_CONTACT_TRANSFER,
    };
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into_from< Value >::into_enum_from(_str);
    }
};

}//Registry::MojeID::{anonymous}
}//Registry::MojeID
}//Registry

namespace Conversion {
namespace Enums {

template < >
struct tools_for< Registry::MojeID::MessageType::Value >
{
    static void enum_to_other_init(void (*set_relation)(Registry::MojeID::MessageType::Value, const std::string&))
    {
        using Registry::MojeID::MessageType;
        set_relation(MessageType::DOMAIN_EXPIRATION,         "domain_expiration");
        set_relation(MessageType::MOJEID_PIN2,               "mojeid_pin2");
        set_relation(MessageType::MOJEID_PIN3,               "mojeid_pin3");
        set_relation(MessageType::MOJEID_SMS_CHANGE,         "mojeid_sms_change");
        set_relation(MessageType::MONITORING,                "monitoring");
        set_relation(MessageType::CONTACT_VERIFICATION_PIN2, "contact_verification_pin2");
        set_relation(MessageType::CONTACT_VERIFICATION_PIN3, "contact_verification_pin3");
        set_relation(MessageType::MOJEID_PIN3_REMINDER,      "mojeid_pin3_reminder");
        set_relation(MessageType::CONTACT_CHECK_NOTICE,      "contact_check_notice");
        set_relation(MessageType::CONTACT_CHECK_THANK_YOU,   "contact_check_thank_you");
        set_relation(MessageType::MOJEID_CARD,               "mojeid_card");
    }
};

template < >
struct tools_for< Registry::MojeID::CommType::Value >
{
    static void enum_to_other_init(void (*set_relation)(Registry::MojeID::CommType::Value, const std::string&))
    {
        using Registry::MojeID::CommType;
        set_relation(CommType::EMAIL,             "email");
        set_relation(CommType::LETTER,            "letter");
        set_relation(CommType::SMS,               "sms");
        set_relation(CommType::REGISTERED_LETTER, "registered_letter");
    }
};

template < >
struct tools_for< Registry::MojeID::SendStatus::Value >
{
    static void enum_to_other_init(void (*set_relation)(Registry::MojeID::SendStatus::Value, const std::string&))
    {
        using Registry::MojeID::SendStatus;
        set_relation(SendStatus::READY,                "ready");
        set_relation(SendStatus::WAITING_CONFIRMATION, "waiting_confirmation");
        set_relation(SendStatus::NO_PROCESSING,        "no_processing");
        set_relation(SendStatus::SEND_FAILED,          "send_failed");
        set_relation(SendStatus::SENT,                 "sent");
        set_relation(SendStatus::BEING_SENT,           "being_sent");
        set_relation(SendStatus::UNDELIVERED,          "undelivered");
    }
};

template < >
struct tools_for< Registry::MojeID::PubReqType::Value >
{
    static void enum_to_other_init(void (*set_relation)(Registry::MojeID::PubReqType::Value, const std::string&))
    {
        typedef Registry::MojeID::PubReqType ET;
        using namespace Fred::MojeID::PublicRequest;
        set_relation(ET::CONTACT_CONDITIONAL_IDENTIFICATION,        as_string< ContactConditionalIdentification >());
        set_relation(ET::CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER, as_string< ConditionallyIdentifiedContactTransfer >());
        set_relation(ET::IDENTIFIED_CONTACT_TRANSFER,               as_string< IdentifiedContactTransfer >());
    }
private:
    template < class PUB_REQ >
    static std::string as_string()
    {
        return PUB_REQ::iface().get_public_request_type();
    }
};

}//namespace Conversion::Enums
}//namespace Conversion

namespace Registry {
namespace MojeID {

namespace {

template < MessageType::Value MT, CommType::Value CT >
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
        Database::query_param_list(_contact_id)                                                      //$1::BIGINT
                                  (Conversion::Enums::into< std::string >(CT))                       //$2::TEXT
                                  (Conversion::Enums::into< std::string >(MT))                       //$3::TEXT
                                  (Conversion::Enums::into< std::string >(SendStatus::NO_PROCESSING))//$4::TEXT
                                  (Conversion::Enums::into< std::string >(SendStatus::SEND_FAILED))  //$5::TEXT
                                  (Conversion::Enums::into< std::string >(SendStatus::READY)));      //$6::TEXT
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

    if (differs(_c1.ssntype, _c2.ssntype)) {
        return true;
    }

    if (differs(_c1.ssn, _c2.ssn)) {
        if (_c1.ssntype.get_value_or_default() != Conversion::Enums::into< std::string >(Fred::SSNType::BIRTHDAY)) {
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
                throw check_contact_data;
            }
        }

        Fred::CreateContact op_create_contact(_contact.username, mojeid_registrar_handle_);
        set_create_contact_arguments(_contact, op_create_contact);
        const Fred::CreateContact::Result new_contact = op_create_contact.exec(ctx);
        Fred::CreatePublicRequestAuth op_create_pub_req(
            Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface());
        Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, new_contact.object_id);
        {
            const Fred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(ctx, locked_contact);
            _ident = result.identification;
            notify(ctx, Notification::created,
                   mojeid_registrar_id_, new_contact.history_id, result.public_request_id);
        }
        prepare_transaction_storage()->store(_trans_id, new_contact.object_id);
        ctx.commit_transaction();
        return new_contact.object_id;
    }
    catch (const MojeIDImplInternal::CheckCreateContactPrepare &e) {
        LOGGER(PACKAGE).error("request failed (incorrect input data)");
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

namespace {

Fred::CreatePublicRequestAuth::Result action_transfer_contact_prepare(
    const Fred::PublicRequestAuthTypeIface &_iface,
    const std::string &_trans_id,
    const Fred::InfoContactData &_contact,
    const Fred::PublicRequestObjectLockGuardByObjectId &_locked_contact,
    const std::string &_registrar_handle,
    Fred::OperationContextTwoPhaseCommitCreator &_ctx)
{
    Fred::CreatePublicRequestAuth op_create_pub_req(_iface);
    if (!_contact.notifyemail.isnull()) {
        op_create_pub_req.set_email_to_answer(_contact.notifyemail.get_value());
    }
    op_create_pub_req.set_registrar_id(_ctx, _registrar_handle);
    const Fred::CreatePublicRequestAuth::Result result =
        op_create_pub_req.exec(_ctx, _locked_contact);
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
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, contact.id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(contact.id).exec(ctx));
        const MojeIDImplInternal::CheckMojeIDRegistration check_result(
            Fred::make_args(contact), Fred::make_args(contact, ctx), Fred::make_args(states));
        if (!check_result.success()) {
            MojeIDImplInternal::raise(check_result);
        }

        Fred::CreatePublicRequestAuth::Result pub_req_result;
        if (states.absents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) &&
            states.absents(Fred::Object::State::IDENTIFIED_CONTACT) &&
            states.absents(Fred::Object::State::VALIDATED_CONTACT) &&
            states.absents(Fred::Object::State::MOJEID_CONTACT))
        {
            pub_req_result = action_transfer_contact_prepare(
                Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface(),
                _trans_id, contact, locked_contact, mojeid_registrar_handle_, ctx);
        }
        else if (states.presents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) &&
                 states.absents(Fred::Object::State::IDENTIFIED_CONTACT) &&
                 states.absents(Fred::Object::State::VALIDATED_CONTACT) &&
                 states.absents(Fred::Object::State::MOJEID_CONTACT))
        {
            pub_req_result = action_transfer_contact_prepare(
                Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer::iface(),
                _trans_id, contact, locked_contact, mojeid_registrar_handle_, ctx);
        }
        else if (states.presents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) &&
                 states.presents(Fred::Object::State::IDENTIFIED_CONTACT) &&
                 states.absents(Fred::Object::State::VALIDATED_CONTACT) &&
                 states.absents(Fred::Object::State::MOJEID_CONTACT))
        {
            pub_req_result = action_transfer_contact_prepare(
                Fred::MojeID::PublicRequest::IdentifiedContactTransfer::iface(),
                _trans_id, contact, locked_contact, mojeid_registrar_handle_, ctx);
        }

        if (contact.sponsoring_registrar_handle != mojeid_registrar_handle_) {
            Fred::UpdateContactById update_contact_op(contact.id, mojeid_registrar_handle_);
            update_contact_op.set_logd_request_id(_log_request_id);
            //transfer contact to 'REG-MOJEID' sponsoring registrar
            update_contact_op.set_sponsoring_registrar(mojeid_registrar_handle_);
            const unsigned long long history_id = update_contact_op.exec(ctx);
            notify(ctx, Notification::transferred,
                   mojeid_registrar_id_, history_id, pub_req_result.public_request_id);
        }
        from_into(contact, _contact);
        ctx.commit_transaction();
        _ident = pub_req_result.identification;
        return;
    }
    catch (const MojeIDImplData::AlreadyMojeidContact&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data - AlreadyMojeidContact)");
        throw;
    }
    catch (const MojeIDImplData::ObjectAdminBlocked&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data - ObjectAdminBlocked)");
        throw;
    }
    catch (const MojeIDImplData::ObjectUserBlocked&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data - ObjectUserBlocked)");
        throw;
    }
    catch (const MojeIDImplData::RegistrationValidationResult&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data - RegistrationValidationResult)");
        throw;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).error("request failed (incorrect input data)");
            throw MojeIDImplData::ObjectDoesntExist();
        }
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
    if (_data_changes.name.isset() && !_data_changes.name.get_value().second.isnull()) {
        _update_op.set_name(_data_changes.name.get_value().second.get_value());
    }
    if (_data_changes.organization.isset() && !_data_changes.organization.get_value().second.isnull()) {
        _update_op.set_organization(_data_changes.organization.get_value().second.get_value());
    }
    if (_data_changes.personal_id.isset()) {
        _update_op.set_personal_id(_data_changes.personal_id.get_value().second);
    }
    if (_data_changes.place.isset() && !_data_changes.place.get_value().second.isnull()) {
        _update_op.set_place(_data_changes.place.get_value().second.get_value());
    }
    if (_data_changes.addresses.isset()) {
        update_address< Fred::ContactAddressType::MAILING >   (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::BILLING >   (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING >  (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING_2 >(_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING_3 >(_data_changes, _update_op);
    }
    if (_data_changes.email.isset() && !_data_changes.email.get_value().second.isnull()) {
        _update_op.set_email(_data_changes.email.get_value().second.get_value());
    }
    if (_data_changes.notifyemail.isset() && !_data_changes.notifyemail.get_value().second.isnull()) {
        _update_op.set_notifyemail(_data_changes.notifyemail.get_value().second.get_value());
    }
    if (_data_changes.telephone.isset() && !_data_changes.telephone.get_value().second.isnull()) {
        _update_op.set_telephone(_data_changes.telephone.get_value().second.get_value());
    }
    if (_data_changes.fax.isset() && !_data_changes.fax.get_value().second.isnull()) {
        _update_op.set_fax(_data_changes.fax.get_value().second.get_value());
    }
}

}

void MojeIDImpl::update_contact_prepare(
        const MojeIDImplData::InfoContact &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::InfoContactData new_data;
        from_into(_new_data, new_data);
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(new_data.id).exec(ctx));
        if (states.absents(Fred::Object::State::MOJEID_CONTACT)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        const Fred::InfoContactData current_data = Fred::InfoContactById(new_data.id).exec(ctx).info_contact_data;
        const Fred::InfoContactDiff data_changes = Fred::diff_contact_data(current_data, new_data);
        if (!(data_changes.name.isset()         ||
              data_changes.organization.isset() ||
              data_changes.personal_id.isset()  ||
              data_changes.place.isset()        ||
              data_changes.addresses.isset()    ||
              data_changes.email.isset()        ||
              data_changes.notifyemail.isset()  ||
              data_changes.telephone.isset()    ||
              data_changes.fax.isset())) {
            return;
        }
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, new_data.id);
        Fred::StatusList to_cancel;
        bool drop_validation = false;
        if (states.presents(Fred::Object::State::VALIDATED_CONTACT)) {
            drop_validation = validated_data_changed(current_data, new_data);
            if (drop_validation) {
                to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::VALIDATED_CONTACT));
            }
        }
        const bool drop_identification = identified_data_changed(current_data, new_data);
        if (drop_identification || differs(current_data.email, new_data.email)) {
            cancel_message_sending< MessageType::MOJEID_CARD, CommType::LETTER >(ctx, new_data.id);
        }
        Fred::PublicRequestId request_id = _unused_request_id;
        if (drop_identification) {
            const bool reidentification_needed = states.presents(Fred::Object::State::IDENTIFIED_CONTACT);
            if (reidentification_needed) {
                to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::IDENTIFIED_CONTACT));
            }
            const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->
                                                                get_handler_ptr_by_type< HandleMojeIDArgs >();
            check_sent_letters_limit(ctx,
                                     new_data.id,
                                     server_conf_ptr->letter_limit_count,
                                     server_conf_ptr->letter_limit_interval);
            Fred::CreatePublicRequestAuth create_public_request_op(
                reidentification_needed ? Fred::MojeID::PublicRequest::ContactReidentification::iface()
                                        : Fred::MojeID::PublicRequest::ContactIdentification::iface());
            create_public_request_op.set_reason("data changed");
            create_public_request_op.set_registrar_id(ctx, mojeid_registrar_handle_);
            request_id = create_public_request_op.exec(ctx, locked_contact, _log_request_id).public_request_id;
        }
        {
            const MojeIDImplInternal::CheckUpdateContactPrepare check_contact_data(new_data);
            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
        }
        const bool manual_verification_done = states.presents(Fred::Object::State::CONTACT_FAILED_MANUAL_VERIFICATION) ||
                                              states.presents(Fred::Object::State::CONTACT_PASSED_MANUAL_VERIFICATION);
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
                if (states.presents(Fred::Object::State::CONTACT_FAILED_MANUAL_VERIFICATION)) {
                    to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::CONTACT_FAILED_MANUAL_VERIFICATION));
                }
                if (states.presents(Fred::Object::State::CONTACT_PASSED_MANUAL_VERIFICATION)) {
                    to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::CONTACT_PASSED_MANUAL_VERIFICATION));
                }
            }
        }
        const bool object_states_changed = !to_cancel.empty();
        if (object_states_changed) {
            Fred::CancelObjectStateRequestId(new_data.id, to_cancel).exec(ctx);
        }
        Fred::UpdateContactById update_contact_op(new_data.id, mojeid_registrar_handle_);
        set_update_contact_op(data_changes, update_contact_op);
        const bool is_identified = states.presents(Fred::Object::State::IDENTIFIED_CONTACT) && !drop_identification;
        const bool is_validated  = states.presents(Fred::Object::State::VALIDATED_CONTACT) && !drop_validation;
        const bool addr_can_be_hidden = (is_identified || is_validated) &&
                                        new_data.organization.get_value_or_default().empty();
        if (!addr_can_be_hidden) {
            update_contact_op.set_discloseaddress(true);
        }
        const unsigned long long history_id = update_contact_op.exec(ctx);

        notify(ctx, Notification::updated, mojeid_registrar_id_, history_id, request_id);

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
        const MojeIDImplData::SetContact &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const
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
        new_data.name   = current_data.name;
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(new_data.id).exec(ctx));
        //check contact is 'mojeidContact', if true throw ALREADY_MOJEID_CONTACT
        if (states.presents(Fred::Object::State::MOJEID_CONTACT)) {
            throw MojeIDImplData::AlreadyMojeidContact();
        }
        //throw OBJECT_ADMIN_BLOCKED if contact is administrative blocked
        if (states.presents(Fred::Object::State::SERVER_BLOCKED)) {
            throw MojeIDImplData::ObjectAdminBlocked();
        }
        //throw OBJECT_USER_BLOCKED if contact is blocked by user
        if (states.presents(Fred::Object::State::SERVER_TRANSFER_PROHIBITED) ||
            states.presents(Fred::Object::State::SERVER_UPDATE_PROHIBITED) ||
            states.presents(Fred::Object::State::SERVER_DELETE_PROHIBITED)) {
            throw MojeIDImplData::ObjectUserBlocked();
        }
        check_limits::sent_letters()(ctx, current_data.id);
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, new_data.id);
        bool drop_identification      = false;
        bool drop_cond_identification = false;
        unsigned long long history_id;
        bool do_transfer = false;
        {
            if (states.presents(Fred::Object::State::IDENTIFIED_CONTACT)) {
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
            if (states.presents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) ||
                (!drop_identification && states.presents(Fred::Object::State::IDENTIFIED_CONTACT)))
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
                if (drop_cond_identification) {
                    to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT));
                }
                //drop identified flag if name, mailing address, e-mail or mobile changed
                if (drop_identification) {
                    to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::IDENTIFIED_CONTACT));
                }
                Fred::CancelObjectStateRequestId(current_data.id, to_cancel).exec(ctx);
            }

            const MojeIDImplInternal::CheckUpdateTransferContactPrepare check_contact_data(
                Fred::make_args(new_data),
                Fred::make_args(new_data, ctx),
                Fred::make_args(states));

            if (!check_contact_data.success()) {
                MojeIDImplInternal::raise(check_contact_data);
            }
            //perform changes
            Fred::UpdateContactById update_contact_op(new_data.id, mojeid_registrar_handle_);
            set_update_contact_op(Fred::diff_contact_data(current_data, new_data), update_contact_op);
            update_contact_op.set_logd_request_id(_log_request_id);
            do_transfer = current_data.sponsoring_registrar_handle != mojeid_registrar_handle_;
            if (do_transfer) {
                //transfer contact to 'REG-MOJEID' sponsoring registrar
                update_contact_op.set_sponsoring_registrar(mojeid_registrar_handle_);
            }
            history_id = update_contact_op.exec(ctx);
        }
        const bool is_identified      = states.presents(Fred::Object::State::IDENTIFIED_CONTACT) &&
                                        !drop_identification;
        const bool is_cond_identified = states.presents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) &&
                                        !drop_cond_identification;
        Fred::CreatePublicRequestAuth op_create_pub_req(
            //for 'identifiedContact' create 'mojeid_identified_contact_transfer' public request
            is_identified      ? Fred::MojeID::PublicRequest::IdentifiedContactTransfer::iface() :
            //for 'conditionallyIdentifiedContact' create 'mojeid_conditionally_identified_contact_transfer' public request
            is_cond_identified ? Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer::iface()
            //in other cases create 'mojeid_contact_conditional_identification' public request
                               : Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface());
        if (!current_data.notifyemail.get_value_or_default().empty()) {
            op_create_pub_req.set_email_to_answer(current_data.notifyemail.get_value());
        }
        op_create_pub_req.set_registrar_id(ctx, mojeid_registrar_handle_);
        const Fred::CreatePublicRequestAuth::Result result =
            op_create_pub_req.exec(ctx, locked_contact, _log_request_id);

        notify(ctx, Notification::updated, mojeid_registrar_id_, history_id, result.public_request_id);
        if (do_transfer) {
            notify(ctx, Notification::transferred, mojeid_registrar_id_, history_id, result.public_request_id);
        }
        //second phase commit will change contact states
        prepare_transaction_storage()->store(_trans_id, current_data.id);

        MojeIDImplData::InfoContact changed_data;
        from_into(Fred::InfoContactById(current_data.id).exec(ctx).info_contact_data, changed_data);
        ctx.commit_transaction();
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

enum { INVALID_LOG_REQUEST_ID = 0 };

Fred::UpdatePublicRequest::Result set_status(
    Fred::OperationContext &_ctx,
    const Fred::PublicRequestLockGuard &_locked_request,
    Fred::PublicRequest::Status::Value _status,
    const std::string &_reason,
    MojeIDImpl::LogRequestId _log_request_id)
{
    Fred::UpdatePublicRequest op_update_public_request;
    op_update_public_request.set_status(_status);
    if (!_reason.empty()) {
        op_update_public_request.set_reason(_reason);
    }
    if (_log_request_id == INVALID_LOG_REQUEST_ID) {
        return op_update_public_request.exec(_ctx, _locked_request);
    }
    return op_update_public_request.exec(_ctx, _locked_request, _log_request_id);
}

Fred::UpdatePublicRequest::Result answer(
    Fred::OperationContext &_ctx,
    const Fred::PublicRequestLockGuard &_locked_request,
    const std::string &_reason = "",
    MojeIDImpl::LogRequestId _log_request_id = INVALID_LOG_REQUEST_ID)
{
    return set_status(_ctx, _locked_request, Fred::PublicRequest::Status::ANSWERED, _reason, _log_request_id);
}

Fred::UpdatePublicRequest::Result invalidate(
    Fred::OperationContext &_ctx,
    const Fred::PublicRequestLockGuard &_locked_request,
    const std::string &_reason = "",
    MojeIDImpl::LogRequestId _log_request_id = INVALID_LOG_REQUEST_ID)
{
    return set_status(_ctx, _locked_request, Fred::PublicRequest::Status::INVALIDATED, _reason, _log_request_id);
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
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).error("request failed (incorrect input data)");
            throw MojeIDImplData::ObjectDoesntExist();
        }
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
            static const std::string msg = "no object associated with this public request";
            invalidate(ctx, locked_request, msg, _log_request_id);
            throw MojeIDImplData::IdentificationFailed();
        }
        const Fred::ObjectId contact_id = pub_req_info.get_object_id().get_value();
        const PubReqType::Value pub_req_type(PubReqType::from(pub_req_info.get_type()));
        try {
            switch (pub_req_info.get_status()) {
            case Fred::PublicRequest::Status::NEW:
                break;
            case Fred::PublicRequest::Status::ANSWERED:
                throw MojeIDImplData::IdentificationAlreadyProcessed();
            case Fred::PublicRequest::Status::INVALIDATED:
                throw MojeIDImplData::IdentificationAlreadyInvalidated();
            }

            Fred::StatusList to_set;
            switch (pub_req_type) {
            case PubReqType::CONTACT_CONDITIONAL_IDENTIFICATION:
                to_set.insert(Conversion::Enums::into< std::string >(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT));
                break;
            case PubReqType::CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER:
            case PubReqType::IDENTIFIED_CONTACT_TRANSFER:
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
                Database::query_param_list(locked_request.get_public_request_id())//$1::BIGINT
                                          (contact_id));                          //$2::BIGINT

            if (dbres.size() != 1) {
                throw std::runtime_error("something wrong happened database is crazy");
            }

            const bool contact_changed = static_cast< bool >(dbres[0][0]);
            if (contact_changed) {
                static const std::string msg = "contact data changed after the public request had been created";
                invalidate(ctx, locked_request, msg, _log_request_id);
                throw MojeIDImplData::ContactChanged();
            }

            if (!pub_req_info.check_password(_password)) {
                throw MojeIDImplData::IdentificationFailed();
            }

            to_set.insert(Conversion::Enums::into< std::string >(Fred::Object::State::SERVER_DELETE_PROHIBITED));
            to_set.insert(Conversion::Enums::into< std::string >(Fred::Object::State::SERVER_TRANSFER_PROHIBITED));
            to_set.insert(Conversion::Enums::into< std::string >(Fred::Object::State::SERVER_UPDATE_PROHIBITED));
            to_set.insert(Conversion::Enums::into< std::string >(Fred::Object::State::MOJEID_CONTACT));
            Fred::CreateObjectStateRequestId(contact_id, to_set).exec(ctx);
            Fred::PerformObjectStateRequest(contact_id).exec(ctx);

            answer(ctx, locked_request, "successfully processed", _log_request_id);

            if (pub_req_type != PubReqType::IDENTIFIED_CONTACT_TRANSFER) {
                Fred::CreatePublicRequestAuth op_create_pub_req(Fred::MojeID::PublicRequest::ContactIdentification::iface());
                Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, contact_id);
                const Fred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(ctx, locked_contact);
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
    catch (const MojeIDImplData::IdentificationFailed&) {
        throw;
    }
    catch (const MojeIDImplData::ContactChanged&) {
        throw;
    }
    catch (const Fred::PublicRequestLockGuardByIdentification::Exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        if (e.is_set_public_request_doesnt_exist()) {
            throw MojeIDImplData::PublicRequestDoesntExist();
        }
        throw std::runtime_error(e.what());
    }
    catch (const Fred::InfoContactById::Exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        if (e.is_set_unknown_object_id()) {
            throw MojeIDImplData::IdentificationFailed();
        }
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
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        Fred::PublicRequestId public_request_id;
        try {
            public_request_id = Fred::GetActivePublicRequest(
                Fred::MojeID::PublicRequest::ContactIdentification::iface())
                .exec(ctx, locked_contact, _log_request_id);
        }
        catch (const Fred::GetActivePublicRequest::Exception &e) {
            if (!e.is_set_no_request_found()) {
                throw;
            }
            try {
                public_request_id = Fred::GetActivePublicRequest(
                    Fred::MojeID::PublicRequest::ContactReidentification::iface())
                    .exec(ctx, locked_contact, _log_request_id);
            }
            catch (const Fred::GetActivePublicRequest::Exception &e) {
                if (e.is_set_no_request_found()) {
                    throw MojeIDImplData::PublicRequestDoesntExist();
                }
                throw;
            }
        }
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (states.absents(Fred::Object::State::MOJEID_CONTACT)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        if (states.absents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT)) {
            throw std::runtime_error("state conditionallyIdentifiedContact missing");
        }
        if (states.presents(Fred::Object::State::IDENTIFIED_CONTACT)) {
            throw MojeIDImplData::IdentificationAlreadyProcessed();
        }
        if (states.presents(Fred::Object::State::SERVER_BLOCKED)) {
            throw MojeIDImplData::ObjectAdminBlocked();
        }
        if (states.absents(Fred::Object::State::SERVER_TRANSFER_PROHIBITED) ||
            states.absents(Fred::Object::State::SERVER_UPDATE_PROHIBITED)   ||
            states.absents(Fred::Object::State::SERVER_DELETE_PROHIBITED))
        {
            throw std::runtime_error("contact not protected against changes");
        }

        const Fred::InfoContactData current_data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        {
            const MojeIDImplInternal::CheckUpdateContactPrepare check_contact_data(current_data);
            if (!check_contact_data.success()) {
                MojeIDImplInternal::raise(check_contact_data);
            }
        }
        Fred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
        if (!Fred::PublicRequestAuthInfo(ctx, locked_request).check_password(_password)) {
            throw MojeIDImplData::IdentificationFailed();
        }
        Fred::StatusList to_set;
        to_set.insert(Conversion::Enums::into< std::string >(Fred::Object::State::IDENTIFIED_CONTACT));
        Fred::CreateObjectStateRequestId(_contact_id, to_set).exec(ctx);
        Fred::PerformObjectStateRequest(_contact_id).exec(ctx);
        answer(ctx, locked_request, "successfully processed", _log_request_id);
        ctx.commit_transaction();
    }
    catch (const MojeIDImplData::UpdateContactPrepareValidationResult&) {
        LOGGER(PACKAGE).info("request failed (UpdateContactPrepareValidationResult)");
        throw;
    }
    catch (const MojeIDImplData::ObjectDoesntExist&) {
        LOGGER(PACKAGE).info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIDImplData::IdentificationAlreadyProcessed&) {
        LOGGER(PACKAGE).error("request failed (IdentificationAlreadyProcessed)");
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
        LOGGER(PACKAGE).error("request failed (cannot retrieve saved transaction data "
                              "using transaction identifier " + _trans_id + ")");
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
        LOGGER(PACKAGE).error("request failed (cannot retrieve saved transaction data "
                              "using transaction identifier " + _trans_id + ")");
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

std::string MojeIDImpl::get_validation_pdf(ContactId _contact_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;

        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _contact_id);

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
                (Fred::PublicRequest::Status(Fred::PublicRequest::Status::NEW).into< std::string >())
                (Fred::MojeID::PublicRequest::ContactValidation::iface().get_public_request_type())
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
        static const std::string czech_language = "cs";
        std::ostringstream pdf_document;
        std::auto_ptr< Fred::Document::Generator > doc_gen(
            doc_manager->createOutputGenerator(Fred::Document::GT_CONTACT_VALIDATION_REQUEST_PIN3,
                                               pdf_document,
                                               czech_language));
        const std::string request_id   = static_cast< std::string >(res[0][0]);
        const std::string name         = static_cast< std::string >(res[0][1]);
        const std::string organization = static_cast< std::string >(res[0][2]);
        const std::string ssn_value    = static_cast< std::string >(res[0][3]);
        const Fred::SSNType::Value ssn_type
                   = Fred::SSNType::from(static_cast< std::string >(res[0][4]));
        const std::string address      = static_cast< std::string >(res[0][5]);
        const std::string handle       = static_cast< std::string >(res[0][6]);
        const bool is_ssn_ico =      ssn_type == Fred::SSNType::ICO;
        const std::string birthday = ssn_type == Fred::SSNType::BIRTHDAY
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
        return pdf_document.str();
    }
    catch (const Fred::PublicRequestObjectLockGuardByObjectId::Exception &e) {
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
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        try {
            Fred::GetActivePublicRequest(Fred::MojeID::PublicRequest::ContactValidation::iface())
                .exec(ctx, locked_contact, _log_request_id);
            throw MojeIDImplData::ValidationRequestExists();
        }
        catch (const Fred::GetActivePublicRequest::Exception &e) {
            if (!e.is_set_no_request_found()) {
                throw;
            }
        }
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (states.presents(Fred::Object::State::VALIDATED_CONTACT)) {
            throw MojeIDImplData::ValidationAlreadyProcessed();
        }
        if (states.absents(Fred::Object::State::MOJEID_CONTACT)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        const Fred::InfoContactData contact_data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        {
            const MojeIDImplInternal::CheckCreateValidationRequest check_create_validation_request(contact_data);
            if (!check_create_validation_request.success()) {
                MojeIDImplInternal::raise(check_create_validation_request);
            }
        }
        Fred::CreatePublicRequest create_public_request_op(Fred::MojeID::PublicRequest::ContactValidation::iface());
        create_public_request_op.exec(ctx, locked_contact, _log_request_id);
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::PublicRequestObjectLockGuardByObjectId::Exception &e) {
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

IsNotNull add_state(const Database::Value &_valid_from, Fred::Object::State::Value _state,
                    Registry::MojeIDImplData::ContactStateInfo &_data)
{
    if (_valid_from.isnull()) {
        return false;
    }
    const std::string db_timestamp = static_cast< std::string >(_valid_from);//2014-12-11 09:28:45.741828
    boost::posix_time::ptime valid_from = boost::posix_time::time_from_string(db_timestamp);
    switch (_state) {
        case Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT:
            _data.conditionally_identification_date = boost::gregorian::date_from_tm(
                                                          boost::posix_time::to_tm(valid_from));
            break;
        case Fred::Object::State::IDENTIFIED_CONTACT:
            _data.identification_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        case Fred::Object::State::VALIDATED_CONTACT:
            _data.validation_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        case Fred::Object::State::MOJEID_CONTACT:
            _data.mojeid_activation_datetime = valid_from;
            break;
        case Fred::Object::State::LINKED:
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
        Database::query_param_list params(mojeid_registrar_handle_);             //$1::TEXT
        params(_last_hours);                                                     //$2::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT));//$3::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::IDENTIFIED_CONTACT));              //$4::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::VALIDATED_CONTACT));               //$5::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::MOJEID_CONTACT));                  //$6::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::LINKED));                          //$7::TEXT
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
            if (!add_state(rcontacts[idx][1], Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT, data)) {
                std::ostringstream msg;
                msg << "contact " << data.contact_id << " hasn't "
                    << Conversion::Enums::into< std::string >(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) << " state";
                LOGGER(PACKAGE).error(msg.str());
                continue;
            }
            add_state(rcontacts[idx][2], Fred::Object::State::IDENTIFIED_CONTACT, data);
            add_state(rcontacts[idx][3], Fred::Object::State::VALIDATED_CONTACT, data);
            if (!add_state(rcontacts[idx][4], Fred::Object::State::MOJEID_CONTACT, data)) {
                std::ostringstream msg;
                msg << "contact " << data.contact_id << " doesn't have "
                    << Conversion::Enums::into< std::string >(Fred::Object::State::MOJEID_CONTACT) << " state";
                throw std::runtime_error(msg.str());
            }
            add_state(rcontacts[idx][5], Fred::Object::State::LINKED, data);
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
        Database::query_param_list params(mojeid_registrar_handle_);                                          //$1::TEXT
        params(_contact_id);                                                                                  //$2::BIGINT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::MOJEID_CONTACT));                  //$3::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT));//$4::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::IDENTIFIED_CONTACT));              //$5::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::VALIDATED_CONTACT));               //$6::TEXT
        params(Conversion::Enums::into< std::string >(Fred::Object::State::LINKED));                          //$7::TEXT
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
        if (!add_state(rcontact[0][1], Fred::Object::State::MOJEID_CONTACT, _result)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        if (!add_state(rcontact[0][2], Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT, _result)) {
            std::ostringstream msg;
            msg << "contact " << _contact_id << " doesn't have "
                << Conversion::Enums::into< std::string >(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) << " state";
            throw std::runtime_error(msg.str());
        }
        add_state(rcontact[0][3], Fred::Object::State::IDENTIFIED_CONTACT, _result);
        add_state(rcontact[0][4], Fred::Object::State::VALIDATED_CONTACT, _result);
        add_state(rcontact[0][5], Fred::Object::State::LINKED, _result);
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
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (!(states.presents(Fred::Object::State::MOJEID_CONTACT) &&
                 (states.presents(Fred::Object::State::VALIDATED_CONTACT)  ||
                  states.presents(Fred::Object::State::IDENTIFIED_CONTACT) ||
                  states.presents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT))
             )) {
            throw std::runtime_error("bad mojeID contact");
        }

        if (states.absents(Fred::Object::State::LINKED)) {
            Fred::DeleteContactById(_contact_id).exec(ctx);
            ctx.commit_transaction();
            return;
        }

        Fred::StatusList to_cancel;
        to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::MOJEID_CONTACT));
        if (states.presents(Fred::Object::State::SERVER_UPDATE_PROHIBITED)) {
            to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::SERVER_UPDATE_PROHIBITED));
        }
        if (states.presents(Fred::Object::State::SERVER_TRANSFER_PROHIBITED)) {
            to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::SERVER_TRANSFER_PROHIBITED));
        }
        if (states.presents(Fred::Object::State::SERVER_DELETE_PROHIBITED)) {
            to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::SERVER_DELETE_PROHIBITED));
        }
        if (states.presents(Fred::Object::State::VALIDATED_CONTACT)) {
            to_cancel.insert(Conversion::Enums::into< std::string >(Fred::Object::State::VALIDATED_CONTACT));
        }
        Fred::CancelObjectStateRequestId(_contact_id, to_cancel).exec(ctx);

        Fred::UpdateContactById(_contact_id, mojeid_registrar_handle_)
            .unset_domain_expiration_letter_flag()
            .reset_address< Fred::ContactAddressType::MAILING >()
            .reset_address< Fred::ContactAddressType::BILLING >()
            .reset_address< Fred::ContactAddressType::SHIPPING >()
            .reset_address< Fred::ContactAddressType::SHIPPING_2 >()
            .reset_address< Fred::ContactAddressType::SHIPPING_3 >()
            .exec(ctx);

        Fred::UpdatePublicRequest().set_status(Fred::PublicRequest::Status::INVALIDATED)
                                   .set_reason("cancel_account_prepare call")
                                   .set_registrar_id(ctx, mojeid_registrar_handle_)
                                   .exec(ctx,
                                         locked_contact,
                                         Fred::MojeID::PublicRequest::ContactValidation::iface(),
                                         _log_request_id);
        prepare_transaction_storage()->store(_trans_id, _contact_id);
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::PublicRequestObjectLockGuardByObjectId::Exception &e) {
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
        const Fred::PublicRequestObjectLockGuardByObjectId locked_object(ctx, _contact_id);
        const Fred::Object::StatesInfo states(Fred::GetObjectStates(_contact_id).exec(ctx));
        if (states.presents(Fred::Object::State::IDENTIFIED_CONTACT)) {
            // nothing to send if contact is identified
            // IdentificationRequestDoesntExist isn't error in frontend
            throw MojeIDImplData::IdentificationRequestDoesntExist();
        }
        if (states.absents(Fred::Object::State::MOJEID_CONTACT)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
//        const occurred event(ctx, _contact_id, mojeid_registrar_handle_, _log_request_id);
        bool has_identification_request = false;
        try {
            const Fred::PublicRequestTypeIface &type = Fred::MojeID::PublicRequest::ContactIdentification::iface();
            Fred::GetActivePublicRequest get_active_public_request_op(type);
            while (true) {
                const Fred::PublicRequestId request_id = get_active_public_request_op.exec(ctx, locked_object);
                Fred::UpdatePublicRequest update_public_request_op;
                Fred::PublicRequestLockGuardById locked_request(ctx, request_id);
                update_public_request_op.set_status(Fred::PublicRequest::Status::INVALIDATED);
                update_public_request_op.set_reason("new pin3 generated");
                update_public_request_op.set_registrar_id(ctx, mojeid_registrar_handle_);
                update_public_request_op.exec(ctx, locked_request);
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
            const Fred::PublicRequestTypeIface &type = Fred::MojeID::PublicRequest::ContactReidentification::iface();
            Fred::GetActivePublicRequest get_active_public_request_op(type);
            while (true) {
                const Fred::PublicRequestId request_id = get_active_public_request_op.exec(ctx, locked_object);
                Fred::UpdatePublicRequest update_public_request_op;
                Fred::PublicRequestLockGuardById locked_request(ctx, request_id);
                update_public_request_op.set_status(Fred::PublicRequest::Status::INVALIDATED);
                update_public_request_op.set_reason("new pin3 generated");
                update_public_request_op.set_registrar_id(ctx, mojeid_registrar_handle_);
                update_public_request_op.exec(ctx, locked_request);
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

        const Fred::PublicRequestAuthTypeIface &type = has_reidentification_request
                                                       ? Fred::MojeID::PublicRequest::ContactReidentification::iface()
                                                       : Fred::MojeID::PublicRequest::ContactIdentification::iface();
        Fred::CreatePublicRequestAuth create_public_request_op(type);
        create_public_request_op.set_registrar_id(ctx, mojeid_registrar_handle_);
        create_public_request_op.set_reason("send_new_pin3 call");
        const Fred::CreatePublicRequestAuth::Result result =
        create_public_request_op.exec(ctx, locked_object, _log_request_id);
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
    catch(const Fred::PublicRequestObjectLockGuardByObjectId::Exception &e) {
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
        if (states.absents(Fred::Object::State::MOJEID_CONTACT)) {
            throw MojeIDImplData::ObjectDoesntExist();
        }
        if (states.absents(Fred::Object::State::IDENTIFIED_CONTACT)) {
//            throw IdentificationRequestDoesntExist("contact already identified");
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
            states.presents(Fred::Object::State::VALIDATED_CONTACT));
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
        ::MojeID::Messages::Generate::Into< CommChannel::SMS >::for_new_requests(ctx);
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
        ::MojeID::Messages::Generate::enable< ::MojeID::Messages::CommChannel::SMS >(ctx, enable);
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
        ::MojeID::Messages::Generate::Into< CommChannel::LETTER >::for_new_requests(
            ctx, ::MojeID::Messages::DefaultMultimanager(), check_limits::sent_letters());
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
        ::MojeID::Messages::Generate::enable< ::MojeID::Messages::CommChannel::LETTER >(ctx, enable);
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
        static const std::string link_hostname_part = CfgArgs::instance()
                                                          ->get_handler_ptr_by_type< HandleMojeIDArgs >()
                                                              ->hostname;
        ::MojeID::Messages::Generate::Into< CommChannel::EMAIL >::for_new_requests(
            ctx,
            ::MojeID::Messages::DefaultMultimanager(),
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
        ::MojeID::Messages::Generate::enable< ::MojeID::Messages::CommChannel::EMAIL >(ctx, enable);
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
              (Conversion::Enums::into< std::string >(Fred::Object::State::VALIDATED_CONTACT));
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

    static const std::string comm_type = "letter";
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
