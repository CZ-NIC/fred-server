/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "tests/interfaces/epp/poll/fixture.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/interfaces/epp/util.h"
#include "src/fredlib/poll/create_update_object_poll_message.h"
#include "src/fredlib/poll/create_poll_message.h"
#include "src/fredlib/domain/transfer_domain.h"
#include "src/fredlib/contact/transfer_contact.h"
#include "src/fredlib/nsset/transfer_nsset.h"
#include "src/fredlib/keyset/transfer_keyset.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/domain/delete_domain.h"
#include "src/epp/poll/poll_request.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_code.h"

#include <boost/test/unit_test.hpp>
#include <boost/variant.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)

namespace {

boost::gregorian::date get_erase_date_by_handle(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    const std::string& _type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT MAX(erdate) FROM object_registry WHERE UPPER(name)=UPPER(")
        .param_text(_handle)(") AND type=get_object_type_id(").param_text(_type)(")");
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    const bool is_null = sql_query_result[0][0].isnull();
    BOOST_REQUIRE(!is_null);
    return boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
}

unsigned long long get_request_id_by_history_id(Fred::OperationContext& _ctx, unsigned long long _history_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT request_id FROM history WHERE id=").param_bigint(_history_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    return static_cast<unsigned long long>(sql_query_result[0][0]);
}

boost::gregorian::date get_transfer_date_by_message_id(Fred::OperationContext& _ctx, unsigned long long _message_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT oh.trdate::date "
              "FROM poll_eppaction pe JOIN object_history oh ON oh.historyid=pe.objid "
              "WHERE pe.msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    return boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
}

unsigned long long create_poll_request_fee_message(
    Fred::OperationContext& _ctx,
    unsigned long long _registrar_id,
    const boost::posix_time::ptime &_period_from,
    const boost::posix_time::ptime &_period_to,
    unsigned long long _total_free_count,
    unsigned long long _request_count,
    const Decimal &_price)
{
    Database::ParamQuery message_sql_query;
    message_sql_query("INSERT INTO message (id, clid, crdate, exdate, msgtype) "
                      "VALUES (nextval('message_id_seq'::regclass), ").param_bigint(_registrar_id)(", ")
                     ("current_timestamp, current_timestamp + interval '7 days', "
                      "(SELECT id FROM messagetype WHERE name='request_fee_info')) "
                      "RETURNING id");

    const Database::Result message_sql_query_result = _ctx.get_conn().exec_params(message_sql_query);
    BOOST_REQUIRE_EQUAL(message_sql_query_result.size(), 1);

    const unsigned long long poll_msg_id = static_cast<unsigned long long>(message_sql_query_result[0][0]);

    const Database::Result poll_request_fee_sql_query_result = _ctx.get_conn().exec_params(
        "INSERT INTO poll_request_fee "
        "(msgid, period_from, period_to, total_free_count, used_count, price) "
        "VALUES ($1::integer, $2::timestamp, "
        "$3::timestamp, $4::bigint, "
        "$5::bigint, $6::numeric(10,2))",
        Database::query_param_list
        (poll_msg_id)
        (_period_from)
        (_period_to)
        (_total_free_count)
        (_request_count)
        (_price.get_string()));

    return poll_msg_id;
}

struct HasPollUpdateDomainMessage : virtual Test::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateDomainMessage()
    {
        const Test::domain domain(ctx);
        history_id = Fred::UpdateDomain(domain.info_data.fqdn,
                                        domain.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::update_domain> SubMessage;
};

struct HasPollUpdateNssetMessage : virtual Test::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateNssetMessage()
    {
        const Test::nsset nsset(ctx);
        history_id = Fred::UpdateNsset(nsset.info_data.handle,
                                       nsset.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::update_nsset> SubMessage;
};

struct HasPollUpdateKeysetMessage : virtual Test::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateKeysetMessage()
    {
        const Test::keyset keyset(ctx);
        history_id = Fred::UpdateKeyset(keyset.info_data.handle,
                                        keyset.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::update_keyset> SubMessage;
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

        const Test::MessageDetail mesage_detail = Test::get_message_ids(T::ctx);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(T::ctx, mesage_detail.registrar_id));

        ep::UpdateInfoEvent update_info_event;
        BOOST_CHECK_NO_THROW(update_info_event = boost::get<ep::UpdateInfoEvent>(output.message));
        SubMessage update_info;
        BOOST_CHECK_NO_THROW(update_info = boost::get<SubMessage>(update_info_event.message));

        BOOST_CHECK_EQUAL(update_info.transaction_id, get_request_id_by_history_id(T::ctx, T::history_id));
        BOOST_CHECK_EQUAL(update_info.poll_id, mesage_detail.message_id);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

struct HasPollTransferDomainMessage : virtual Test::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferDomainMessage()
    {
        const Test::domain domain(ctx);
        const Test::registrar registrar(ctx);

        object_handle = domain.info_data.fqdn;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = Fred::TransferDomain(domain.info_data.id,
                                                                   registrar.info_data.handle,
                                                                   domain.info_data.authinfopw,
                                                                   Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreatePollMessage<Fred::Poll::MessageType::transfer_domain>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_domain> SubMessage;
};

struct HasPollTransferContactMessage : virtual Test::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferContactMessage()
    {
        const Test::contact contact(ctx);
        const Test::registrar registrar(ctx);

        object_handle = contact.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = Fred::TransferContact(contact.info_data.id,
                                                                    registrar.info_data.handle,
                                                                    contact.info_data.authinfopw,
                                                                    Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreatePollMessage<Fred::Poll::MessageType::transfer_contact>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_contact> SubMessage;
};

struct HasPollTransferNssetMessage : virtual Test::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferNssetMessage()
    {
        const Test::nsset nsset(ctx);
        const Test::registrar registrar(ctx);

        object_handle = nsset.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = Fred::TransferNsset(nsset.info_data.id,
                                                                  registrar.info_data.handle,
                                                                  nsset.info_data.authinfopw,
                                                                  Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreatePollMessage<Fred::Poll::MessageType::transfer_nsset>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_nsset> SubMessage;
};

struct HasPollTransferKeysetMessage : virtual Test::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferKeysetMessage()
    {
        const Test::keyset keyset(ctx);
        const Test::registrar registrar(ctx);

        object_handle = keyset.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        const unsigned long long history_id = Fred::TransferKeyset(keyset.info_data.id,
                                                                   registrar.info_data.handle,
                                                                   keyset.info_data.authinfopw,
                                                                   Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreatePollMessage<Fred::Poll::MessageType::transfer_keyset>().exec(ctx, history_id);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_keyset> SubMessage;
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

struct HasPollDeleteDomainMessage : virtual Test::autorollbacking_context
{
    std::string handle;
    const std::string message_type;

    HasPollDeleteDomainMessage() : message_type("domain")
    {
        const Test::domain domain(ctx);
        handle = domain.info_data.fqdn;
        unsigned long long history_id =
            Fred::InfoDomainByHandle(domain.info_data.fqdn).exec(ctx).info_domain_data.historyid;
        Fred::DeleteDomainByHandle(domain.info_data.fqdn).exec(ctx);

        Fred::Poll::CreatePollMessage<Fred::Poll::MessageType::delete_domain>().exec(ctx, history_id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::delete_domain> SubMessage;
};

struct HasPollDeleteContactMessage : virtual Test::autorollbacking_context
{
    std::string handle;
    const std::string message_type;

    HasPollDeleteContactMessage() : message_type("contact")
    {
        const Test::contact contact(ctx);
        handle = contact.info_data.handle;
        unsigned long long history_id =
            Fred::InfoContactByHandle(contact.info_data.handle).exec(ctx).info_contact_data.historyid;
        Fred::DeleteContactByHandle(contact.info_data.handle).exec(ctx);

        Fred::Poll::CreatePollMessage<Fred::Poll::MessageType::delete_contact>().exec(ctx, history_id);
    }

    typedef Epp::Poll::MessageEvent::Data<Epp::Poll::MessageEvent::delete_contact> SubMessage;
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

        const boost::gregorian::date real_erase_date = get_erase_date_by_handle(T::ctx, T::handle, T::message_type);

        BOOST_CHECK_EQUAL(message_info.date, real_erase_date);
        BOOST_CHECK_EQUAL(message_info.handle, T::handle);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_CHECK_EQUAL(after_message_count, 1);
    }
};

struct HasPollRequestFeeInfoMessage : virtual Test::autorollbacking_context
{
    unsigned long long message_id;
    unsigned long long registrar_id;
    Epp::Poll::RequestFeeInfoEvent golden_request_fee_info_event;

    HasPollRequestFeeInfoMessage()
    {
        const Test::registrar registrar(ctx);
        registrar_id = registrar.info_data.id;
        golden_request_fee_info_event.from = boost::posix_time::time_from_string("2017-03-30 14:16:02.713506");
        golden_request_fee_info_event.to = boost::posix_time::time_from_string("2017-03-30 14:17:07.113492");

        golden_request_fee_info_event.free_count = 20;
        golden_request_fee_info_event.used_count = 10;
        golden_request_fee_info_event.price = "1024.42";

        message_id = create_poll_request_fee_message(ctx,
                                                     registrar_id,
                                                     golden_request_fee_info_event.from,
                                                     golden_request_fee_info_event.to,
                                                     golden_request_fee_info_event.free_count,
                                                     golden_request_fee_info_event.used_count,
                                                     golden_request_fee_info_event.price);
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

} // namespace {anonymous}

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

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
