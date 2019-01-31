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

#include "test/backend/epp/poll/fixture.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/backend/epp/util.hh"
#include "libfred/poll/create_update_object_poll_message.hh"
#include "libfred/poll/create_poll_message.hh"
#include "src/backend/epp/poll/poll_acknowledgement.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollAcknowledgement)

namespace {

struct HasPollInfoMessage : virtual Test::Backend::Epp::autorollbacking_context
{
    HasPollInfoMessage()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::domain domain(ctx);
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, domain.info_data.historyid);
    }
};

struct HasTwoPollInfoMessages : virtual Test::Backend::Epp::autorollbacking_context
{
    HasTwoPollInfoMessages()
    {
        Test::mark_all_messages_as_seen(ctx);
        const Test::domain domain(ctx);
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, domain.info_data.historyid);

        const unsigned long long new_history_id =
            ::LibFred::UpdateDomain(domain.info_data.fqdn,
                               domain.info_data.sponsoring_registrar_handle
                ).set_authinfo("doesntmatter").exec(ctx);
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(successful_acknowledgement, HasPollInfoMessage)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    Epp::Poll::PollAcknowledgementOutputData output;
    BOOST_CHECK_NO_THROW(output =
        Epp::Poll::poll_acknowledgement(ctx, mesage_detail.message_id, mesage_detail.registrar_id));
    BOOST_CHECK_EQUAL(output.number_of_unseen_messages, 0);

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 0);
}

BOOST_FIXTURE_TEST_CASE(successful_chain_of_acknowledgements, HasTwoPollInfoMessages)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 2);

    const Test::MessageDetail first_mesage_detail = Test::get_message_ids(ctx);

    Epp::Poll::PollAcknowledgementOutputData first_output;
    BOOST_CHECK_NO_THROW(first_output =
        Epp::Poll::poll_acknowledgement(ctx, first_mesage_detail.message_id, first_mesage_detail.registrar_id));
    BOOST_CHECK_EQUAL(first_output.number_of_unseen_messages, 1);

    const unsigned long long middle_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(middle_message_count, 1);

    const Test::MessageDetail second_mesage_detail = Test::get_message_ids(ctx);
    BOOST_CHECK_EQUAL(second_mesage_detail.message_id, first_output.oldest_unseen_message_id);

    Epp::Poll::PollAcknowledgementOutputData second_output;
    BOOST_CHECK_NO_THROW(second_output =
        Epp::Poll::poll_acknowledgement(ctx, second_mesage_detail.message_id, second_mesage_detail.registrar_id));
    BOOST_CHECK_EQUAL(second_output.number_of_unseen_messages, 0);

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 0);
}

BOOST_FIXTURE_TEST_CASE(failed_acknowledgement, HasPollInfoMessage)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    const unsigned long long bogus_message_id = Test::get_nonexistent_message_id(ctx);
    const unsigned long long bogus_registrar_id = Test::get_nonexistent_registrar_id(ctx);

    BOOST_CHECK_THROW(
        Epp::Poll::poll_acknowledgement(ctx, mesage_detail.message_id, bogus_registrar_id), Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_acknowledgement(ctx, bogus_message_id, mesage_detail.registrar_id), Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_acknowledgement(ctx, bogus_message_id, bogus_registrar_id), Epp::EppResponseFailure);

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
