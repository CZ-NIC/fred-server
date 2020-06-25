/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
#include "test/backend/epp/poll/fixture.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/backend/epp/util.hh"
#include "libfred/poll/create_update_object_poll_message.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/registrable_object/domain/transfer_domain.hh"
#include "libfred/registrable_object/contact/transfer_contact.hh"
#include "libfred/registrable_object/nsset/transfer_nsset.hh"
#include "libfred/registrable_object/keyset/transfer_keyset.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/object/object_type.hh"
#include "libfred/poll/create_poll_message.hh"
#include "libfred/poll/create_request_fee_info_message.hh"
#include "src/backend/epp/poll/poll_request.hh"
#include "src/backend/epp/poll/message_type.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/test/unit_test.hpp>
#include <boost/variant.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)

namespace {

boost::gregorian::date get_erase_date_by_handle(
    ::LibFred::OperationContext& _ctx,
    const std::string& _handle,
    ::LibFred::Object_Type::Enum _type)
{
    Database::ParamQuery sql_query;
    if (_type == ::LibFred::Object_Type::domain)
    {
        sql_query("SELECT MAX(erdate) FROM object_registry WHERE name=LOWER(");
    }
    else
    {
        sql_query("SELECT MAX(erdate) FROM object_registry WHERE UPPER(name)=UPPER(");
    }
    sql_query.param_text(_handle)(") AND type=get_object_type_id(")
             .param_text(Conversion::Enums::to_db_handle(_type))(")");

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    const bool is_null = sql_query_result[0][0].isnull();
    BOOST_REQUIRE(!is_null);
    return boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
}

unsigned long long get_request_id_by_history_id(::LibFred::OperationContext& _ctx, unsigned long long _history_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT request_id FROM history WHERE id=").param_bigint(_history_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    return static_cast<unsigned long long>(sql_query_result[0][0]);
}

boost::gregorian::date get_transfer_date_by_message_id(::LibFred::OperationContext& _ctx, unsigned long long _message_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT oh.trdate::date "
              "FROM poll_eppaction pe JOIN object_history oh ON oh.historyid=pe.objid "
              "WHERE pe.msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    return boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
}

unsigned long long create_message_and_get_message_id(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _registrar_id,
    Epp::Poll::MessageType::Enum _type)
{
    Database::ParamQuery sql_query;
    sql_query("INSERT INTO message (id, clid, crdate, exdate, msgtype) "
              "VALUES (nextval('message_id_seq'::regclass), ").param_bigint(_registrar_id)(", ")
             ("current_timestamp, current_timestamp + interval '7 days', "
              "(SELECT id FROM messagetype WHERE name=")
             .param_text(Conversion::Enums::to_db_handle(_type))(")) RETURNING id");

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);

    return static_cast<unsigned long long>(sql_query_result[0][0]);
}

unsigned long long create_poll_object_state_record_validation(
    ::LibFred::OperationContext& _ctx,
    const std::string& _state,
    unsigned long long _domain_id)
{
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO object_state (object_id, state_id, valid_from, valid_to, ohid_from, ohid_to) "
        "SELECT eh.domainid, (SELECT id FROM enum_object_states WHERE name=$1::text), "
        "NOW()-'2MONTHS'::INTERVAL, NULL, eh.historyid, NULL "
        "FROM enumval_history eh "
        "WHERE domainid=$2::integer "
        "ORDER BY historyid DESC "
        "LIMIT 1 "
        "RETURNING id",
        Database::query_param_list
        (_state)
        (_domain_id));

    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);

    return static_cast<unsigned long long>(sql_query_result[0][0]);
}

struct StateRecordData
{
    unsigned long long id;
    boost::gregorian::date date;
};

StateRecordData create_poll_object_state_record_rest(
    ::LibFred::OperationContext& _ctx,
    const std::string& _state,
    unsigned long long _object_id)
{
    StateRecordData ret;

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO object_state (object_id, state_id, valid_from, valid_to, ohid_from, ohid_to) "
        "SELECT oh.id, (SELECT id FROM enum_object_states WHERE name=$1::text), "
        "NOW()-'2MONTHS'::INTERVAL, NULL, oh.historyid, NULL "
        "FROM object_history oh "
        "WHERE id=$2::integer "
        "ORDER BY historyid DESC "
        "LIMIT 1 "
        "RETURNING id, valid_from::date",
        Database::query_param_list
        (_state)
        (_object_id));

    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);

    ret.id = static_cast<unsigned long long>(sql_query_result[0][0]);
    ret.date = boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][1]));
    return ret;
}

void create_poll_statechange_record(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _state_change_id)
{
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO poll_statechange "
        "(msgid, stateid) "
        "VALUES ($1::integer, $2::integer)",
        Database::query_param_list
        (_message_id)
        (_state_change_id));

    BOOST_REQUIRE_EQUAL(sql_query_result.rows_affected(), 1);
}

unsigned long long create_poll_low_credit_message(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _registrar_id,
    const std::string& _zone,
    const Decimal& _limit,
    const Decimal& _credit)
{
    const unsigned long long poll_msg_id =
        create_message_and_get_message_id(_ctx, _registrar_id, ::Epp::Poll::MessageType::credit);

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO poll_credit "
        "(msgid, zone, credlimit, credit) "
        "VALUES ($1::integer, (SELECT id FROM zone WHERE fqdn=$2::text), "
        "$3::numeric(10,2), $4::numeric(10,2))",
        Database::query_param_list
        (poll_msg_id)
        (_zone)
        (_limit)
        (_credit));

    BOOST_REQUIRE_EQUAL(sql_query_result.rows_affected(), 1);

    return poll_msg_id;
}

unsigned long long create_check_nsset_record(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _object_id)
{
   const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO check_nsset (nsset_hid, reason, overallstatus, extra_fqdns, dig, attempt) "
        "VALUES ((SELECT historyid FROM nsset_history "
        "WHERE id=$1::integer AND checklevel=3 ORDER BY historyid LIMIT 1), "
        "1, 1, '{}', false, 1) RETURNING id",
        Database::query_param_list
        (_object_id));

    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);

    return static_cast<unsigned long long>(sql_query_result[0][0]);
}

std::vector<Epp::Poll::Test> create_check_result_record(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _check_nsset_id)
{
    std::vector<Epp::Poll::Test> ret;

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO check_result (checkid, testid, status) "
        "SELECT $1::integer, id, 0 FROM check_test RETURNING "
        "(SELECT name FROM check_test WHERE id=testid)",
        Database::query_param_list
        (_check_nsset_id));

    BOOST_REQUIRE(sql_query_result.size() > 0);

    for (std::size_t idx = 0; idx < sql_query_result.size(); ++idx)
    {
        const std::string testname = static_cast<std::string>(sql_query_result[idx][0]);
        const int status = 0;
        const std::string note;
        ret.push_back(Epp::Poll::Test(testname, note, status));
    }

    return ret;
}

void create_poll_techeck_record(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _check_nsset_id)
{
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO poll_techcheck (msgid, cnid) VALUES ($1::integer, $2::integer)",
        Database::query_param_list
        (_message_id)
        (_check_nsset_id));

    BOOST_REQUIRE_EQUAL(sql_query_result.rows_affected(), 1);
}

// fixtures

struct HasPollUpdateContactMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateContactMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::contact contact(ctx);
        const Test::registrar different_registrar(ctx);
        // update done not by contact's sponsoring registar
        history_id = ::LibFred::UpdateContactByHandle(contact.info_data.handle,
                                        different_registrar.info_data.handle
            ).set_authinfo("doesntmatter").exec(ctx);
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, history_id);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::UpdateContact> SubMessage;
};

struct HasPollUpdateDomainMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateDomainMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::domain domain(ctx);
        history_id = ::LibFred::UpdateDomain(domain.info_data.fqdn,
                                        domain.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, history_id);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::UpdateDomain> SubMessage;
};

struct HasPollUpdateNssetMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateNssetMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::nsset nsset(ctx);
        history_id = ::LibFred::UpdateNsset(nsset.info_data.handle,
                                       nsset.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, history_id);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::UpdateNsset> SubMessage;
};

struct HasPollUpdateKeysetMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateKeysetMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::keyset keyset(ctx);
        history_id = ::LibFred::UpdateKeyset(keyset.info_data.handle,
                                        keyset.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, history_id);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::UpdateKeyset> SubMessage;
};

template<typename T>
struct HasPollUpdate : T
{
    void test()
    {
        namespace ep = Epp::Poll;
        typedef typename T::SubMessage SubMessage;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        const Test::MessageDetail message_detail = Test::get_message_ids(T::ctx);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(T::ctx, message_detail.registrar_id));

        ep::UpdateInfoEvent update_info_event;
        BOOST_CHECK_NO_THROW(update_info_event = boost::get<ep::UpdateInfoEvent>(output.message));
        SubMessage update_info;
        BOOST_CHECK_NO_THROW(update_info = boost::get<SubMessage>(update_info_event.message));

        BOOST_CHECK_EQUAL(update_info.transaction_id, get_request_id_by_history_id(T::ctx, T::history_id));
        BOOST_CHECK_EQUAL(update_info.poll_id, message_detail.message_id);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

struct HasPollTransferDomainMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferDomainMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::domain domain(ctx);
        const Test::registrar registrar(ctx);

        object_handle = domain.info_data.fqdn;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = ::LibFred::TransferDomain(domain.info_data.id,
                                                                   registrar.info_data.handle,
                                                                   domain.info_data.authinfopw,
                                                                   Nullable<unsigned long long>()).exec(ctx);

        ::LibFred::Poll::CreatePollMessage<::LibFred::Poll::MessageType::transfer_domain>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::TransferDomain> SubMessage;
};

struct HasPollTransferContactMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferContactMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::contact contact(ctx);
        const Test::registrar registrar(ctx);

        object_handle = contact.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = ::LibFred::TransferContact(contact.info_data.id,
                                                                    registrar.info_data.handle,
                                                                    contact.info_data.authinfopw,
                                                                    Nullable<unsigned long long>()).exec(ctx);

        ::LibFred::Poll::CreatePollMessage<::LibFred::Poll::MessageType::transfer_contact>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::TransferContact> SubMessage;
};

struct HasPollTransferNssetMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferNssetMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::nsset nsset(ctx);
        const Test::registrar registrar(ctx);

        object_handle = nsset.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = ::LibFred::TransferNsset(nsset.info_data.id,
                                                                  registrar.info_data.handle,
                                                                  nsset.info_data.authinfopw,
                                                                  Nullable<unsigned long long>()).exec(ctx);

        ::LibFred::Poll::CreatePollMessage<::LibFred::Poll::MessageType::transfer_nsset>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::TransferNsset> SubMessage;
};

struct HasPollTransferKeysetMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferKeysetMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::keyset keyset(ctx);
        const Test::registrar registrar(ctx);

        object_handle = keyset.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = ::LibFred::TransferKeyset(keyset.info_data.id,
                                                                   registrar.info_data.handle,
                                                                   keyset.info_data.authinfopw,
                                                                   Nullable<unsigned long long>()).exec(ctx);

        ::LibFred::Poll::CreatePollMessage<::LibFred::Poll::MessageType::transfer_keyset>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::TransferKeyset> SubMessage;
};

template<typename T>
struct HasPollTransfer : T
{
    void test()
    {
        namespace ep = Epp::Poll;
        typedef typename T::SubMessage SubMessage;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        const Test::MessageDetail message_detail = Test::get_message_ids(T::ctx);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(T::ctx, message_detail.registrar_id));

        ep::TransferEvent transfer_event;
        BOOST_CHECK_NO_THROW(transfer_event = boost::get<ep::TransferEvent>(output.message));
        SubMessage transfer_info;
        BOOST_CHECK_NO_THROW(transfer_info = boost::get<SubMessage>(transfer_event.message));

        const boost::gregorian::date real_transfer_date = get_transfer_date_by_message_id(T::ctx, message_detail.message_id);

        BOOST_CHECK_EQUAL(transfer_info.transfer_date, real_transfer_date);
        BOOST_CHECK_EQUAL(transfer_info.object_handle, T::object_handle);
        BOOST_CHECK_EQUAL(transfer_info.dst_registrar_handle, T::dst_registrar_handle);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

struct HasPollRequestFeeInfoMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    unsigned long long message_id;
    unsigned long long registrar_id;
    Epp::Poll::RequestFeeInfoEvent golden_request_fee_info_event;

    HasPollRequestFeeInfoMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        registrar_id = registrar.info_data.id;
        golden_request_fee_info_event.from = boost::posix_time::time_from_string("2017-03-30 14:16:02.713506");
        golden_request_fee_info_event.to = boost::posix_time::time_from_string("2017-03-30 14:17:07.113492");

        golden_request_fee_info_event.free_count = 20;
        golden_request_fee_info_event.used_count = 10;
        golden_request_fee_info_event.price = "1024.42";

        // sadly the timestamp is converted from local time to utc when stored, but not converted back when fetched
        typedef boost::date_time::local_adjustor<boost::posix_time::ptime, +2, boost::posix_time::no_dst> cest;

        message_id = ::LibFred::Poll::CreateRequestFeeInfoMessage(
            registrar_id,
            cest::utc_to_local(golden_request_fee_info_event.from),
            cest::utc_to_local(golden_request_fee_info_event.to),
            golden_request_fee_info_event.free_count,
            golden_request_fee_info_event.used_count,
            golden_request_fee_info_event.price,
            "Europe/Prague").exec(ctx);
    }

    void test()
    {
        namespace ep = Epp::Poll;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(ctx, registrar_id));

        ep::RequestFeeInfoEvent request_fee_info_event;
        BOOST_CHECK_NO_THROW(request_fee_info_event = boost::get<ep::RequestFeeInfoEvent>(output.message));

        BOOST_CHECK_EQUAL(golden_request_fee_info_event.from, request_fee_info_event.from);
        BOOST_CHECK_EQUAL(golden_request_fee_info_event.to, request_fee_info_event.to);
        BOOST_CHECK_EQUAL(golden_request_fee_info_event.free_count, request_fee_info_event.free_count);
        BOOST_CHECK_EQUAL(golden_request_fee_info_event.used_count, request_fee_info_event.used_count);
        BOOST_CHECK_EQUAL(golden_request_fee_info_event.price, request_fee_info_event.price);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

struct HasPollRequestLowCreditMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    unsigned long long message_id;
    unsigned long long registrar_id;
    Epp::Poll::LowCreditEvent golden_low_credit_event;

    HasPollRequestLowCreditMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        registrar_id = registrar.info_data.id;
        golden_low_credit_event.zone = "cz";
        golden_low_credit_event.limit = "1024.42";
        golden_low_credit_event.credit = "512.07";

        message_id = create_poll_low_credit_message(ctx,
                                                    registrar_id,
                                                    golden_low_credit_event.zone,
                                                    golden_low_credit_event.limit,
                                                    golden_low_credit_event.credit);
    }

    void test()
    {
        namespace ep = Epp::Poll;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(ctx, registrar_id));

        ep::LowCreditEvent low_credit_event;
        BOOST_CHECK_NO_THROW(low_credit_event = boost::get<ep::LowCreditEvent>(output.message));

        BOOST_CHECK_EQUAL(golden_low_credit_event.zone, low_credit_event.zone);
        BOOST_CHECK_EQUAL(golden_low_credit_event.credit, low_credit_event.credit);
        BOOST_CHECK_EQUAL(golden_low_credit_event.limit, low_credit_event.limit);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

struct HasPollDeleteDomainMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollDeleteDomainMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::domain domain(ctx);
        handle = domain.info_data.fqdn;
        unsigned long long history_id =
            ::LibFred::InfoDomainByFqdn(domain.info_data.fqdn).exec(ctx).info_domain_data.historyid;
        ::LibFred::DeleteDomainByFqdn(domain.info_data.fqdn).exec(ctx);

        date = get_erase_date_by_handle(ctx, handle, ::LibFred::Object_Type::domain);

        ::LibFred::Poll::CreatePollMessage<::LibFred::Poll::MessageType::delete_domain>().exec(ctx, history_id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::DeleteDomain> SubMessage;
};

struct HasPollDeleteContactMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollDeleteContactMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::contact contact(ctx);
        handle = contact.info_data.handle;
        unsigned long long history_id =
            ::LibFred::InfoContactByHandle(contact.info_data.handle).exec(ctx).info_contact_data.historyid;
        ::LibFred::DeleteContactByHandle(contact.info_data.handle).exec(ctx);

        date = get_erase_date_by_handle(ctx, handle, ::LibFred::Object_Type::contact);

        ::LibFred::Poll::CreatePollMessage<::LibFred::Poll::MessageType::delete_contact>().exec(ctx, history_id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::DeleteContact> SubMessage;
};

struct HasPollValidationMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollValidationMessage() :
        handle("1.2.3.4.5.6.7.8.9.0.2.4.e164.arpa"),
        date(boost::posix_time::second_clock::local_time().date())
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        const Test::contact contact(ctx);
        const ::LibFred::CreateDomain::Result result =
            ::LibFred::CreateDomain(handle, registrar.info_data.handle, contact.info_data.handle)
            .set_enum_validation_expiration(date).exec(ctx, "Europe/Prague");
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar.info_data.id, ::Epp::Poll::MessageType::validation);
        const unsigned long long state_change_id =
            create_poll_object_state_record_validation(ctx, "notValidated", result.create_object_result.object_id);
        create_poll_statechange_record(ctx, poll_msg_id, state_change_id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::Validation> SubMessage;
};

struct HasPollImpValidationMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollImpValidationMessage() :
        handle("1.2.3.4.5.6.7.8.9.0.2.4.e164.arpa"),
        date(boost::posix_time::second_clock::local_time().date() + boost::gregorian::days(7))
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        const Test::contact contact(ctx);
        const ::LibFred::CreateDomain::Result result =
            ::LibFred::CreateDomain(handle, registrar.info_data.handle, contact.info_data.handle)
            .set_enum_validation_expiration(date).exec(ctx, "Europe/Prague");
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar.info_data.id, Epp::Poll::MessageType::imp_validation);
        const unsigned long long state_change_id =
            create_poll_object_state_record_validation(ctx, "validationWarning1", result.create_object_result.object_id);
        create_poll_statechange_record(ctx, poll_msg_id, state_change_id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::ImpValidation> SubMessage;
};

struct HasPollExpirationMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollExpirationMessage() :
        handle("expirationxxxxxxaxxdxefxfxxxxeca.cz"),
        date(boost::posix_time::second_clock::local_time().date())
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        const Test::contact contact(ctx);
        const ::LibFred::CreateDomain::Result result =
            ::LibFred::CreateDomain(handle, registrar.info_data.handle, contact.info_data.handle)
            .set_expiration_date(date).exec(ctx, "Europe/Prague");
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar.info_data.id, Epp::Poll::MessageType::expiration);
        const StateRecordData state_record_data =
            create_poll_object_state_record_rest(ctx, "expired", result.create_object_result.object_id);
        create_poll_statechange_record(ctx, poll_msg_id, state_record_data.id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::Expiration> SubMessage;
};

struct HasPollImpExpirationMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollImpExpirationMessage() :
        handle("impendingexpirationdxefxfxxxxecb.cz"),
        date(boost::posix_time::second_clock::local_time().date() + boost::gregorian::days(7))
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        const Test::contact contact(ctx);
        const ::LibFred::CreateDomain::Result result =
            ::LibFred::CreateDomain(handle, registrar.info_data.handle, contact.info_data.handle)
            .set_expiration_date(date).exec(ctx, "Europe/Prague");
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar.info_data.id, Epp::Poll::MessageType::imp_expiration);
        const StateRecordData state_record_data =
            create_poll_object_state_record_rest(ctx, "expirationWarning", result.create_object_result.object_id);
        create_poll_statechange_record(ctx, poll_msg_id, state_record_data.id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::ImpExpiration> SubMessage;
};

struct HasPollIdleDeleteDomainMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollIdleDeleteDomainMessage() :
        handle("idledeletexxxxxxaxxdxefxfxxxxecc.cz"),
        date(boost::posix_time::second_clock::local_time().date() - boost::gregorian::days(25))
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        const Test::contact contact(ctx);
        const ::LibFred::CreateDomain::Result result =
            ::LibFred::CreateDomain(handle, registrar.info_data.handle, contact.info_data.handle)
            .set_expiration_date(date).exec(ctx, "Europe/Prague");
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar.info_data.id, Epp::Poll::MessageType::idle_delete_domain);
        const StateRecordData state_record_data =
            create_poll_object_state_record_rest(ctx, "deleteCandidate", result.create_object_result.object_id);
        create_poll_statechange_record(ctx, poll_msg_id, state_record_data.id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::IdleDeleteDomain> SubMessage;
};

struct HasPollIdleDeleteContactMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollIdleDeleteContactMessage() :
        handle("IDLEDELETECONTACTCONTACTPGKGCNEBOCO")
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::contact contact(ctx, handle);
        const unsigned long long registrar_id =
            ::LibFred::InfoRegistrarByHandle(contact.info_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id;
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar_id, Epp::Poll::MessageType::idle_delete_contact);
        const StateRecordData state_record_data =
            create_poll_object_state_record_rest(ctx, "deleteCandidate", contact.info_data.id);
        date = state_record_data.date;
        create_poll_statechange_record(ctx, poll_msg_id, state_record_data.id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::IdleDeleteContact> SubMessage;
};

struct HasPollIdleDeleteNssetMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollIdleDeleteNssetMessage() :
        handle("IDLEDELETENSSETNSSETPGKGCNEBOCO")
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::nsset nsset(ctx, handle);
        const unsigned long long registrar_id =
            ::LibFred::InfoRegistrarByHandle(nsset.info_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id;
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar_id, Epp::Poll::MessageType::idle_delete_nsset);
        const StateRecordData state_record_data =
            create_poll_object_state_record_rest(ctx, "deleteCandidate", nsset.info_data.id);
        date = state_record_data.date;
        create_poll_statechange_record(ctx, poll_msg_id, state_record_data.id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::IdleDeleteNsset> SubMessage;
};

struct HasPollIdleDeleteKeysetMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollIdleDeleteKeysetMessage() :
        handle("IDLEDELETEKEYSETKEYSETPGKGCNEBOCO")
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::keyset keyset(ctx, handle);
        const unsigned long long registrar_id =
            ::LibFred::InfoRegistrarByHandle(keyset.info_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id;
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar_id, Epp::Poll::MessageType::idle_delete_keyset);
        const StateRecordData state_record_data =
            create_poll_object_state_record_rest(ctx, "deleteCandidate", keyset.info_data.id);
        date = state_record_data.date;
        create_poll_statechange_record(ctx, poll_msg_id, state_record_data.id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::IdleDeleteKeyset> SubMessage;
};

struct HasPollOutzoneUnguardedMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    std::string handle;
    boost::gregorian::date date;

    HasPollOutzoneUnguardedMessage() :
        handle("outzoneoutguardedxxdxefxfxxxxecc.cz"),
        date(boost::posix_time::second_clock::local_time().date() - boost::gregorian::days(90))
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        const Test::contact contact(ctx);
        const ::LibFred::CreateDomain::Result result =
            ::LibFred::CreateDomain(handle, registrar.info_data.handle, contact.info_data.handle)
            .set_expiration_date(date).exec(ctx, "Europe/Prague");
        const unsigned long long poll_msg_id =
            create_message_and_get_message_id(ctx, registrar.info_data.id, Epp::Poll::MessageType::outzone);
        const StateRecordData state_record_data =
            create_poll_object_state_record_rest(ctx, "outzoneUnguarded", result.create_object_result.object_id);
        create_poll_statechange_record(ctx, poll_msg_id, state_record_data.id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::Outzone> SubMessage;
};

template<typename T>
struct HasPollMessage : T
{
    void test()
    {
        namespace ep = Epp::Poll;
        typedef typename T::SubMessage SubMessage;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        const Test::MessageDetail message_detail = Test::get_message_ids(T::ctx);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(T::ctx, message_detail.registrar_id));

        ep::MessageEvent message_event;
        BOOST_CHECK_NO_THROW(message_event = boost::get<ep::MessageEvent>(output.message));
        SubMessage message_info;
        BOOST_CHECK_NO_THROW(message_info = boost::get<SubMessage>(message_event.message));

        BOOST_CHECK_EQUAL(message_info.date, T::date);
        BOOST_CHECK_EQUAL(message_info.handle, T::handle);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

struct HasPollTechCheckMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    Epp::Poll::TechCheckEvent golden;
    const std::string name;

    HasPollTechCheckMessage() : name("MYOWNNSSETILSHIBAMF")
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::registrar registrar(ctx);
        const ::LibFred::CreateNsset::Result result =
            ::LibFred::CreateNsset(name, registrar.info_data.handle)
            .set_tech_check_level(3).exec(ctx);

        const unsigned long long message_id =
            create_message_and_get_message_id(ctx, registrar.info_data.id, ::Epp::Poll::MessageType::techcheck);
        const unsigned long long check_nsset_id =
            create_check_nsset_record(ctx, result.create_object_result.object_id);

        golden.handle = name;
        golden.tests = create_check_result_record(ctx, check_nsset_id);
        create_poll_techeck_record(ctx, message_id, check_nsset_id);
    }

    void test()
    {
        namespace ep = Epp::Poll;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        const Test::MessageDetail message_detail = Test::get_message_ids(ctx);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(ctx, message_detail.registrar_id));

        ep::TechCheckEvent tech_check_event;
        BOOST_CHECK_NO_THROW(tech_check_event = boost::get<ep::TechCheckEvent>(output.message));

        BOOST_CHECK_EQUAL(tech_check_event.handle, golden.handle);

        struct TestComparator
        {
            static bool less(const Epp::Poll::Test& _a, const Epp::Poll::Test& _b)
            {
                return _a.testname < _b.testname;
            }
        };

        BOOST_CHECK_EQUAL(tech_check_event.tests.size(), golden.tests.size());
        std::sort(tech_check_event.tests.begin(), tech_check_event.tests.end(), &TestComparator::less);
        std::sort(golden.tests.begin(), golden.tests.end(), &TestComparator::less);
        for (std::vector<Epp::Poll::Test>::const_iterator
                 test_itr = tech_check_event.tests.begin(),
                 golden_itr = golden.tests.begin();
             test_itr != tech_check_event.tests.end() && golden_itr != golden.tests.end();
             ++test_itr, ++golden_itr)
        {
            BOOST_CHECK_EQUAL(test_itr->testname, golden_itr->testname);
            BOOST_CHECK_EQUAL(test_itr->get_status(), golden_itr->get_status());
            BOOST_CHECK_EQUAL(test_itr->note, golden_itr->note);
        }

        BOOST_CHECK_EQUAL(tech_check_event.fqdns.size(), golden.fqdns.size());
        std::sort(tech_check_event.fqdns.begin(), tech_check_event.fqdns.end());
        std::sort(golden.fqdns.begin(), golden.fqdns.end());
        for (std::vector<std::string>::const_iterator
                 fqdn_itr = tech_check_event.fqdns.begin(),
                 golden_itr = golden.fqdns.begin();
             fqdn_itr != tech_check_event.fqdns.end() && golden_itr != golden.fqdns.end();
             ++fqdn_itr, ++golden_itr)
        {
            BOOST_CHECK_EQUAL(*fqdn_itr, *golden_itr);
        }


        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(request_contact_update_message, HasPollUpdate<HasPollUpdateContactMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_domain_update_message, HasPollUpdate<HasPollUpdateDomainMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_nsset_update_message, HasPollUpdate<HasPollUpdateNssetMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_keyset_update_message, HasPollUpdate<HasPollUpdateKeysetMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_domain_transfer_message, HasPollTransfer<HasPollTransferDomainMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_contact_transfer_message, HasPollTransfer<HasPollTransferContactMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_nsset_transfer_message, HasPollTransfer<HasPollTransferNssetMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_keyset_transfer_message, HasPollTransfer<HasPollTransferKeysetMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_domain_delete_message, HasPollMessage<HasPollDeleteDomainMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_contact_delete_message, HasPollMessage<HasPollDeleteContactMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_fee_info_message, HasPollRequestFeeInfoMessage)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_low_credit_message, HasPollRequestLowCreditMessage)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_validation_message, HasPollMessage<HasPollValidationMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_imp_validation_message, HasPollMessage<HasPollImpValidationMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_expiration_message, HasPollMessage<HasPollExpirationMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_imp_expiration_message, HasPollMessage<HasPollImpExpirationMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_idle_delete_domain_message, HasPollMessage<HasPollIdleDeleteDomainMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_idle_delete_contact_message, HasPollMessage<HasPollIdleDeleteContactMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_idle_delete_nsset_message, HasPollMessage<HasPollIdleDeleteNssetMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_idle_delete_keyset_message, HasPollMessage<HasPollIdleDeleteKeysetMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_outzone_message, HasPollMessage<HasPollOutzoneUnguardedMessage>)
{
    test();
}

BOOST_FIXTURE_TEST_CASE(request_tech_check_message, HasPollTechCheckMessage)
{
    test();
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
