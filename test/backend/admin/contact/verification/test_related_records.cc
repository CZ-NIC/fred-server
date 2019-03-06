/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  integration tests for admin/contact/verification/related_records.cc
 */

#include "src/backend/admin/contact/verification/related_records.hh"

#include <boost/test/unit_test.hpp>


#include "test/backend/admin/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"


BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestRelatedRecords, Test::instantiate_db_template)

const std::string server_name = "test-contact_verification_integration-related_records";

/**
testing that id of added related mail propagates successfully
@pre existing check
@pre valid mail ids
@post correct id in contact_check_message_map.mail_archive_id
 */
BOOST_AUTO_TEST_CASE(test_Ids_of_added_related_mail)
{
    setup_testsuite testsuite;
    setup_check check(testsuite.testsuite_handle);
    std::set<unsigned long long> added_mail_archive_ids;

    for(int i=0; i<20; ++i) {
        added_mail_archive_ids.insert(RandomDataGenerator().xuint());
    }

    ::LibFred::OperationContextCreator ctx;

    Fred::Backend::Admin::Contact::Verification::add_related_mail(
        ctx,
        uuid::from_string( check.check_handle_ ),
        added_mail_archive_ids);

    Database::Result requests_res = ctx.get_conn().exec_params(
        "SELECT map_.mail_archive_id AS id_ "
        "   FROM contact_check_message_map AS map_ "
        "       JOIN contact_check AS c_ch ON map_.contact_check_id = c_ch.id "
        "   WHERE c_ch.handle = $1::uuid ",
        Database::query_param_list
            (check.check_handle_)
    );

    std::set<unsigned long long> selected_mail_archive_ids;

    for(Database::Result::Iterator it = requests_res.begin();
        it != requests_res.end();
        ++it
    ) {
        selected_mail_archive_ids.insert(
            static_cast<unsigned long long>((*it)["id_"])
        );
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(
        added_mail_archive_ids.begin(), added_mail_archive_ids.end(),
        selected_mail_archive_ids.begin(), selected_mail_archive_ids.end()
    );
}

/**
testing that id of added related message propagates successfully
@pre existing check
@pre valid message ids
@post correct id in contact_check_message_map.message_archive_id
 */
BOOST_AUTO_TEST_CASE(test_Ids_of_added_related_messages)
{
    setup_testsuite testsuite;
    setup_check check(testsuite.testsuite_handle);
    std::set<unsigned long long> added_message_archive_ids;

    for(int i=0; i<20; ++i) {
        added_message_archive_ids.insert(RandomDataGenerator().xuint());
    }

    ::LibFred::OperationContextCreator ctx;

    Fred::Backend::Admin::Contact::Verification::add_related_messages(
        ctx,
        uuid::from_string( check.check_handle_ ),
        added_message_archive_ids);

    Database::Result requests_res = ctx.get_conn().exec_params(
        "SELECT map_.message_archive_id AS id_ "
        "   FROM contact_check_message_map AS map_ "
        "       JOIN contact_check AS c_ch ON map_.contact_check_id = c_ch.id "
        "   WHERE c_ch.handle = $1::uuid ",
        Database::query_param_list
            (check.check_handle_)
    );

    std::set<unsigned long long> selected_message_archive_ids;

    for(Database::Result::Iterator it = requests_res.begin();
        it != requests_res.end();
        ++it
    ) {
        selected_message_archive_ids.insert(
            static_cast<unsigned long long>((*it)["id_"])
        );
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(
        added_message_archive_ids.begin(), added_message_archive_ids.end(),
        selected_message_archive_ids.begin(), selected_message_archive_ids.end()
    );
}

/**
testing that id of added related object state request propagates successfully
@pre existing check
@pre valid object state request ids
@post correct id in contact_check_object_state_request_map.object_state_request_id
 */
BOOST_AUTO_TEST_CASE(test_Ids_of_added_related_object_state_request)
{
    setup_testsuite testsuite;
    setup_check check(testsuite.testsuite_handle);
    std::set<unsigned long long> added_object_state_request_ids;

    for(int i=0; i<20; ++i) {
        added_object_state_request_ids.insert(RandomDataGenerator().xuint());
    }

    ::LibFred::OperationContextCreator ctx;

    Fred::Backend::Admin::Contact::Verification::add_related_object_state_requests(
        ctx,
        uuid::from_string( check.check_handle_ ),
        added_object_state_request_ids);

    Database::Result requests_res = ctx.get_conn().exec_params(
        "SELECT map_.object_state_request_id AS id_ "
        "   FROM contact_check_object_state_request_map AS map_ "
        "       JOIN contact_check AS c_ch ON map_.contact_check_id = c_ch.id "
        "   WHERE c_ch.handle = $1::uuid ",
        Database::query_param_list
            (check.check_handle_)
    );

    std::set<unsigned long long> selected_object_state_request_ids;

    for(Database::Result::Iterator it = requests_res.begin();
        it != requests_res.end();
        ++it
    ) {
        selected_object_state_request_ids.insert(
            static_cast<unsigned long long>((*it)["id_"])
        );
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(
        added_object_state_request_ids.begin(), added_object_state_request_ids.end(),
        selected_object_state_request_ids.begin(), selected_object_state_request_ids.end()
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
