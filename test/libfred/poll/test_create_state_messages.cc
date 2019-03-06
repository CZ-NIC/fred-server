/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/poll/create_state_messages.hh"
#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "libfred/poll/message_type_set.hh"

#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>

BOOST_AUTO_TEST_SUITE(TestPoll)
BOOST_AUTO_TEST_SUITE(TestCreateStateMessages)

namespace {

unsigned long long get_number_of_poll_messages(::LibFred::OperationContext& _ctx)
{
    const Database::Result sql_query_result = _ctx.get_conn().exec("SELECT COUNT(*) FROM message");
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    return static_cast<unsigned long long>(sql_query_result[0][0]);
}

// fixtures

struct PollStateMessages : Test::instantiate_db_template
{
    ::LibFred::OperationContextCreator ctx;
    std::string handle;
    boost::gregorian::date date;

    PollStateMessages() :
        handle("expirationxazuxxaxxdxefxfxfuxeca.cz"),
        date(boost::posix_time::second_clock::local_time().date() - boost::gregorian::days(7))
    {
        const auto preliminary_message_creator = ::LibFred::Poll::CreateStateMessages(
                std::set<::LibFred::Poll::MessageType::Enum>(),
                boost::optional<int>());
        BOOST_CHECK_NO_THROW(preliminary_message_creator.exec(ctx));
        const Test::registrar registrar(ctx);
        const Test::contact contact(ctx);
        const ::LibFred::CreateDomain::Result result =
            ::LibFred::CreateDomain(handle, registrar.info_data.handle, contact.info_data.handle)
            .set_expiration_date(date).exec(ctx, "Europe/Prague");
        const Database::Result sql_query_result = ctx.get_conn().exec_params(
            "INSERT INTO object_state (object_id, state_id, valid_from, valid_to, ohid_from, ohid_to) "
            "SELECT oh.id, (SELECT id FROM enum_object_states WHERE name='expired'), "
            "NOW()-'2MONTHS'::INTERVAL, NULL, oh.historyid, NULL "
            "FROM object_history oh "
            "WHERE id=$1::integer "
            "ORDER BY historyid DESC "
            "LIMIT 1 "
            "RETURNING id, valid_from::date",
            Database::query_param_list
            (result.create_object_result.object_id));
    }

    void test()
    {
        const unsigned long long start_message_count = get_number_of_poll_messages(ctx);

        std::set<::LibFred::Poll::MessageType::Enum> except_list =
            Conversion::Enums::Sets::from_config_string<::LibFred::Poll::MessageType>(
                    "expiration,imp_expiration,validation,imp_validation,outzone,idle_delete_contact,"
                    "idle_delete_nsset,idle_delete_domain,idle_delete_keyset");

        const ::LibFred::Poll::CreateStateMessages blacklisted_message_creator(except_list, boost::optional<int>(10));
        BOOST_CHECK_NO_THROW(blacklisted_message_creator.exec(ctx));

        const unsigned long long after_the_first_query_message_count = get_number_of_poll_messages(ctx);
        BOOST_CHECK_EQUAL(after_the_first_query_message_count, start_message_count);

        const auto real_message_creator = ::LibFred::Poll::CreateStateMessages(
                std::set<::LibFred::Poll::MessageType::Enum>(),
                boost::optional<int>());
        BOOST_CHECK_NO_THROW(real_message_creator.exec(ctx));

        const unsigned long long after_the_second_query_message_count = get_number_of_poll_messages(ctx);
        BOOST_CHECK_GT(after_the_second_query_message_count, start_message_count);

        const Database::Result sql_query_result = ctx.get_conn().exec(
            "SELECT dh.exdate::date, obr.name "
            "FROM poll_statechange ps "
            "JOIN object_state os ON os.id=ps.stateid "
            "JOIN object_registry obr ON obr.id=os.object_id "
            "JOIN message m ON m.id=ps.msgid "
            "JOIN domain_history dh ON dh.historyid=os.ohid_from "
            "WHERE m.msgtype=10 AND m.seen = false ORDER BY m.id DESC LIMIT 1");

        BOOST_CHECK_EQUAL(sql_query_result.size(), 1);
        boost::gregorian::date db_date = boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
        std::string db_handle = static_cast<std::string>(sql_query_result[0][1]);

        BOOST_CHECK_EQUAL(db_date, date);
        BOOST_CHECK_EQUAL(db_handle, handle);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(fredlib_state_messages, PollStateMessages)
{
    test();
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
