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

#include "src/mojeid/state_machine.h"
#include "src/mojeid/mojeid2.h"
#include "src/mojeid/safe_data_storage.h"
#include "src/mojeid/mojeid_public_request.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/public_request/create_public_request_auth.h"
#include "src/fredlib/public_request/info_public_request_auth.h"
#include "src/fredlib/public_request/update_public_request.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/public_request/get_active_public_request.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/get_states_presence.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include "util/random.h"
#include "util/log/context.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/cfg/config_handler_decl.h"

#include <algorithm>
#include <map>

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
            const Fred::PublicRequestObjectLockGuard locked_contact_;
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
            const Fred::PublicRequestObjectLockGuard locked_object_;
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
            const CheckCreateContactPrepare check_contact_data(Fred::make_args(_contact),
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
        Fred::PublicRequestObjectLockGuard locked_contact(ctx, new_contact.object_id);
        {
            const Fred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(ctx, locked_contact);
            _ident = result.identification;
        }
        prepare_transaction_storage()->store(_trans_id, new_contact.object_id);
        ctx.commit_transaction();
        return new_contact.object_id;
    }
    catch (const CreateContactPrepareError&) {
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
    catch (const TransferContactPrepareError&) {
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
        }
        catch (const IdentificationFailed&) {
            ctx.commit_transaction();
            throw;
        }
        catch (const ContactChanged&) {
            ctx.commit_transaction();
            throw;
        }
        return event.get_object_id();
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

void MojeID2Impl::send_new_pin3(
    ContactId _contact_id,
    LogRequestId _log_request_id)const
{
    try {
        Fred::OperationContextCreator ctx;
        typedef Fred::Object::State FOS;
        typedef transitions::event::send_new_pin3 occurred;
        const GetContact::States< occurred::RelatedStates >::Presence states =
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
    catch(const Fred::PublicRequestObjectLockGuard::Exception &e) {
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
    try {
        Fred::OperationContextCreator ctx;
        typedef Fred::Object::State FOS;
        typedef FOS::set<
            FOS::MOJEID_CONTACT,
            FOS::IDENTIFIED_CONTACT >::type RelatedStates;
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
        check_sent_letters_limit(ctx,
                                 _contact_id,
                                 server_conf_ptr->letter_limit_count,
                                 server_conf_ptr->letter_limit_interval);
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
    const MojeID2Impl::CheckTransferContactPrepare check_result(Fred::make_args(_event.get_contact()),
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
            const MojeID2Impl::CheckCreateContactPrepare check_contact_data(
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
        const Fred::PublicRequestObjectLockGuard locked_object = _event.get_locked_object();
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
        const Fred::PublicRequestObjectLockGuard locked_object = _event.get_locked_object();
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
