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
#include "src/epp/poll/poll_acknowledgement.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollAcknowledgement)

namespace {

unsigned long long get_number_of_unseen_poll_messages(Fred::OperationContext& _ctx)
{
    const Database::Result sql_query = _ctx.get_conn().exec("SELECT COUNT(id) FROM message WHERE NOT seen");
    BOOST_REQUIRE_EQUAL(sql_query.size(), 1);
    return static_cast<unsigned long long>(sql_query[0][0]);
}

struct MessageDetail
{
    unsigned long long message_id;
    unsigned long long registrar_id;
};

MessageDetail get_message_ids(Fred::OperationContext& _ctx)
{
    MessageDetail ret;

    const Database::Result sql_query =
        _ctx.get_conn().exec("SELECT id, clid FROM message WHERE NOT seen ORDER BY id DESC LIMIT 1");
    BOOST_REQUIRE_EQUAL(sql_query.size(), 1);
    ret.message_id = static_cast<unsigned long long>(sql_query[0][0]);
    ret.registrar_id = static_cast<unsigned long long>(sql_query[0][1]);

    return ret;
}

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(successful_acknowledgement, HasPollInfoMessage)
{
    const unsigned long long before_message_count = get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const MessageDetail mesage_detail = get_message_ids(ctx);

    Epp::Poll::PollAcknowledgementOutputData output;
    BOOST_CHECK_NO_THROW(output =
        Epp::Poll::poll_acknowledgement(ctx, mesage_detail.message_id, mesage_detail.registrar_id));
    BOOST_CHECK_EQUAL(output.number_of_unseen_messages, 0);

    const unsigned long long after_message_count = get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 0);
}

BOOST_FIXTURE_TEST_CASE(successful_chain_of_acknowledgements, HasTwoPollInfoMessages)
{
    const unsigned long long before_message_count = get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 2);

    const MessageDetail first_mesage_detail = get_message_ids(ctx);

    Epp::Poll::PollAcknowledgementOutputData first_output;
    BOOST_CHECK_NO_THROW(first_output =
        Epp::Poll::poll_acknowledgement(ctx, first_mesage_detail.message_id, first_mesage_detail.registrar_id));
    BOOST_CHECK_EQUAL(first_output.number_of_unseen_messages, 1);

    const unsigned long long middle_message_count = get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(middle_message_count, 1);

    const MessageDetail second_mesage_detail = get_message_ids(ctx);
    BOOST_CHECK_EQUAL(second_mesage_detail.message_id, first_output.oldest_unseen_message_id);

    Epp::Poll::PollAcknowledgementOutputData second_output;
    BOOST_CHECK_NO_THROW(second_output =
        Epp::Poll::poll_acknowledgement(ctx, second_mesage_detail.message_id, second_mesage_detail.registrar_id));
    BOOST_CHECK_EQUAL(second_output.number_of_unseen_messages, 0);

    const unsigned long long after_message_count = get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 0);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
