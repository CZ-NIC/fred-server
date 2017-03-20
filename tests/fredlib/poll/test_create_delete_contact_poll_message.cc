/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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


#include "src/fredlib/poll/create_epp_action_poll_message.h"
#include "src/fredlib/poll/message_type.h"

#include <boost/test/unit_test.hpp>

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

BOOST_FIXTURE_TEST_SUITE(TestPoll, Test::instantiate_db_template)
BOOST_AUTO_TEST_SUITE(TestCreateDeleteContactPollMessage)

/**
 @test
 executing CreateDeleteContactPollMessage and checking data in database

 @pre existing contact history_id
 @post correct values present in message and poll_eppaction tables
 */
BOOST_AUTO_TEST_CASE( test_correct_data )
{
    Fred::OperationContextCreator ctx;

    Test::contact contact(ctx);

    const Database::Result count_res = ctx.get_conn().exec(
        "SELECT COUNT(id) AS count_ FROM message");

    const int count_before = static_cast<int>(count_res[0]["count_"]);

    const unsigned long long message_id = Fred::Poll::CreateEppActionPollMessage::
            Of<Fred::Poll::MessageType::delete_contact>().exec(ctx, contact.info_data.historyid);

    Database::Result count_res2 = ctx.get_conn().exec(
        "SELECT COUNT(id) AS count_ "
        "   FROM message "
    );

    BOOST_CHECK_EQUAL(count_before + 1, static_cast<int>(count_res2[0]["count_"]));

    Database::Result message_res = ctx.get_conn().exec_params(
        "SELECT "
        "       m.seen      AS seen_, "
        "       mt.name     AS msgtype_name_ "
        "   FROM message AS m "
        "       JOIN messagetype AS mt "
        "           ON m.msgtype = mt.id "
        "   WHERE m.id=$1::integer",
        Database::query_param_list
            (message_id)
    );

    BOOST_CHECK_EQUAL(message_res.size(), 1);
    BOOST_CHECK(!static_cast<bool>(message_res[0]["seen_"]));
    BOOST_CHECK_EQUAL(static_cast<std::string>(message_res[0]["msgtype_name_"]),
                      Conversion::Enums::to_db_handle(Fred::Poll::MessageType::delete_contact));

    Database::Result polleppaction_res = ctx.get_conn().exec_params(
        "SELECT objid "
        "   FROM poll_eppaction "
        "   WHERE msgid=$1::integer",
        Database::query_param_list
            (message_id)
    );

    BOOST_CHECK_EQUAL(polleppaction_res.size(), 1);
    BOOST_CHECK_EQUAL(static_cast<unsigned long long>(polleppaction_res[0]["objid"]), contact.info_data.historyid);

    ctx.commit_transaction();
}

/**
 @test
 executing CreateDeleteContactPollMessage with nonexistent history_id and checking thrown exception

 @pre nonexistent object history_id (not even for types other than contact)
 @post exception is thrown and has correct flag set
 */
BOOST_AUTO_TEST_CASE(test_nonexistent_historyid)
{
    Fred::OperationContextCreator ctx;
    const unsigned long long nonexistent_object_historyid = Test::get_nonexistent_object_historyid(ctx);
    try {
        Fred::Poll::CreateEppActionPollMessage::
                    Of<Fred::Poll::MessageType::delete_contact>().exec(ctx, nonexistent_object_historyid);
    }
    catch (const Fred::Poll::CreateEppActionPollMessage::Exception::ObjectHistoryNotFound& e) {
        BOOST_CHECK_EQUAL(e.history_id, nonexistent_object_historyid);
    }
}

/**
 @test
 executing CreateDeleteContactPollMessage with history_id of existing domain and checking thrown exception

 @pre existing domain history_id
 @post exception is thrown and has correct flag set
 */
BOOST_AUTO_TEST_CASE( test_different_object_type )
{
    Fred::OperationContextCreator ctx;
    const Test::domain domain(ctx);

    try {
        Fred::Poll::CreateEppActionPollMessage::
                            Of<Fred::Poll::MessageType::delete_contact>().exec(ctx, domain.info_data.historyid);
    }
    catch (const Fred::Poll::CreateEppActionPollMessage::Exception::ObjectNotRequestedType&) {
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
