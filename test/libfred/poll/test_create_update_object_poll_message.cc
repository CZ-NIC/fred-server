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


#include "libfred/poll/create_update_object_poll_message.hh"
#include "libfred/poll/message_type.hh"

#include <boost/test/unit_test.hpp>

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

BOOST_FIXTURE_TEST_SUITE(TestPoll, Test::instantiate_db_template)
BOOST_AUTO_TEST_SUITE(TestCreateUpdateObjectPollMessage)

/**
 @test
 executing TestCreateUpdateObjectPollMessage and checking data in database common for all types

 @pre existing domain history_id
 @post correct values present in message and poll_eppaction tables
 */
BOOST_AUTO_TEST_CASE( test_correct_generic_data )
{
    ::LibFred::OperationContextCreator ctx;

    Test::domain domain(ctx);

    Database::Result ids_res = ctx.get_conn().exec(
        "SELECT id AS count_ "
        "   FROM message "
    );

    std::vector<int> ids_before;
    for(Database::Result::Iterator it = ids_res.begin();
        it != ids_res.end();
        ++it
    ) {
        ids_before.push_back(
            static_cast<int>((*it)["count_"])
        );
    }

    ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, domain.info_data.historyid);

    Database::Result ids2_res = ctx.get_conn().exec(
        "SELECT id AS count_ "
        "   FROM message "
    );

    int new_message_id = -1;
    std::vector<int> ids_after;
    for(Database::Result::Iterator it = ids2_res.begin();
        it != ids2_res.end();
        ++it
    ) {
        ids_after.push_back(
            static_cast<int>((*it)["count_"])
        );

        if( std::find(ids_before.begin(), ids_before.end(), *ids_after.rbegin())
            == ids_before.end()
        ) {
            new_message_id = *ids_after.rbegin();
        }
    }

    BOOST_CHECK_MESSAGE(new_message_id != -1, "SOMETHING IS TERRIBLY WRONG");
    BOOST_CHECK_EQUAL(ids_before.size() + 1, ids_after.size());

    Database::Result message_res = ctx.get_conn().exec_params(
        "SELECT seen "
        "   FROM message "
        "   WHERE id=$1::integer",
        Database::query_param_list
            (new_message_id)
    );

    BOOST_CHECK_EQUAL(message_res.size(), 1);
    BOOST_CHECK_EQUAL(static_cast<bool>(message_res[0]["seen"]), false);

    Database::Result polleppaction_res = ctx.get_conn().exec_params(
        "SELECT msgid, objid "
        "   FROM poll_eppaction "
        "   WHERE msgid=$1::integer",
        Database::query_param_list
            (new_message_id)
    );

    BOOST_CHECK_EQUAL(polleppaction_res.size(), 1);
    BOOST_CHECK_EQUAL(static_cast<unsigned long long>(polleppaction_res[0]["objid"]), domain.info_data.historyid);

    ctx.commit_transaction();
}

/**
 @test
 executing TestCreateUpdateObjectPollMessage with history_ids of all registry object types

 @pre existing samples of all registry object types
 @post correct messagetype present poll_eppaction tables or correct exception thrown
 */
BOOST_AUTO_TEST_CASE( test_correct_type_specific_data)
{

    for(int i=0; i<3; ++i) {
        ::LibFred::OperationContextCreator ctx;

        Database::Result ids_res = ctx.get_conn().exec(
            "SELECT id AS count_ "
            "   FROM message "
        );

        std::vector<int> ids_before;
        for(Database::Result::Iterator it = ids_res.begin();
            it != ids_res.end();
            ++it
        ) {
            ids_before.push_back(
                static_cast<int>((*it)["count_"])
            );
        }
        Test::contact contact(ctx);
        Test::domain domain(ctx);
        Test::keyset keyset(ctx);
        Test::nsset nsset(ctx);

        switch(i) {
            case 0:
                ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, contact.info_data.historyid);
                break;
            case 1:
                ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, domain.info_data.historyid);
                break;
            case 2:
                ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, keyset.info_data.historyid);
                break;
            case 3:
                ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, nsset.info_data.historyid);
                break;
        }
        Database::Result ids2_res = ctx.get_conn().exec(
            "SELECT id AS count_ "
            "   FROM message "
        );

        int new_message_id = -1;
        std::vector<int> ids_after;
        for(Database::Result::Iterator it = ids2_res.begin();
            it != ids2_res.end();
            ++it
        ) {
            ids_after.push_back(
                static_cast<int>((*it)["count_"])
            );

            if( std::find(ids_before.begin(), ids_before.end(), *ids_after.rbegin())
                == ids_before.end()
            ) {
                new_message_id = *ids_after.rbegin();
            }
        }

        BOOST_CHECK_MESSAGE(new_message_id != -1, "SOMETHING IS TERRIBLY WRONG");
        BOOST_CHECK_EQUAL(ids_before.size() + 1, ids_after.size());

        Database::Result message_res = ctx.get_conn().exec_params(
            "SELECT mt.name     AS msgtype_name_ "
            "   FROM message AS m "
            "       JOIN messagetype AS mt "
            "           ON m.msgtype = mt.id "
            "   WHERE m.id=$1::integer",
            Database::query_param_list
                (new_message_id)
        );

        switch (i) {
            case 0:
                BOOST_CHECK_EQUAL(static_cast<std::string>(message_res[0]["msgtype_name_"]),
                                  Conversion::Enums::to_db_handle(::LibFred::Poll::MessageType::update_contact));
                break;
            case 1:
                BOOST_CHECK_EQUAL(static_cast<std::string>(message_res[0]["msgtype_name_"]),
                                  Conversion::Enums::to_db_handle(::LibFred::Poll::MessageType::update_domain));
                break;
            case 2:
                BOOST_CHECK_EQUAL(static_cast<std::string>(message_res[0]["msgtype_name_"]),
                                  Conversion::Enums::to_db_handle(::LibFred::Poll::MessageType::update_keyset));
                break;
            case 3:
                BOOST_CHECK_EQUAL(static_cast<std::string>(message_res[0]["msgtype_name_"]),
                                  Conversion::Enums::to_db_handle(::LibFred::Poll::MessageType::update_nsset));
                break;
        }

        ctx.commit_transaction();
    }
}

/**
 @test
 executing CreateUpdateObjectPollMessage with nonexistent history_id and checking thrown exception

 @pre nonexistent object history_id
 @post exception is thrown and has correct flag set
 */
BOOST_AUTO_TEST_CASE( test_nonexistent_historyid )
{
    ::LibFred::OperationContextCreator ctx;

    const unsigned long long nonexistent_object_historyid = Test::get_nonexistent_object_historyid(ctx);
    try {
        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, nonexistent_object_historyid);
        BOOST_CHECK(false);
    }
    catch (const ::LibFred::OperationException&) {
        BOOST_CHECK(true);
    }
    catch (...) {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
