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
#include "src/fredlib/poll/create_transfer_domain_poll_message.h"
#include "src/fredlib/poll/create_transfer_contact_poll_message.h"
#include "src/fredlib/poll/create_transfer_nsset_poll_message.h"
#include "src/fredlib/poll/create_transfer_keyset_poll_message.h"
#include "src/fredlib/domain/transfer_domain.h"
#include "src/fredlib/contact/transfer_contact.h"
#include "src/fredlib/nsset/transfer_nsset.h"
#include "src/fredlib/keyset/transfer_keyset.h"
#include "src/epp/poll/poll_request.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_code.h"

#include <boost/test/unit_test.hpp>
#include <boost/variant.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)

namespace {

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

struct HasPollUpdateDomainMessage : virtual Test::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateDomainMessage()
    {
        Test::domain domain(ctx);
        history_id = Fred::UpdateDomain(domain.info_data.fqdn,
                                        domain.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::update_domain> submessage;
};

struct HasPollUpdateNssetMessage : virtual Test::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateNssetMessage()
    {
        Test::nsset nsset(ctx);
        history_id = Fred::UpdateNsset(nsset.info_data.handle,
                                       nsset.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::update_nsset> submessage;
};

struct HasPollUpdateKeysetMessage : virtual Test::autorollbacking_context
{
    unsigned long long history_id;

    HasPollUpdateKeysetMessage()
    {
        Test::keyset keyset(ctx);
        history_id = Fred::UpdateKeyset(keyset.info_data.handle,
                                        keyset.info_data.sponsoring_registrar_handle
            ).set_authinfo("doesntmatter").exec(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::UpdateInfoEvent::Data<Epp::Poll::UpdateInfoEvent::update_keyset> submessage;
};

template<typename T>
struct HasPollUpdate : T
{
    void test()
    {
        namespace ep = Epp::Poll;
        typedef typename T::submessage submessage;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        const Test::MessageDetail mesage_detail = Test::get_message_ids(T::ctx);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(T::ctx, mesage_detail.registrar_id));

        ep::UpdateInfoEvent update_info_event;
        BOOST_CHECK_NO_THROW(update_info_event = boost::get<ep::UpdateInfoEvent>(output.message));
        submessage update_info;
        BOOST_CHECK_NO_THROW(update_info = boost::get<submessage>(update_info_event.message));

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
        Test::domain domain(ctx);
        Test::registrar registrar(ctx);

        object_handle = domain.info_data.fqdn;
        dst_registrar_handle = registrar.info_data.handle;

        unsigned long long history_id = Fred::TransferDomain(domain.info_data.id,
                                                             registrar.info_data.handle,
                                                             domain.info_data.authinfopw,
                                                             Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreateTransferDomainPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_domain> submessage;
};

struct HasPollTransferContactMessage : virtual Test::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferContactMessage()
    {
        Test::contact contact(ctx);
        Test::registrar registrar(ctx);

        object_handle = contact.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        unsigned long long history_id = Fred::TransferContact(contact.info_data.id,
                                                              registrar.info_data.handle,
                                                              contact.info_data.authinfopw,
                                                              Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreateTransferContactPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_contact> submessage;
};

struct HasPollTransferNssetMessage : virtual Test::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferNssetMessage()
    {
        Test::nsset nsset(ctx);
        Test::registrar registrar(ctx);

        object_handle = nsset.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        unsigned long long history_id = Fred::TransferNsset(nsset.info_data.id,
                                                            registrar.info_data.handle,
                                                            nsset.info_data.authinfopw,
                                                            Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreateTransferNssetPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_nsset> submessage;
};

struct HasPollTransferKeysetMessage : virtual Test::autorollbacking_context
{
    std::string object_handle;
    std::string dst_registrar_handle;

    HasPollTransferKeysetMessage()
    {
        Test::keyset keyset(ctx);
        Test::registrar registrar(ctx);

        object_handle = keyset.info_data.handle;
        dst_registrar_handle = registrar.info_data.handle;

        unsigned long long history_id = Fred::TransferKeyset(keyset.info_data.id,
                                                             registrar.info_data.handle,
                                                             keyset.info_data.authinfopw,
                                                             Nullable<unsigned long long>()).exec(ctx);

        Fred::Poll::CreateTransferKeysetPollMessage(history_id).exec(ctx);
    }

    typedef Epp::Poll::TransferEvent::Data<Epp::Poll::TransferEvent::transfer_keyset> submessage;
};

template<typename T>
struct HasPollTransfer : T
{
    void test()
    {
        namespace ep = Epp::Poll;
        typedef typename T::submessage submessage;

        const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
        BOOST_REQUIRE_EQUAL(before_message_count, 1);

        const Test::MessageDetail message_detail = Test::get_message_ids(T::ctx);

        ep::PollRequestOutputData output;
        BOOST_CHECK_NO_THROW(output = ep::poll_request(T::ctx, message_detail.registrar_id));

        ep::TransferEvent transfer_event;
        BOOST_CHECK_NO_THROW(transfer_event = boost::get<ep::TransferEvent>(output.message));
        submessage transfer_info;
        BOOST_CHECK_NO_THROW(transfer_info = boost::get<submessage>(transfer_event.message));

        const boost::gregorian::date real_transfer_date = get_transfer_date_by_message_id(T::ctx, message_detail.message_id);

        BOOST_CHECK_EQUAL(transfer_info.transfer_date, real_transfer_date);
        BOOST_CHECK_EQUAL(transfer_info.object_handle, T::object_handle);
        BOOST_CHECK_EQUAL(transfer_info.dst_registrar_handle, T::dst_registrar_handle);

        const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(T::ctx);
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

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
