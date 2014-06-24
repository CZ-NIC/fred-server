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

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "setup_utils.h"

#include "src/fredlib/poll/create_delete_domain_poll_message.h"
#include "src/fredlib/poll/message_types.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPoll)
BOOST_AUTO_TEST_SUITE(TestCreateDeleteDomainPollMessage)

/**
 @test
 executing CreateDeleteDomainPollMessage and checking data in database

 @pre existing domain history_id
 @post correct values present in message and poll_eppaction tables
 */

BOOST_AUTO_TEST_CASE( test_correct_data )
{
    Fred::OperationContext ctx;

    setup_domain domain;

    Database::Result count_res = ctx.get_conn().exec(
        "SELECT COUNT(id) AS count_ "
        "   FROM message "
    );

    int count_before = static_cast<int>(count_res[0]["count_"]);

    unsigned long long message_id = Fred::Poll::CreateDeleteDomainPollMessage(
        domain.data_.info_domain_data.historyid
    ).exec(ctx);

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
    BOOST_CHECK_EQUAL(static_cast<bool>(message_res[0]["seen_"]), false);
    BOOST_CHECK_EQUAL(static_cast<std::string>(message_res[0]["msgtype_name_"]), Fred::Poll::DELETE_DOMAIN);

    Database::Result polleppaction_res = ctx.get_conn().exec_params(
        "SELECT objid "
        "   FROM poll_eppaction "
        "   WHERE msgid=$1::integer",
        Database::query_param_list
            (message_id)
    );

    BOOST_CHECK_EQUAL(polleppaction_res.size(), 1);
    BOOST_CHECK_EQUAL(static_cast<unsigned long long>(polleppaction_res[0]["objid"]), domain.data_.info_domain_data.historyid);

    ctx.commit_transaction();
}

/**
 @test
 executing CreateDeleteDomainPollMessage with nonexistent history_id and checking thrown exception

 @pre existing object history_id (not even for types other than domain)
 @post exception is thrown and has correct flag set
 */
BOOST_AUTO_TEST_CASE( test_nonexistent_historyid )
{
    setup_nonexistent_object_historyid nonexist_historyid;

    Fred::OperationContext ctx;

    bool correct_exception_caught = false;
    try {
        Fred::Poll::CreateDeleteDomainPollMessage(
            nonexist_historyid.history_id_
        ).exec(ctx);
    } catch(const Fred::Poll::CreateDeleteDomainPollMessage::Exception& e) {
        if(e.get_object_history_not_found()) {
            correct_exception_caught = true;
        }
    }

    BOOST_CHECK_EQUAL(correct_exception_caught, true);
}

/**
 @test
 executing CreateDeleteDomainPollMessage with history_id of existing contact and checking thrown exception

 @pre existing contact history_id
 @post exception is thrown and has correct flag set
 */
BOOST_AUTO_TEST_CASE( test_different_object_type )
{
    setup_contact contact;

    Fred::OperationContext ctx;

    bool correct_exception_caught = false;
    try {
        Fred::Poll::CreateDeleteDomainPollMessage(
                contact.data_.info_contact_data.historyid
        ).exec(ctx);
    } catch(const Fred::Poll::CreateDeleteDomainPollMessage::Exception& e) {
        if(e.get_domain_not_found()) {
            correct_exception_caught = true;
        }
    }

    BOOST_CHECK_EQUAL(correct_exception_caught, true);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
