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
 *  mojeid2 implementation
 */

#include "src/mojeid/mojeid2.h"
#include "src/mojeid/safe_data_storage.h"
#include "src/mojeid/mojeid_public_request.h"
#include "src/mojeid/messages/generate.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/info_contact_diff.h"
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
#include "src/corba/mojeid/corba_conversion2.h"
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
    LogContext(const MojeID2Impl &_impl, const std::string &_op_name)
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

void set_create_contact_arguments(
    const Fred::InfoContactData &_contact,
    Fred::CreateContact &_arguments)
{
    if (!_contact.name.isnull()) {
        _arguments.set_name(_contact.name.get_value());
    }
    if (!_contact.place.isnull()) {
        _arguments.set_place(_contact.place.get_value());
    }
    if (!_contact.email.isnull()) {
        _arguments.set_email(_contact.email.get_value());
    }
    if (!_contact.telephone.isnull()) {
        _arguments.set_telephone(_contact.telephone.get_value());
    }
    if (!_contact.organization.isnull()) {
        _arguments.set_organization(_contact.organization.get_value());
    }
    if (!_contact.notifyemail.isnull()) {
        _arguments.set_notifyemail(_contact.notifyemail.get_value());
    }
    if (!_contact.fax.isnull()) {
        _arguments.set_fax(_contact.fax.get_value());
    }
    if (!_contact.addresses.empty()) {
        _arguments.set_addresses(_contact.addresses);
    }
    if (!_contact.vat.isnull()) {
        _arguments.set_vat(_contact.vat.get_value());
    }
    if (!_contact.ssntype.isnull()) {
        _arguments.set_ssntype(_contact.ssntype.get_value());
    }
    if (!_contact.ssn.isnull()) {
        _arguments.set_ssn(_contact.ssn.get_value());
    }
}

void check_sent_letters_limit(Fred::OperationContext &_ctx,
                              MojeID2Impl::ContactId _contact_id,
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
    if (result.size() <= 0) {
        return;
    }
    throw MojeID2Impl::MessageLimitExceeded(
        boost::gregorian::from_simple_string(static_cast< std::string >(result[0][0])),
        _max_sent_letters,
        _watched_period_in_days);
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
        if (_c1.ssntype.get_value_or_default() != "BIRTHDAY") {
            return true;
        }
        const Nullable< boost::gregorian::date > bd1 = Corba::Conversion::convert_as_birthdate(_c1.ssn);
        const Nullable< boost::gregorian::date > bd2 = Corba::Conversion::convert_as_birthdate(_c2.ssn);
        if (differs(bd1, bd2)) {
            return true;
        }
    }

    return false;
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
    MessageType(Value _value):value_(_value) { }
    struct bad_conversion:std::runtime_error
    {
        bad_conversion(const std::string &_msg):std::runtime_error(_msg) { }
    };
    template < typename T >
    T into()const { T result; return this->into(result); }
    std::string& into(std::string &_value)const
    {
        typedef std::map< Value, std::string > ValueToStr;
        static ValueToStr value_to_str;
        if (value_to_str.empty()) {
            value_to_str[DOMAIN_EXPIRATION]         = "domain_expiration";
            value_to_str[MOJEID_PIN2]               = "mojeid_pin2";
            value_to_str[MOJEID_PIN3]               = "mojeid_pin3";
            value_to_str[MOJEID_SMS_CHANGE]         = "mojeid_sms_change";
            value_to_str[MONITORING]                = "monitoring";
            value_to_str[CONTACT_VERIFICATION_PIN2] = "contact_verification_pin2";
            value_to_str[CONTACT_VERIFICATION_PIN3] = "contact_verification_pin3";
            value_to_str[MOJEID_PIN3_REMINDER]      = "mojeid_pin3_reminder";
            value_to_str[CONTACT_CHECK_NOTICE]      = "contact_check_notice";
            value_to_str[CONTACT_CHECK_THANK_YOU]   = "contact_check_thank_you";
            value_to_str[MOJEID_CARD]               = "mojeid_card";
        }
        ValueToStr::const_iterator item_ptr = value_to_str.find(value_);
        if (item_ptr != value_to_str.end()) {
            return _value = item_ptr->second;
        }
        throw bad_conversion("invalid value");
    }
private:
    const Value value_;
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
    CommType(Value _value):value_(_value) { }
    struct bad_conversion:std::runtime_error
    {
        bad_conversion(const std::string &_msg):std::runtime_error(_msg) { }
    };
    template < typename T >
    T into()const { T result; return this->into(result); }
    std::string& into(std::string &_value)const
    {
        typedef std::map< Value, std::string > ValueToStr;
        static ValueToStr value_to_str;
        if (value_to_str.empty()) {
            value_to_str[EMAIL]             = "email";
            value_to_str[LETTER]            = "letter";
            value_to_str[SMS]               = "sms";
            value_to_str[REGISTERED_LETTER] = "registered_letter";
        }
        ValueToStr::const_iterator item_ptr = value_to_str.find(value_);
        if (item_ptr != value_to_str.end()) {
            return _value = item_ptr->second;
        }
        throw bad_conversion("invalid value");
    }
private:
    const Value value_;
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
    SendStatus(Value _value):value_(_value) { }
    struct bad_conversion:std::runtime_error
    {
        bad_conversion(const std::string &_msg):std::runtime_error(_msg) { }
    };
    template < typename T >
    T into()const { T result; return this->into(result); }
    std::string& into(std::string &_value)const
    {
        typedef std::map< Value, std::string > ValueToStr;
        static ValueToStr value_to_str;
        if (value_to_str.empty()) {
            value_to_str[READY]                = "ready";
            value_to_str[WAITING_CONFIRMATION] = "waiting_confirmation";
            value_to_str[NO_PROCESSING]        = "no_processing";
            value_to_str[SEND_FAILED]          = "send_failed";
            value_to_str[SENT]                 = "sent";
            value_to_str[BEING_SENT]           = "being_sent";
            value_to_str[UNDELIVERED]          = "undelivered";
        }
        ValueToStr::const_iterator item_ptr = value_to_str.find(value_);
        if (item_ptr != value_to_str.end()) {
            return _value = item_ptr->second;
        }
        throw bad_conversion("invalid value");
    }
private:
    const Value value_;
};

template < MessageType::Value MT, CommType::Value CT >
::size_t cancel_message_sending(Fred::OperationContext &_ctx, MojeID2Impl::ContactId _contact_id)
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
        Database::query_param_list(_contact_id)                                                //$1::BIGINT
                                  (CommType(CT).into< std::string >())                         //$2::TEXT
                                  (MessageType(MT).into< std::string >())                      //$3::TEXT
                                  (SendStatus(SendStatus::NO_PROCESSING).into< std::string >())//$4::TEXT
                                  (SendStatus(SendStatus::SEND_FAILED).into< std::string >())  //$5::TEXT
                                  (SendStatus(SendStatus::READY).into< std::string >()));      //$6::TEXT
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
        if (_c1.ssntype.get_value_or_default() != "BIRTHDAY") {
            return true;
        }
        const Nullable< boost::gregorian::date > bd1 = Corba::Conversion::convert_as_birthdate(_c1.ssn);
        const Nullable< boost::gregorian::date > bd2 = Corba::Conversion::convert_as_birthdate(_c2.ssn);
        if (differs(bd1, bd2)) {
            return true;
        }
    }

    return false;
}

typedef data_storage< std::string, MojeID2Impl::ContactId >::safe prepare_transaction_storage;
typedef prepare_transaction_storage::object_type::data_not_found prepare_transaction_data_not_found;

struct PubReqType
{
    enum Value
    {
        CONTACT_CONDITIONAL_IDENTIFICATION,
        CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER,
        IDENTIFIED_CONTACT_TRANSFER,
    };
    static Value from(const std::string &_type)
    {
        typedef std::map< std::string, Value > StrToValue;
        static StrToValue convert;
        if (convert.empty()) {
            using namespace Fred::MojeID::PublicRequest;
            convert[into_string< ContactConditionalIdentification >()]       = CONTACT_CONDITIONAL_IDENTIFICATION;
            convert[into_string< ConditionallyIdentifiedContactTransfer >()] = CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
            convert[into_string< IdentifiedContactTransfer >()]              = IDENTIFIED_CONTACT_TRANSFER;
        }
        StrToValue::const_iterator value_ptr = convert.find(_type);
        if (value_ptr != convert.end()) {
            return value_ptr->second;
        }
        throw std::runtime_error("unexpected public request type " + _type);
    }
    template < class PUB_REQ >
    static std::string into_string()
    {
        return PUB_REQ::iface().get_public_request_type();
    }
};

class transitions:public StateMachine::base< transitions >
{
public:
    typedef StateMachine::base< transitions > base_state_machine;
    template < Fred::Object::State::Value FRED_STATE >
    struct single:boost::integral_constant< Fred::Object::State::Value, FRED_STATE > { };

    struct C:single< Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT > { };
    struct I:single< Fred::Object::State::IDENTIFIED_CONTACT > { };
    struct V:single< Fred::Object::State::VALIDATED_CONTACT > { };
    struct M:single< Fred::Object::State::MOJEID_CONTACT > { };

    template < bool HAS_C, bool HAS_I, bool HAS_V, bool HAS_M >
    struct civm_collection:StateMachine::state_on_change_action< civm_collection< HAS_C, HAS_I, HAS_V, HAS_M > >
    {
        static const bool has_c = HAS_C;
        static const bool has_i = HAS_I;
        static const bool has_v = HAS_V;
        static const bool has_m = HAS_M;

        static const bool has_any = has_c || has_i || has_v || has_m;

        template < typename STATE >
        struct add
        {
            typedef civm_collection< has_c || boost::is_same< STATE, C >::value,
                                     has_i || boost::is_same< STATE, I >::value,
                                     has_v || boost::is_same< STATE, V >::value,
                                     has_m || boost::is_same< STATE, M >::value > type;
        };

        typedef Fred::StatusList state_container;
        typedef state_container::key_type state_item;

        template < typename STATE, bool PRESENT >
        struct into_container
        {
            static void add(state_container &_status_list)
            {
                _status_list.insert(Fred::Object::State(STATE::value).into< state_item >());
            }
        };

        template < typename STATE >
        struct into_container< STATE, false >
        {
            static void add(const state_container&) { }
        };

        static const state_container& as_state_container()
        {
            static state_container status_list;
            if (has_any && status_list.empty()) {
                into_container< C, has_c >::add(status_list);
                into_container< I, has_i >::add(status_list);
                into_container< V, has_v >::add(status_list);
                into_container< M, has_m >::add(status_list);
            }
            return status_list;
        }

        template < typename EVENT >
        static void set(const EVENT &event)
        {
            if (has_any) {
                const state_container &to_set = as_state_container();
                Fred::CreateObjectStateRequestId(event.get_object_id(), to_set).exec(event.get_operation_context());
            }
        }

        template < typename EVENT >
        static void reset(const EVENT &event)
        {
            if (has_any) {
                const state_container &to_reset = as_state_container();
                Fred::CancelObjectStateRequestId(event.get_object_id(), to_reset).exec(event.get_operation_context());
            }
        }
    };

    template < typename STATES >
    struct collect
    {
        typedef civm_collection< boost::mpl::contains< STATES, C >::value,
                                 boost::mpl::contains< STATES, I >::value,
                                 boost::mpl::contains< STATES, V >::value,
                                 boost::mpl::contains< STATES, M >::value > type;
    };

    typedef typename collect< boost::mpl::set<            > >::type civm;
    typedef typename collect< boost::mpl::set< C          > >::type Civm;
    typedef typename collect< boost::mpl::set<    I       > >::type cIvm;
    typedef typename collect< boost::mpl::set< C, I       > >::type CIvm;
    typedef typename collect< boost::mpl::set<       V    > >::type ciVm;
    typedef typename collect< boost::mpl::set< C,    V    > >::type CiVm;
    typedef typename collect< boost::mpl::set<    I, V    > >::type cIVm;
    typedef typename collect< boost::mpl::set< C, I, V    > >::type CIVm;
    typedef typename collect< boost::mpl::set<          M > >::type civM;
    typedef typename collect< boost::mpl::set< C,       M > >::type CivM;
    typedef typename collect< boost::mpl::set<    I,    M > >::type cIvM;
    typedef typename collect< boost::mpl::set< C, I,    M > >::type CIvM;
    typedef typename collect< boost::mpl::set<       V, M > >::type ciVM;
    typedef typename collect< boost::mpl::set< C,    V, M > >::type CiVM;
    typedef typename collect< boost::mpl::set<    I, V, M > >::type cIVM;
    typedef typename collect< boost::mpl::set< C, I, V, M > >::type CIVM;

    template < typename STATE >
    struct single_state
    {
        template < typename STATE_PRESENT >
        static bool present_in(const STATE_PRESENT &states)
        {
            return states.STATE_PRESENT::template get< STATE::value >();
        }
        template < typename STATES >
        struct add_into
        {
            typedef typename STATES::template add< STATE >::type type;
        };
    };

    template < typename STATES >
    struct state_collection
    {
        template < typename A, typename B >
        struct in_a_and_not_in_b
        {
            typedef civm_collection< A::has_c && !B::has_c,
                                     A::has_i && !B::has_i,
                                     A::has_v && !B::has_v,
                                     A::has_m && !B::has_m > type;
        };
        template < typename NEXT, typename EVENT >
        static void set(const EVENT &event)
        {
            typedef STATES current;
            typedef NEXT next;
            typedef typename in_a_and_not_in_b< current, next >::type to_reset;
            typedef typename in_a_and_not_in_b< next, current >::type to_set;
            to_reset::reset(event);
            to_set::set(event);
        }
    };

    struct event
    {
        class transfer_contact_prepare
        {
        public:
            transfer_contact_prepare(
                Fred::OperationContext &_ctx,
                const Fred::InfoContactData &_contact,
                const std::string &_trans_id,
                const std::string &_registrar_handle,
                MojeID2Impl::LogRequestId _log_request_id,
                std::string &_ident);
            Fred::OperationContext& get_operation_context()const { return ctx_; }
            const Fred::InfoContactData& get_contact()const { return contact_; }
            Fred::ObjectId get_object_id()const { return contact_.id; }
            const std::string& get_trans_id()const { return trans_id_; }
            const std::string& get_registrar_handle()const { return registrar_handle_; }
            void set_ident(const std::string &_value)const { ident_ = _value; }
            const Fred::PublicRequestObjectLockGuard& get_locked_contact()const { return locked_contact_; }
            typedef Fred::Object::State::set<
                        Fred::Object::State::SERVER_TRANSFER_PROHIBITED,
                        Fred::Object::State::SERVER_UPDATE_PROHIBITED,
                        Fred::Object::State::SERVER_DELETE_PROHIBITED,
                        Fred::Object::State::SERVER_BLOCKED,
                        Fred::Object::State::MOJEID_CONTACT,
                        Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT,
                        Fred::Object::State::IDENTIFIED_CONTACT,
                        Fred::Object::State::VALIDATED_CONTACT >::type RelatedStates;
            typedef MojeID2Impl::GetContact::States< RelatedStates >::Presence StatesPresence;
        private:
            Fred::OperationContext &ctx_;
            const Fred::InfoContactData &contact_;
            const std::string trans_id_;
            const std::string registrar_handle_;
            const MojeID2Impl::LogRequestId log_request_id_;
            std::string &ident_;
            const Fred::PublicRequestObjectLockGuardByObjectId locked_contact_;
        };

        class process_registration_request
        {
        public:
            process_registration_request(
                Fred::OperationContext &_ctx,
                const std::string &_ident_request_id,
                const std::string &_password,
                const std::string &_registrar_handle,
                MojeID2Impl::LogRequestId _log_request_id);
            Fred::OperationContext& get_operation_context()const { return ctx_; }
            const std::string& get_password()const { return password_; }
            const std::string& get_registrar_handle()const { return registrar_handle_; }
            MojeID2Impl::LogRequestId get_log_request_id()const { return log_request_id_; }
            const Fred::PublicRequestLockGuard& get_locked_request()const { return locked_request_; }
            const Fred::PublicRequestAuthInfo& get_pub_req_info()const { return pub_req_info_; }
            PubReqType::Value get_pub_req_type()const { return pub_req_type_; }
            Fred::ObjectId get_object_id()const { return pub_req_info_.get_object_id().get_value(); }
            typedef Fred::Object::State::set<
                        Fred::Object::State::SERVER_TRANSFER_PROHIBITED,
                        Fred::Object::State::SERVER_UPDATE_PROHIBITED,
                        Fred::Object::State::SERVER_DELETE_PROHIBITED,
                        Fred::Object::State::SERVER_BLOCKED,
                        Fred::Object::State::MOJEID_CONTACT,
                        Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT,
                        Fred::Object::State::IDENTIFIED_CONTACT,
                        Fred::Object::State::VALIDATED_CONTACT >::type RelatedStates;
            typedef MojeID2Impl::GetContact::States< RelatedStates >::Presence StatesPresence;
        private:
            Fred::OperationContext &ctx_;
            const std::string password_;
            const std::string registrar_handle_;
            const MojeID2Impl::LogRequestId log_request_id_;
            const Fred::PublicRequestLockGuardByIdentification locked_request_;
            const Fred::PublicRequestAuthInfo pub_req_info_;
            const PubReqType::Value pub_req_type_;
        };

        class send_new_pin3
        {
        public:
            send_new_pin3(
                Fred::OperationContext &_ctx,
                MojeID2Impl::ContactId _contact_id,
                const std::string &_registrar_handle,
                MojeID2Impl::LogRequestId _log_request_id);
            Fred::OperationContext& get_operation_context()const { return ctx_; }
            const Fred::PublicRequestObjectLockGuard& get_locked_object()const { return locked_object_; }
            const std::string& get_registrar_handle()const { return registrar_handle_; }
            MojeID2Impl::LogRequestId get_log_request_id()const { return log_request_id_; }
            Fred::ObjectId get_object_id()const { return contact_id_; }
            typedef Fred::Object::State::set<
                        Fred::Object::State::SERVER_TRANSFER_PROHIBITED,
                        Fred::Object::State::SERVER_UPDATE_PROHIBITED,
                        Fred::Object::State::SERVER_DELETE_PROHIBITED,
                        Fred::Object::State::SERVER_BLOCKED,
                        Fred::Object::State::MOJEID_CONTACT,
                        Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT,
                        Fred::Object::State::IDENTIFIED_CONTACT,
                        Fred::Object::State::VALIDATED_CONTACT >::type RelatedStates;
            typedef MojeID2Impl::GetContact::States< RelatedStates >::Presence StatesPresence;
        private:
            Fred::OperationContext &ctx_;
            const MojeID2Impl::ContactId contact_id_;
            const Fred::PublicRequestObjectLockGuardByObjectId locked_object_;
            const std::string registrar_handle_;
            const MojeID2Impl::LogRequestId log_request_id_;
        };
    };

    struct guard
    {

        struct transfer_contact_prepare
        {
            void operator()(const event::transfer_contact_prepare &_event,
                            const event::transfer_contact_prepare::StatesPresence &_states)const;
        };

        struct process_registration_request
        {
            void operator()(const event::process_registration_request &_event,
                            const event::process_registration_request::StatesPresence &_states)const;
        };
        struct process_contact_conditional_identification
        {
            void operator()(const event::process_registration_request &_event,
                            const event::process_registration_request::StatesPresence &_states)const;
        };
        struct process_conditionally_identified_contact_transfer
        {
            void operator()(const event::process_registration_request &_event,
                            const event::process_registration_request::StatesPresence &_states)const;
        };
        struct process_identified_contact_transfer
        {
            void operator()(const event::process_registration_request &_event,
                            const event::process_registration_request::StatesPresence &_states)const;
        };

        struct send_new_pin3
        {
            void operator()(const event::send_new_pin3 &_event,
                            const event::send_new_pin3::StatesPresence &_states)const;
        };
    };

    struct action:base_state_machine::action
    {
        struct transfer_contact_prepare_civm
        {
            void operator()(const event::transfer_contact_prepare &_event,
                            const event::transfer_contact_prepare::StatesPresence &_states)const;
        };
        struct transfer_contact_prepare_Civm
        {
            void operator()(const event::transfer_contact_prepare &_event,
                            const event::transfer_contact_prepare::StatesPresence &_states)const;
        };
        struct transfer_contact_prepare_CIvm
        {
            void operator()(const event::transfer_contact_prepare &_event,
                            const event::transfer_contact_prepare::StatesPresence &_states)const;
        };

        struct process_contact_conditional_identification
        {
            void operator()(const event::process_registration_request &_event,
                            const event::process_registration_request::StatesPresence &_states)const;
        };
        struct process_conditionally_identified_contact_transfer
        {
            void operator()(const event::process_registration_request &_event,
                            const event::process_registration_request::StatesPresence &_states)const;
        };
        struct process_identified_contact_transfer
        {
            void operator()(const event::process_registration_request &_event,
                            const event::process_registration_request::StatesPresence &_states)const;
        };

        struct send_new_pin3
        {
            void operator()(const event::send_new_pin3 &_event,
                            const event::send_new_pin3::StatesPresence &_states)const;
        };
    };

    typedef boost::mpl::set<
        a_row< civm,  event::transfer_contact_prepare,
                     action::transfer_contact_prepare_civm,
                      guard::transfer_contact_prepare,
               civm >,
        a_row< civm,  event::process_registration_request,
                     action::process_contact_conditional_identification,
                      guard::process_contact_conditional_identification,
               CivM >,
        a_row< Civm,  event::transfer_contact_prepare,
                     action::transfer_contact_prepare_Civm,
                      guard::transfer_contact_prepare,
               Civm >,
        a_row< Civm,  event::process_registration_request,
                     action::process_conditionally_identified_contact_transfer,
                      guard::process_conditionally_identified_contact_transfer,
               CivM >,
        a_row< CIvm,  event::transfer_contact_prepare,
                     action::transfer_contact_prepare_CIvm,
                      guard::transfer_contact_prepare,
               CIvm >,
        a_row< CIvm,  event::process_registration_request,
                     action::process_identified_contact_transfer,
                      guard::process_identified_contact_transfer,
               CIvM >,
        a_row< CivM,  event::send_new_pin3,
                     action::send_new_pin3,
                      guard::send_new_pin3,
               CivM >
    >                                      transition_table;
    typedef civm                           empty_state_collection;
    typedef boost::mpl::list< C, I, V, M > list_of_checked_states;
};

}//Registry::MojeID::{anonymous}

MojeID2Impl::MojeID2Impl(const std::string &_server_name)
:   server_name_(_server_name),
    mojeid_registrar_handle_(get_mojeid_registrar_handle())
{
    LogContext log_ctx(*this, "init");
}//MojeID2Impl::MojeID2Impl

MojeID2Impl::~MojeID2Impl()
{
}

const std::string& MojeID2Impl::get_server_name()const
{
    return server_name_;
}

HandleList& MojeID2Impl::get_unregistrable_contact_handles(
        HandleList &_result)const
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
        return _result;
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

MojeID2Impl::ContactId MojeID2Impl::create_contact_prepare(
        const Fred::InfoContactData &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);

        {
              const CheckCreateContactPrepare check_contact_data(
                Fred::make_args(_contact),
                Fred::make_args(_contact, ctx));

            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
        }

        Fred::CreateContact op_create_contact(_contact.handle, mojeid_registrar_handle_);
        set_create_contact_arguments(_contact, op_create_contact);
        const Fred::CreateContact::Result new_contact = op_create_contact.exec(ctx);
        Fred::CreatePublicRequestAuth op_create_pub_req(
            Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface());
        Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, new_contact.object_id);
        {
            const Fred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(ctx, locked_contact);
            _ident = result.identification;
        }
        prepare_transaction_storage()->store(_trans_id, new_contact.object_id);
        ctx.commit_transaction();
        return new_contact.object_id;
    }
    catch (const CheckCreateContactPrepare&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data)");
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

Fred::InfoContactData& MojeID2Impl::transfer_contact_prepare(
        const std::string &_handle,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        Fred::InfoContactData &_contact,
        std::string &_ident)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        _contact = Fred::InfoContactByHandle(_handle).exec(ctx).info_contact_data;
        typedef transitions::event::transfer_contact_prepare occurred_event;
        transitions::process(
            occurred_event(ctx, _contact, _trans_id, mojeid_registrar_handle_, _log_request_id, _ident),
            GetContact(_contact.id).states< occurred_event::RelatedStates >().presence(ctx));
        ctx.commit_transaction();
        return _contact;
    }
    catch (const CheckMojeIDRegistration&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data)");
        throw;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).error("request failed (incorrect input data)");
            throw Fred::Object::Get< Fred::Object::Type::CONTACT >::object_doesnt_exist();
        }
        throw;
    }
    catch (const GetContact::object_doesnt_exist &e) {
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
        _update_op.set_name(_data_changes.name.get_value().second.get_value());
    }
    if (_data_changes.organization.isset()) {
        _update_op.set_organization(_data_changes.organization.get_value().second.get_value_or_default());
    }
    if (_data_changes.ssntype.isset()) {
        _update_op.set_ssntype(_data_changes.ssntype.get_value().second.get_value_or_default());
    }
    if (_data_changes.ssn.isset()) {
        _update_op.set_ssn(_data_changes.ssn.get_value().second.get_value_or_default());
    }
    if (_data_changes.place.isset()) {
        _update_op.set_place(_data_changes.place.get_value().second.get_value());
    }
    if (_data_changes.addresses.isset()) {
        update_address< Fred::ContactAddressType::MAILING >   (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::BILLING >   (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING >  (_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING_2 >(_data_changes, _update_op);
        update_address< Fred::ContactAddressType::SHIPPING_3 >(_data_changes, _update_op);
    }
    if (_data_changes.email.isset()) {
        _update_op.set_email(_data_changes.email.get_value().second.get_value());
    }
    if (_data_changes.notifyemail.isset()) {
        _update_op.set_notifyemail(_data_changes.notifyemail.get_value().second.get_value_or_default());
    }
    if (_data_changes.telephone.isset()) {
        _update_op.set_telephone(_data_changes.telephone.get_value().second.get_value_or_default());
    }
    if (_data_changes.fax.isset()) {
        _update_op.set_fax(_data_changes.fax.get_value().second.get_value_or_default());
    }
}

}

void MojeID2Impl::update_contact_prepare(
        const Fred::InfoContactData &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        typedef Fred::Object::State FOS;
        typedef FOS::set<
            FOS::SERVER_TRANSFER_PROHIBITED,
            FOS::SERVER_UPDATE_PROHIBITED,
            FOS::SERVER_DELETE_PROHIBITED,
            FOS::SERVER_BLOCKED,
            FOS::MOJEID_CONTACT,
            FOS::CONDITIONALLY_IDENTIFIED_CONTACT,
            FOS::IDENTIFIED_CONTACT,
            FOS::VALIDATED_CONTACT,
            FOS::CONTACT_FAILED_MANUAL_VERIFICATION,
            FOS::CONTACT_PASSED_MANUAL_VERIFICATION >::type RelatedStates;
        typedef GetContact::States< RelatedStates >::Presence StatesPresence;
        const StatesPresence states =
            GetContact(_new_data.id).states< RelatedStates >().presence(ctx);
        if (!states.get< FOS::MOJEID_CONTACT >()) {
            throw GetContact::object_doesnt_exist();
        }
        const Fred::InfoContactData current_data = Fred::InfoContactById(_new_data.id).exec(ctx).info_contact_data;
        const Fred::InfoContactDiff data_changes = Fred::diff_contact_data(current_data, _new_data);
        if (!(data_changes.name.isset()         ||
              data_changes.organization.isset() ||
              data_changes.ssntype.isset()      ||
              data_changes.ssn.isset()          ||
              data_changes.place.isset()        ||
              data_changes.addresses.isset()    ||
              data_changes.email.isset()        ||
              data_changes.notifyemail.isset()  ||
              data_changes.telephone.isset()    ||
              data_changes.fax.isset())) {
            return;
        }
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _new_data.id);
        Fred::StatusList to_cancel;
        bool drop_validation = false;
        if (states.get< FOS::VALIDATED_CONTACT >()) {
            drop_validation = validated_data_changed(current_data, _new_data);
            if (drop_validation) {
                to_cancel.insert(FOS(FOS::VALIDATED_CONTACT).into< std::string >());
            }
        }
        const bool drop_identification = identified_data_changed(current_data, _new_data);
        if (drop_identification || differs(current_data.email, _new_data.email)) {
            cancel_message_sending< MessageType::MOJEID_CARD, CommType::LETTER >(ctx, _new_data.id);
        }
        if (drop_identification) {
            const bool reidentification_needed = states.get< FOS::IDENTIFIED_CONTACT >();
            if (reidentification_needed) {
                to_cancel.insert(FOS(FOS::IDENTIFIED_CONTACT).into< std::string >());
            }
            const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->
                                                                get_handler_ptr_by_type< HandleMojeIDArgs >();
            check_sent_letters_limit(ctx,
                                     _new_data.id,
                                     server_conf_ptr->letter_limit_count,
                                     server_conf_ptr->letter_limit_interval);
            Fred::CreatePublicRequestAuth create_public_request_op(
                reidentification_needed ? Fred::MojeID::PublicRequest::ContactReidentification::iface()
                                        : Fred::MojeID::PublicRequest::ContactIdentification::iface());
            create_public_request_op.set_reason("data changed");
            create_public_request_op.set_registrar_id(ctx, mojeid_registrar_handle_);
            create_public_request_op.exec(ctx, locked_contact, _log_request_id);
        }
        {
            const CheckUpdateContactPrepare check_contact_data(_new_data);
            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
        }
        const bool manual_verification_done = states.get< FOS::CONTACT_FAILED_MANUAL_VERIFICATION >() ||
                                              states.get< FOS::CONTACT_PASSED_MANUAL_VERIFICATION >();
        if (manual_verification_done) {
            const bool cancel_manual_verification = data_changes.name.isset()         ||
                                                    data_changes.organization.isset() ||
                                                    data_changes.ssntype.isset()      ||
                                                    data_changes.ssn.isset()          ||
                                                    data_changes.place.isset()        ||
                                                    data_changes.email.isset()        ||
                                                    data_changes.notifyemail.isset()  ||
                                                    data_changes.telephone.isset()    ||
                                                    data_changes.fax.isset();
            if (cancel_manual_verification) {
                if (states.get< FOS::CONTACT_FAILED_MANUAL_VERIFICATION >()) {
                    to_cancel.insert(FOS(FOS::CONTACT_FAILED_MANUAL_VERIFICATION).into< std::string >());
                }
                if (states.get< FOS::CONTACT_PASSED_MANUAL_VERIFICATION >()) {
                    to_cancel.insert(FOS(FOS::CONTACT_PASSED_MANUAL_VERIFICATION).into< std::string >());
                }
            }
        }
        if (!to_cancel.empty()) {
            Fred::CancelObjectStateRequestId(_new_data.id, to_cancel).exec(ctx);
        }
        Fred::UpdateContactById update_contact_op(_new_data.id, mojeid_registrar_handle_);
        set_update_contact_op(data_changes, update_contact_op);
        const bool is_identified = states.get< FOS::IDENTIFIED_CONTACT >() && !drop_identification;
        const bool is_validated  = states.get< FOS::VALIDATED_CONTACT  >() && !drop_validation;
        const bool addr_can_be_hidden = (is_identified || is_validated) &&
                                        _new_data.organization.get_value_or_default().empty();
        if (!addr_can_be_hidden) {
            update_contact_op.set_discloseaddress(true);
        }
        update_contact_op.exec(ctx);
        ctx.commit_transaction();
        return;
    }
    catch (const GetContact::object_doesnt_exist &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch(const MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch(const UpdateContactPrepareError &e) {
        LOGGER(PACKAGE).error("request failed (incorrect input data)");
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

void MojeID2Impl::update_transfer_contact_prepare(
        const std::string &_username,
        Fred::InfoContactData &_new_data,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        //check contact is registered
        const Fred::InfoContactData current_data = Fred::InfoContactByHandle(_username).exec(ctx).info_contact_data;
        _new_data.id     = current_data.id;
        _new_data.handle = current_data.handle;
        typedef Fred::Object::State FOS;
        typedef FOS::set<
            FOS::SERVER_TRANSFER_PROHIBITED,
            FOS::SERVER_UPDATE_PROHIBITED,
            FOS::SERVER_DELETE_PROHIBITED,
            FOS::SERVER_BLOCKED,
            FOS::MOJEID_CONTACT,
            FOS::CONDITIONALLY_IDENTIFIED_CONTACT,
            FOS::IDENTIFIED_CONTACT,
            FOS::VALIDATED_CONTACT >::type RelatedStates;
        typedef GetContact::States< RelatedStates >::Presence StatesPresence;
        const StatesPresence states =
            GetContact(_new_data.id).states< RelatedStates >().presence(ctx);
        //check contact is 'mojeidContact', if true throw ALREADY_MOJEID_CONTACT
        if (states.get< FOS::MOJEID_CONTACT >()) {
            throw AlreadyMojeidContact("unable to transfer mojeID contact into mojeID");
        }
        //throw OBJECT_ADMIN_BLOCKED if contact is administrative blocked
        if (states.get< FOS::SERVER_BLOCKED >()) {
            throw ObjectAdminBlocked("unable to transfer administrative blocked contact into mojeID");
        }
        //throw OBJECT_USER_BLOCKED if contact is blocked by user
        if (states.get< FOS::SERVER_TRANSFER_PROHIBITED >() ||
            states.get< FOS::SERVER_UPDATE_PROHIBITED >() ||
            states.get< FOS::SERVER_DELETE_PROHIBITED >()) {
            throw ObjectUserBlocked("unable to transfer user blocked contact into mojeID");
        }
        //transfer contact to 'REG-MOJEID' sponsoring registrar
        if (current_data.sponsoring_registrar_handle != mojeid_registrar_handle_) {
            Fred::UpdateContactById op_update_contact(current_data.id, mojeid_registrar_handle_);
            op_update_contact.set_sponsoring_registrar(mojeid_registrar_handle_);
            op_update_contact.set_logd_request_id(_log_request_id);
            op_update_contact.exec(ctx);
        }
        check_limits::sent_letters()(ctx, current_data.id);
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _new_data.id);
        bool drop_identification      = false;
        bool drop_cond_identification = false;
        {
            if (states.get< FOS::IDENTIFIED_CONTACT >()) {
                const Fred::InfoContactData &c1 = current_data;
                const Fred::InfoContactData &c2 = _new_data;
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
            if (states.get< FOS::CONDITIONALLY_IDENTIFIED_CONTACT >() ||
                (!drop_identification && states.get< FOS::IDENTIFIED_CONTACT >()))
            {
                const Fred::InfoContactData &c1 = current_data;
                const Fred::InfoContactData &c2 = _new_data;
                drop_cond_identification =
                    (c1.telephone.get_value_or_default() != c2.telephone.get_value_or_default()) ||
                    (c1.email.get_value_or_default()     != c2.email.get_value_or_default());
                drop_identification |= drop_cond_identification;
            }
            if (drop_cond_identification || drop_identification) {
                Fred::StatusList to_cancel;
                //drop conditionally identified flag if e-mail or mobile changed
                if (drop_cond_identification) {
                    to_cancel.insert(FOS(FOS::CONDITIONALLY_IDENTIFIED_CONTACT).into< std::string >());
                }
                //drop identified flag if name, mailing address, e-mail or mobile changed
                if (drop_identification) {
                    to_cancel.insert(FOS(FOS::IDENTIFIED_CONTACT).into< std::string >());
                }
                Fred::CreateObjectStateRequestId(current_data.id, to_cancel).exec(ctx);
            }

            const CheckMojeIDRegistration check_contact_data(
                Fred::make_args(_new_data),
                Fred::make_args(_new_data, ctx),
                Fred::make_args(states));

            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
            //perform changes
            Fred::UpdateContactById update_contact_op(_new_data.id, mojeid_registrar_handle_);
            set_update_contact_op(Fred::diff_contact_data(current_data, _new_data), update_contact_op);
            update_contact_op.exec(ctx);
        }
        const bool is_identified      = states.get< FOS::IDENTIFIED_CONTACT >() &&
                                        !drop_identification;
        const bool is_cond_identified = states.get< FOS::CONDITIONALLY_IDENTIFIED_CONTACT >() &&
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
        //second phase commit will change contact states
        prepare_transaction_storage()->store(_trans_id, current_data.id);

        _new_data = Fred::InfoContactById(current_data.id).exec(ctx).info_contact_data;
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        //check contact is registered, throw OBJECT_NOT_EXISTS if isn't
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).info(boost::format("request failed (%1%)") % e.what());
            throw ObjectDoesntExist("no contact associated with this handle");
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const AlreadyMojeidContact &e) {
        LOGGER(PACKAGE).info(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const ObjectAdminBlocked &e) {
        LOGGER(PACKAGE).info(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const ObjectUserBlocked &e) {
        LOGGER(PACKAGE).info(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const CheckMojeIDRegistration &e) {
        LOGGER(PACKAGE).info("request failed (UpdateTransferError)");
        throw;
    }
    catch(const MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.what());
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
    MojeID2Impl::LogRequestId _log_request_id)
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
    MojeID2Impl::LogRequestId _log_request_id = INVALID_LOG_REQUEST_ID)
{
    return set_status(_ctx, _locked_request, Fred::PublicRequest::Status::ANSWERED, _reason, _log_request_id);
}

Fred::UpdatePublicRequest::Result invalidate(
    Fred::OperationContext &_ctx,
    const Fred::PublicRequestLockGuard &_locked_request,
    const std::string &_reason = "",
    MojeID2Impl::LogRequestId _log_request_id = INVALID_LOG_REQUEST_ID)
{
    return set_status(_ctx, _locked_request, Fred::PublicRequest::Status::INVALIDATED, _reason, _log_request_id);
}

}//namespace Registry::MojeID::{anonymous}

Fred::InfoContactData& MojeID2Impl::info_contact(
        const std::string &_username,
        Fred::InfoContactData &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        _result = Fred::InfoContactByHandle(_username).exec(ctx).info_contact_data;
        ctx.commit_transaction();
        return _result;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).error("request failed (incorrect input data)");
            throw Fred::Object::Get< Fred::Object::Type::CONTACT >::object_doesnt_exist();
        }
        throw;
    }
    catch (const GetContact::object_doesnt_exist &e) {
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

MojeID2Impl::ContactId MojeID2Impl::process_registration_request(
        const std::string &_ident_request_id,
        const std::string &_password,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        typedef transitions::event::process_registration_request occurred;
        const occurred event(ctx, _ident_request_id, _password, mojeid_registrar_handle_, _log_request_id);
        try {
            transitions::process(
                event,
                GetContact(event.get_object_id()).states< occurred::RelatedStates >().presence(ctx));
            Fred::PerformObjectStateRequest(event.get_object_id()).exec(ctx);
            ctx.commit_transaction();
            return event.get_object_id();
        }
        catch (const IdentificationFailed&) {
            ctx.commit_transaction();
            throw;
        }
        catch (const ContactChanged&) {
            ctx.commit_transaction();
            throw;
        }
    }
    catch (const Fred::PublicRequestLockGuardByIdentification::Exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        if (e.is_set_public_request_doesnt_exist()) {
            throw PublicRequestDoesntExist(e.what());
        }
        throw std::runtime_error(e.what());
    }
    catch (const Fred::InfoContactById::Exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        if (e.is_set_unknown_object_id()) {
            throw IdentificationFailed("no contact associated with this public request");
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

void MojeID2Impl::process_identification_request(
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
                    throw PublicRequestDoesntExist("no public request found");
                }
                throw;
            }
        }
        typedef Fred::Object::State FOS;
        typedef FOS::set<
            FOS::SERVER_TRANSFER_PROHIBITED,
            FOS::SERVER_UPDATE_PROHIBITED,
            FOS::SERVER_DELETE_PROHIBITED,
            FOS::SERVER_BLOCKED,
            FOS::MOJEID_CONTACT,
            FOS::CONDITIONALLY_IDENTIFIED_CONTACT,
            FOS::IDENTIFIED_CONTACT >::type RelatedStates;
        typedef GetContact::States< RelatedStates >::Presence StatesPresence;
        const StatesPresence states =
            GetContact(_contact_id).states< RelatedStates >().presence(ctx);
        if (!states.get< FOS::MOJEID_CONTACT >()) {
            throw GetContact::object_doesnt_exist();
        }
        if (!states.get< FOS::CONDITIONALLY_IDENTIFIED_CONTACT >()) {
            throw std::runtime_error("state conditionallyIdentifiedContact missing");
        }
        if (states.get< FOS::IDENTIFIED_CONTACT >()) {
            throw IdentificationAlreadyProcessed("contact already identified");;
        }
        if (states.get< FOS::SERVER_BLOCKED >()) {
            throw ObjectAdminBlocked("contact administratively protected against changes");
        }
        if (!(states.get< FOS::SERVER_TRANSFER_PROHIBITED >() &&
              states.get< FOS::SERVER_UPDATE_PROHIBITED >()   &&
              states.get< FOS::SERVER_DELETE_PROHIBITED >())) {
            throw std::runtime_error("contact not protected against changes");
        }

        const Fred::InfoContactData current_data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        {
            const CheckProcessIdentificationRequest check_contact_data(current_data);
            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
        }
        Fred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
        if (!Fred::PublicRequestAuthInfo(ctx, locked_request).check_password(_password)) {
            throw IdentificationFailed("password doesn't match");
        }
        Fred::StatusList to_set;
        to_set.insert(FOS(FOS::IDENTIFIED_CONTACT).into< std::string >());
        Fred::CreateObjectStateRequestId(_contact_id, to_set).exec(ctx);
        Fred::PerformObjectStateRequest(_contact_id).exec(ctx);
        answer(ctx, locked_request, "successfully processed", _log_request_id);
        ctx.commit_transaction();
    }
    catch (const GetContact::object_doesnt_exist &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const IdentificationAlreadyProcessed &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const IdentificationFailed &e) {
        LOGGER(PACKAGE).warning(boost::format("request failed (%1%)") % e.what());
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

void MojeID2Impl::commit_prepared_transaction(const std::string &_trans_id)const
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

void MojeID2Impl::rollback_prepared_transaction(const std::string &_trans_id)const
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
        out << std::setw(2) << std::setfill('0') << std::right << ymd.day << "."   //dd.
            << std::setw(2) << std::setfill('0') << std::right << ymd.month << "." //dd.mm.
            << std::setw(0)                                    << ymd.year;        //dd.mm.yyyy
    }
    return out.str();
}

}

std::string MojeID2Impl::get_validation_pdf(ContactId _contact_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;

        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _contact_id);

        Database::Result res = ctx.get_conn().exec_params(
            "SELECT pr.id,c.name,c.organization,c.ssn,"
                   "(SELECT type FROM enum_ssntype WHERE id=c.ssntype),"
                   "CONCAT_WS(', ',"
                       "NULLIF(BTRIM(c.street1),''),NULLIF(BTRIM(c.street2),''),"
                       "NULLIF(BTRIM(c.street3),''),NULLIF(BTRIM(c.postalcode),''),"
                       "NULLIF(BTRIM(c.city),''),c.country),"
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
            throw ObjectDoesntExist("unable to generate pdf");
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
        const std::string ssn_type     = static_cast< std::string >(res[0][4]);
        const std::string address      = static_cast< std::string >(res[0][5]);
        const std::string handle       = static_cast< std::string >(res[0][6]);
        const bool is_ssn_ico =      ssn_type == "ICO";
        const std::string birthday = ssn_type == "BIRTHDAY"
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
            throw ObjectDoesntExist(e.what());
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const ObjectDoesntExist &e) {
        LOGGER(PACKAGE).warning(boost::format("request doesn't exist (%1%)") % e.what());
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
}//MojeID2Impl::get_validation_pdf

void MojeID2Impl::create_validation_request(
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
            throw ValidationRequestExists("public request already exists");
        }
        catch (const Fred::GetActivePublicRequest::Exception &e) {
            if (!e.is_set_no_request_found()) {
                throw;
            }
        }
        typedef Fred::Object::State FOS;
        typedef FOS::set<
            FOS::SERVER_TRANSFER_PROHIBITED,
            FOS::SERVER_UPDATE_PROHIBITED,
            FOS::SERVER_DELETE_PROHIBITED,
            FOS::SERVER_BLOCKED,
            FOS::MOJEID_CONTACT,
            FOS::CONDITIONALLY_IDENTIFIED_CONTACT,
            FOS::IDENTIFIED_CONTACT,
            FOS::VALIDATED_CONTACT >::type RelatedStates;
        typedef GetContact::States< RelatedStates >::Presence StatesPresence;
        const StatesPresence states = GetContact(_contact_id).states< RelatedStates >().presence(ctx);
        if (states.get< FOS::VALIDATED_CONTACT >()) {
            throw ValidationAlreadyProcessed("state validatedContact presents");
        }
        if (!states.get< FOS::MOJEID_CONTACT >()) {
            throw ObjectDoesntExist("not mojeID contact");
        }
        const Fred::InfoContactData contact_data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        {
            const CheckCreateValidationRequest check_create_validation_request(contact_data);
            if (!check_create_validation_request.success()) {
                throw check_create_validation_request;
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
            throw ObjectDoesntExist(e.what());
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const GetContact::object_doesnt_exist &e) {
        LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
        throw ObjectDoesntExist(e.what());
    }
    catch (const ObjectDoesntExist &e) {
        LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
        throw;
    }
    catch (const ValidationRequestExists &e) {
        LOGGER(PACKAGE).warning(boost::format("unable to create new request (%1%)") % e.what());
        throw;
    }
    catch (const CreateValidationRequestError &e) {
        LOGGER(PACKAGE).warning("request failed (invalid contact data)");
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

IsNotNull add_state(const Database::Value &_valid_from, ContactStateData::State _state,
                    ContactStateData &_data)
{
    if (_valid_from.isnull()) {
        return false;
    }
    const std::string db_timestamp = static_cast< std::string >(_valid_from);//2014-12-11 09:28:45.741828
    _data.set_validity(_state, boost::posix_time::time_from_string(db_timestamp));
    return true;
}

}

ContactStateDataList& MojeID2Impl::get_contacts_state_changes(
    unsigned long _last_hours,
    ContactStateDataList &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        typedef Fred::Object::State FOS;
        Fred::OperationContextCreator ctx;
        Database::query_param_list params(mojeid_registrar_handle_);             //$1::TEXT
        params(_last_hours);                                                     //$2::TEXT
        params(FOS(FOS::CONDITIONALLY_IDENTIFIED_CONTACT).into< std::string >());//$3::TEXT
        params(FOS(FOS::IDENTIFIED_CONTACT).into< std::string >());              //$4::TEXT
        params(FOS(FOS::VALIDATED_CONTACT).into< std::string >());               //$5::TEXT
        params(FOS(FOS::MOJEID_CONTACT).into< std::string >());                  //$6::TEXT
        params(FOS(FOS::LINKED).into< std::string >());                          //$7::TEXT
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
            ContactStateData data(static_cast< ContactId >(rcontacts[idx][0]));
            if (!add_state(rcontacts[idx][1], FOS::CONDITIONALLY_IDENTIFIED_CONTACT, data)) {
                std::ostringstream msg;
                msg << "contact " << data.get_contact_id() << " hasn't "
                    << FOS(FOS::CONDITIONALLY_IDENTIFIED_CONTACT).into< std::string >() << " state";
                LOGGER(PACKAGE).error(msg.str());
                continue;
            }
            add_state(rcontacts[idx][2], FOS::IDENTIFIED_CONTACT, data);
            add_state(rcontacts[idx][3], FOS::VALIDATED_CONTACT, data);
            if (!add_state(rcontacts[idx][4], FOS::MOJEID_CONTACT, data)) {
                std::ostringstream msg;
                msg << "contact " << data.get_contact_id() << " hasn't "
                    << FOS(FOS::MOJEID_CONTACT).into< std::string >() << " state";
                throw std::runtime_error(msg.str());
            }
            add_state(rcontacts[idx][5], FOS::LINKED, data);
            _result.push_back(data);
        }
        return _result;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
    return _result;
}

ContactStateData& MojeID2Impl::get_contact_state(
    ContactId _contact_id,
    ContactStateData &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        typedef Fred::Object::State FOS;
        Fred::OperationContextCreator ctx;
        Database::query_param_list params(mojeid_registrar_handle_);             //$1::TEXT
        params(_contact_id);                                                     //$2::BIGINT
        params(FOS(FOS::MOJEID_CONTACT).into< std::string >());                  //$3::TEXT
        params(FOS(FOS::CONDITIONALLY_IDENTIFIED_CONTACT).into< std::string >());//$4::TEXT
        params(FOS(FOS::IDENTIFIED_CONTACT).into< std::string >());              //$5::TEXT
        params(FOS(FOS::VALIDATED_CONTACT).into< std::string >());               //$6::TEXT
        params(FOS(FOS::LINKED).into< std::string >());                          //$7::TEXT
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
            throw ObjectDoesntExist();
        }

        if (1 < rcontact.size()) {
            std::ostringstream msg;
            msg << "contact " << _contact_id << " returns multiple (" << rcontact.size() << ") records";
            throw std::runtime_error(msg.str());
        }

        if (static_cast< bool >(rcontact[0][0])) { // contact's registrar missing
            throw ObjectDoesntExist();
        }

        _result.clear();
        _result.set_contact_id(_contact_id);
        if (!add_state(rcontact[0][1], FOS::MOJEID_CONTACT, _result)) {
            throw ObjectDoesntExist();
        }
        if (!add_state(rcontact[0][2], FOS::CONDITIONALLY_IDENTIFIED_CONTACT, _result)) {
            std::ostringstream msg;
            msg << "contact " << _contact_id << " hasn't "
                << FOS(FOS::CONDITIONALLY_IDENTIFIED_CONTACT).into< std::string >() << " state";
            throw std::runtime_error(msg.str());
        }
        add_state(rcontact[0][3], FOS::IDENTIFIED_CONTACT, _result);
        add_state(rcontact[0][4], FOS::VALIDATED_CONTACT, _result);
        add_state(rcontact[0][5], FOS::LINKED, _result);
        return _result;
    }//try
    catch (const ObjectDoesntExist &e) {
        LOGGER(PACKAGE).warning(e.what());
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

void MojeID2Impl::cancel_account_prepare(
        ContactId _contact_id,
        const std::string &_trans_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const Fred::PublicRequestObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        typedef Fred::Object::State FOS;
        typedef FOS::set<
            FOS::SERVER_TRANSFER_PROHIBITED,
            FOS::SERVER_UPDATE_PROHIBITED,
            FOS::SERVER_DELETE_PROHIBITED,
            FOS::SERVER_BLOCKED,
            FOS::MOJEID_CONTACT,
            FOS::CONDITIONALLY_IDENTIFIED_CONTACT,
            FOS::IDENTIFIED_CONTACT,
            FOS::VALIDATED_CONTACT,
            FOS::LINKED >::type RelatedStates;
        typedef GetContact::States< RelatedStates >::Presence StatesPresence;
        const StatesPresence states = GetContact(_contact_id).states< RelatedStates >().presence(ctx);
        if (!(states.get< FOS::MOJEID_CONTACT >() && (states.get< FOS::VALIDATED_CONTACT >()  ||
                                                      states.get< FOS::IDENTIFIED_CONTACT >() ||
                                                      states.get< FOS::CONDITIONALLY_IDENTIFIED_CONTACT >())
             )) {
            throw std::runtime_error("bad mojeID contact");
        }

        if (!states.get< FOS::LINKED >()) {
            Fred::DeleteContactById(_contact_id).exec(ctx);
            ctx.commit_transaction();
            return;
        }

        Fred::StatusList to_cancel;
        to_cancel.insert(FOS(FOS::MOJEID_CONTACT).into< std::string >());
        if (states.get< FOS::VALIDATED_CONTACT >()) {
            to_cancel.insert(FOS(FOS::VALIDATED_CONTACT).into< std::string >());
        }
        if (states.get< FOS::IDENTIFIED_CONTACT >()) {
            to_cancel.insert(FOS(FOS::IDENTIFIED_CONTACT).into< std::string >());
        }
        if (states.get< FOS::CONDITIONALLY_IDENTIFIED_CONTACT >()) {
            to_cancel.insert(FOS(FOS::CONDITIONALLY_IDENTIFIED_CONTACT).into< std::string >());
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
        ctx.commit_transaction();
        return;
    }
    catch (const Fred::PublicRequestObjectLockGuardByObjectId::Exception &e) {
        if (e.is_set_object_doesnt_exist()) {
            LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
            throw ObjectDoesntExist(e.what());
        }
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const GetContact::object_doesnt_exist &e) {
        LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
        throw ObjectDoesntExist(e.what());
    }
    catch (const ObjectDoesntExist &e) {
        LOGGER(PACKAGE).warning(boost::format("contact doesn't exist (%1%)") % e.what());
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

void MojeID2Impl::send_new_pin3(
    ContactId _contact_id,
    LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        typedef Fred::Object::State FOS;
        typedef transitions::event::send_new_pin3 occurred;
        const occurred::StatesPresence states =
            GetContact(_contact_id).states< occurred::RelatedStates >().presence(ctx);
        if (states.get< FOS::IDENTIFIED_CONTACT >()) {
            // nothing to send if contact is identified
            // IdentificationRequestDoesntExist isn't error in frontend
            throw IdentificationRequestDoesntExist("contact already identified");
        }
        if (!states.get< FOS::MOJEID_CONTACT >()) {
            throw ObjectDoesntExist("isn't mojeID contact");
        }
        const occurred event(ctx, _contact_id, mojeid_registrar_handle_, _log_request_id);
        transitions::process(event, states);
        ctx.commit_transaction();
        return;
    }
    catch(const ObjectDoesntExist &e) {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch(const Fred::Object::Get< Fred::Object::Type::CONTACT >::object_doesnt_exist &e) {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectDoesntExist("object not found in database");
    }
    catch(const MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch(const Fred::PublicRequestObjectLockGuardByObjectId::Exception &e) {
        if (e.is_set_object_doesnt_exist()) {
            LOGGER(PACKAGE).info(e.what());
            throw ObjectDoesntExist("object not found in database");
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch(const IdentificationRequestDoesntExist &e) {
        LOGGER(PACKAGE).info(e.what());
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

void MojeID2Impl::send_mojeid_card(
    ContactId _contact_id,
    LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        typedef Fred::Object::State FOS;
        typedef FOS::set<
            FOS::MOJEID_CONTACT,
            FOS::IDENTIFIED_CONTACT,
            FOS::VALIDATED_CONTACT >::type RelatedStates;
        typedef GetContact::States< RelatedStates >::Presence StatesPresence;
        const StatesPresence states =
            GetContact(_contact_id).states< RelatedStates >().presence(ctx);
        if (!states.get< FOS::MOJEID_CONTACT >()) {
            throw ObjectDoesntExist("isn't mojeID contact");
        }
        if (!states.get< FOS::IDENTIFIED_CONTACT >()) {
//            throw IdentificationRequestDoesntExist("contact already identified");
        }
        const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->
                                                            get_handler_ptr_by_type< HandleMojeIDArgs >();
        const Fred::InfoContactData data = Fred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        const Fred::Messages::ManagerPtr manager_ptr = Fred::Messages::create_manager();
        MojeID2Impl::send_mojeid_card(
            ctx,
            manager_ptr.get(),
            data,
            server_conf_ptr->letter_limit_count,
            server_conf_ptr->letter_limit_interval,
            _log_request_id,
            Optional< boost::posix_time::ptime >(),
            states.get< FOS::VALIDATED_CONTACT >());
        ctx.commit_transaction();
        return;
    }
    catch(const ObjectDoesntExist &e) {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch(const Fred::Object::Get< Fred::Object::Type::CONTACT >::object_doesnt_exist &e) {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectDoesntExist("object not found in database");
    }
    catch(const MessageLimitExceeded &e) {
        LOGGER(PACKAGE).info(e.what());
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

void MojeID2Impl::generate_sms_messages()const
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

void MojeID2Impl::enable_sms_messages_generation(bool enable)const
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

void MojeID2Impl::generate_letter_messages()const
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

void MojeID2Impl::enable_letter_messages_generation(bool enable)const
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

void MojeID2Impl::generate_email_messages()const
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

void MojeID2Impl::enable_email_messages_generation(bool enable)const
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

MojeID2Impl::MessageId MojeID2Impl::send_mojeid_card(
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
    std::string sql = "SELECT (SELECT country_cs FROM enum_country WHERE country=$1::TEXT)";
    if (!_validated_contact.isset()) {
        params(_data.id)
              (Fred::Object::State(Fred::Object::State::VALIDATED_CONTACT).into< std::string >());
        sql.append(",EXISTS(SELECT * FROM object_state "
                           "WHERE object_id=$2::BIGINT AND "
                                 "state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                 "valid_to IS NULL)");
    }
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    const std::string addr_country = dbres[0][0].isnull()
                                     ? pa.country
                                     : static_cast< std::string >(dbres[0][0]);
    const std::string contact_handle = _data.handle;
    const boost::gregorian::date letter_date = _letter_time.isset()
                                               ? _letter_time.get_value().date()
                                               : boost::gregorian::day_clock::local_day();
    const std::string contact_state = (_validated_contact.isset() && _validated_contact.get_value()) ||
                                      (!_validated_contact.isset() && static_cast< bool >(dbres[0][1]))
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

namespace {

transitions::event::transfer_contact_prepare::transfer_contact_prepare(
    Fred::OperationContext &_ctx,
    const Fred::InfoContactData &_contact,
    const std::string &_trans_id,
    const std::string &_registrar_handle,
    MojeID2Impl::LogRequestId _log_request_id,
    std::string &_ident)
:   ctx_(_ctx),
    contact_(_contact),
    trans_id_(_trans_id),
    registrar_handle_(_registrar_handle),
    log_request_id_(_log_request_id),
    ident_(_ident),
    locked_contact_(ctx_, _contact.id)
{
}

void transitions::guard::transfer_contact_prepare::operator()(
    const event::transfer_contact_prepare &_event,
    const event::transfer_contact_prepare::StatesPresence &_states)const
{
    const MojeID2Impl::CheckMojeIDRegistration check_result(Fred::make_args(_event.get_contact()),
                                         Fred::make_args(_event.get_contact(), _event.get_operation_context()),
                                         Fred::make_args(_states));
    if (!check_result.success()) {
        throw check_result;
    }
}

void action_transfer_contact_prepare(
    const Fred::PublicRequestAuthTypeIface &_iface,
    const transitions::event::transfer_contact_prepare &_event)
{
    Fred::CreatePublicRequestAuth op_create_pub_req(_iface);
    if (!_event.get_contact().notifyemail.isnull()) {
        op_create_pub_req.set_email_to_answer(_event.get_contact().notifyemail.get_value());
    }
    op_create_pub_req.set_registrar_id(_event.get_operation_context(), _event.get_registrar_handle());
    const Fred::CreatePublicRequestAuth::Result result =
        op_create_pub_req.exec(_event.get_operation_context(), _event.get_locked_contact());
    _event.set_ident(result.identification);
    prepare_transaction_storage()->store(_event.get_trans_id(), _event.get_object_id());
}

void transitions::action::transfer_contact_prepare_civm::operator()(
    const event::transfer_contact_prepare &_event,
    const event::transfer_contact_prepare::StatesPresence &_states)const
{
    action_transfer_contact_prepare(Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface(),
                                    _event);
}

void transitions::action::transfer_contact_prepare_Civm::operator()(
    const event::transfer_contact_prepare &_event,
    const event::transfer_contact_prepare::StatesPresence &_states)const
{
    action_transfer_contact_prepare(Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer::iface(),
                                    _event);
}

void transitions::action::transfer_contact_prepare_CIvm::operator()(
    const event::transfer_contact_prepare &_event,
    const event::transfer_contact_prepare::StatesPresence &_states)const
{
    action_transfer_contact_prepare(Fred::MojeID::PublicRequest::IdentifiedContactTransfer::iface(),
                                    _event);
}


transitions::event::process_registration_request::process_registration_request(
    Fred::OperationContext &_ctx,
    const std::string &_ident_request_id,
    const std::string &_password,
    const std::string &_registrar_handle,
    MojeID2Impl::LogRequestId _log_request_id)
:   ctx_(_ctx),
    password_(_password),
    registrar_handle_(_registrar_handle),
    log_request_id_(_log_request_id),
    locked_request_(ctx_, _ident_request_id),
    pub_req_info_(ctx_, locked_request_),
    pub_req_type_(PubReqType::from(pub_req_info_.get_type()))
{
    if (pub_req_info_.get_object_id().isnull()) {
        static const std::string msg = "no object associated with this public request";
        invalidate(ctx_, locked_request_, msg, log_request_id_);
        throw MojeID2Impl::IdentificationFailed(msg);
    }
}

void transitions::guard::process_registration_request::operator()(
    const event::process_registration_request &_event,
    const event::process_registration_request::StatesPresence &_states)const
{
    switch (_event.get_pub_req_info().get_status()) {
    case Fred::PublicRequest::Status::NEW:
        break;
    case Fred::PublicRequest::Status::ANSWERED:
        throw MojeID2Impl::IdentificationAlreadyProcessed("identification already processed");
    case Fred::PublicRequest::Status::INVALIDATED:
        throw MojeID2Impl::IdentificationAlreadyInvalidated("identification already invalidated");
    }

    switch (_event.get_pub_req_type()) {
    case PubReqType::CONTACT_CONDITIONAL_IDENTIFICATION:
    case PubReqType::CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER:
    case PubReqType::IDENTIFIED_CONTACT_TRANSFER:
        break;
    default:
        throw std::runtime_error("unexpected public request type " + _event.get_pub_req_type());
    }

    const Fred::ObjectId object_id = _event.get_object_id();
    const Database::Result dbres = _event.get_operation_context().get_conn().exec_params(
        "SELECT EXISTS(SELECT 1 FROM public_request "
                      "WHERE id=$1::BIGINT AND "
                            "create_time<(SELECT GREATEST(update,trdate) FROM object "
                                         "WHERE id=$2::BIGINT)"
                     ") AS object_changed",
        Database::query_param_list(_event.get_locked_request().get_public_request_id())//$1::BIGINT
                                  (object_id));                                        //$2::BIGINT

    if (dbres.size() != 1) {
        throw std::runtime_error("something wrong happened database is crazy");
    }

    const bool contact_changed = static_cast< bool >(dbres[0][0]);
    if (contact_changed) {
        static const std::string msg = "contact data changed after the public request had been created";
        invalidate(_event.get_operation_context(), _event.get_locked_request(), msg, _event.get_log_request_id());
        throw MojeID2Impl::ContactChanged(msg);
    }

    if (!_event.get_pub_req_info().check_password(_event.get_password())) {
        throw MojeID2Impl::IdentificationFailed("password doesn't match");
    }
}

void transitions::guard::process_contact_conditional_identification::operator()(
    const event::process_registration_request &_event,
    const event::process_registration_request::StatesPresence &_states)const
{
    if (_event.get_pub_req_type() != PubReqType::CONTACT_CONDITIONAL_IDENTIFICATION) {
        throw std::runtime_error("unexpected public request type " + _event.get_pub_req_info().get_type());
    }
    process_registration_request()(_event, _states);
}

void transitions::guard::process_conditionally_identified_contact_transfer::operator()(
    const event::process_registration_request &_event,
    const event::process_registration_request::StatesPresence &_states)const
{
    if (_event.get_pub_req_type() != PubReqType::CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER) {
        throw std::runtime_error("unexpected public request type " + _event.get_pub_req_info().get_type());
    }
    process_registration_request()(_event, _states);
}

void transitions::guard::process_identified_contact_transfer::operator()(
    const event::process_registration_request &_event,
    const event::process_registration_request::StatesPresence &_states)const
{
    if (_event.get_pub_req_type() != PubReqType::IDENTIFIED_CONTACT_TRANSFER) {
        throw std::runtime_error("unexpected public request type " + _event.get_pub_req_info().get_type());
    }
    process_registration_request()(_event, _states);
}

void transitions::action::process_contact_conditional_identification::operator()(
    const event::process_registration_request &_event,
    const event::process_registration_request::StatesPresence &_states)const
{
    try {
        const Fred::InfoContactData contact = Fred::InfoContactById(_event.get_object_id())
                                                  .exec(_event.get_operation_context()).info_contact_data;
        {
            const MojeID2Impl::CheckProcessRegistrationRequest check_contact_data(
                Fred::make_args(contact),
                Fred::make_args(contact, _event.get_operation_context()));

            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
        }

        if (_states.get< Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT >()) {
            throw MojeID2Impl::IdentificationAlreadyProcessed("contact already conditionally identified");
        }

        if (_states.get< Fred::Object::State::MOJEID_CONTACT >()) {
            throw MojeID2Impl::AlreadyMojeidContact("contact mustn't be in mojeidContact state");
        }

        if (_states.get< Fred::Object::State::SERVER_BLOCKED >()) {
            throw MojeID2Impl::ObjectAdminBlocked("contact administratively protected against changes");
        }

        if (_states.get< Fred::Object::State::SERVER_TRANSFER_PROHIBITED >() ||
            _states.get< Fred::Object::State::SERVER_UPDATE_PROHIBITED >() ||
            _states.get< Fred::Object::State::SERVER_DELETE_PROHIBITED >()) {
            throw MojeID2Impl::ObjectUserBlocked("contact protected against changes");
        }

        if (contact.sponsoring_registrar_handle != _event.get_registrar_handle()) {
            Fred::UpdateContactById op_update_contact(contact.id, _event.get_registrar_handle());
            op_update_contact.set_sponsoring_registrar(_event.get_registrar_handle());
            if (_event.get_log_request_id() != INVALID_LOG_REQUEST_ID) {
                op_update_contact.set_logd_request_id(_event.get_log_request_id());
            }
            op_update_contact.exec(_event.get_operation_context());
        }
    }
    catch (const MojeID2Impl::IdentificationAlreadyProcessed &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::AlreadyMojeidContact &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::ObjectAdminBlocked &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::ObjectUserBlocked &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    answer(_event.get_operation_context(), _event.get_locked_request(), "process_registration_request call",
           _event.get_log_request_id());
}

void transitions::action::process_conditionally_identified_contact_transfer::operator()(
    const event::process_registration_request &_event,
    const event::process_registration_request::StatesPresence &_states)const
{
    try {
        const Fred::InfoContactData contact = Fred::InfoContactById(_event.get_object_id())
                                                  .exec(_event.get_operation_context()).info_contact_data;
    }
    catch (const MojeID2Impl::IdentificationAlreadyProcessed &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::AlreadyMojeidContact &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::ObjectAdminBlocked &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::ObjectUserBlocked &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    answer(_event.get_operation_context(), _event.get_locked_request(), "process_registration_request call",
           _event.get_log_request_id());
}

void transitions::action::process_identified_contact_transfer::operator()(
    const event::process_registration_request &_event,
    const event::process_registration_request::StatesPresence &_states)const
{
    try {
        const Fred::InfoContactData contact = Fred::InfoContactById(_event.get_object_id())
                                                  .exec(_event.get_operation_context()).info_contact_data;
    }
    catch (const MojeID2Impl::IdentificationAlreadyProcessed &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::AlreadyMojeidContact &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::ObjectAdminBlocked &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    catch (const MojeID2Impl::ObjectUserBlocked &e) {
        invalidate(_event.get_operation_context(), _event.get_locked_request(), e.what(), _event.get_log_request_id());
        throw;
    }
    answer(_event.get_operation_context(), _event.get_locked_request(), "process_registration_request call",
           _event.get_log_request_id());
}

transitions::event::send_new_pin3::send_new_pin3(
    Fred::OperationContext &_ctx,
    MojeID2Impl::ContactId _contact_id,
    const std::string &_registrar_handle,
    MojeID2Impl::LogRequestId _log_request_id)
:   ctx_(_ctx),
    contact_id_(_contact_id),
    locked_object_(_ctx, _contact_id),
    registrar_handle_(_registrar_handle),
    log_request_id_(_log_request_id)
{
}

void transitions::guard::send_new_pin3::operator()(
    const event::send_new_pin3 &_event,
    const event::send_new_pin3::StatesPresence &_states)const
{
}

void transitions::action::send_new_pin3::operator()(
    const event::send_new_pin3 &_event,
    const event::send_new_pin3::StatesPresence &_states)const
{
    bool has_identification_request = false;
    try {
        const Fred::PublicRequestTypeIface &type = Fred::MojeID::PublicRequest::ContactIdentification::iface();
        Fred::GetActivePublicRequest get_active_public_request_op(type);
        Fred::OperationContext &ctx = _event.get_operation_context();
        const Fred::PublicRequestObjectLockGuard &locked_object = _event.get_locked_object();
        while (true) {
            const Fred::PublicRequestId request_id = get_active_public_request_op.exec(ctx, locked_object);
            Fred::UpdatePublicRequest update_public_request_op;
            Fred::PublicRequestLockGuardById locked_request(ctx, request_id);
            update_public_request_op.set_status(Fred::PublicRequest::Status::INVALIDATED);
            update_public_request_op.set_reason("new pin3 generated");
            update_public_request_op.set_registrar_id(ctx, _event.get_registrar_handle());
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
        Fred::OperationContext &ctx = _event.get_operation_context();
        const Fred::PublicRequestObjectLockGuard &locked_object = _event.get_locked_object();
        while (true) {
            const Fred::PublicRequestId request_id = get_active_public_request_op.exec(ctx, locked_object);
            Fred::UpdatePublicRequest update_public_request_op;
            Fred::PublicRequestLockGuardById locked_request(ctx, request_id);
            update_public_request_op.set_status(Fred::PublicRequest::Status::INVALIDATED);
            update_public_request_op.set_reason("new pin3 generated");
            update_public_request_op.set_registrar_id(ctx, _event.get_registrar_handle());
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
        throw MojeID2Impl::IdentificationRequestDoesntExist("no usable request found");
    }

    Fred::OperationContext &ctx = _event.get_operation_context();
    const HandleMojeIDArgs *const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >();
    check_sent_letters_limit(ctx,
                             _event.get_object_id(),
                             server_conf_ptr->letter_limit_count,
                             server_conf_ptr->letter_limit_interval);

    const Fred::PublicRequestAuthTypeIface &type = has_reidentification_request
                                                   ? Fred::MojeID::PublicRequest::ContactReidentification::iface()
                                                   : Fred::MojeID::PublicRequest::ContactIdentification::iface();
    Fred::CreatePublicRequestAuth create_public_request_op(type);
    create_public_request_op.set_registrar_id(ctx, _event.get_registrar_handle());
    create_public_request_op.set_reason("send_new_pin3 call");
    const Fred::CreatePublicRequestAuth::Result result =
        create_public_request_op.exec(ctx, _event.get_locked_object(), _event.get_log_request_id());
}

}//namespace Registry::MojeID::{anonymous}
}//namespace Registry::MojeID
}//namespace Registry
