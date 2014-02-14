/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  integration tests for ListContactChecks operation
 */


#include "src/fredlib/contact/verification/list_enum_objects.h"
#include "src/fredlib/contact/verification/enum_test_status.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <algorithm>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestListEnumObjects_integ, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification-list_enum_objects_integ";

/**
 testing correct returning of existing test result statuses
 @pre existing test result statuses
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_test_result_statuses)
{
    std::vector<std::string> created_statuses;

    for(int i=0; i<5; ++i) {
        created_statuses.push_back(setup_test_status().status_handle_);
    }

    created_statuses.push_back(Fred::ContactTestStatus::ENQUEUED);
    created_statuses.push_back(Fred::ContactTestStatus::RUNNING);
    created_statuses.push_back(Fred::ContactTestStatus::OK);
    created_statuses.push_back(Fred::ContactTestStatus::SKIPPED);
    created_statuses.push_back(Fred::ContactTestStatus::FAIL);
    created_statuses.push_back(Fred::ContactTestStatus::MANUAL);
    created_statuses.push_back(Fred::ContactTestStatus::ERROR);

    std::vector<Fred::test_result_status> listed_statuses = Fred::list_test_result_statuses("en");

    BOOST_CHECK_EQUAL(created_statuses.size(), listed_statuses.size());

    std::vector<std::string> listed_status_handles;
    for(std::vector<Fred::test_result_status>::const_iterator it = listed_statuses.begin();
        it != listed_statuses.end();
        ++it
    ) {
        listed_status_handles.push_back(it->handle);
    }

    std::sort(listed_status_handles.begin(), listed_status_handles.end());
    std::sort(created_statuses.begin(), created_statuses.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        listed_status_handles.begin(), listed_status_handles.end(),
        created_statuses.begin(), created_statuses.end());
}

/**
 testing correct returning of existing check statuses
 @pre existing check statuses
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_check_statuses)
{
    std::vector<std::string> created_statuses;

    for(int i=0; i<5; ++i) {
        created_statuses.push_back(setup_check_status().status_handle);
    }

    created_statuses.push_back(Fred::ContactCheckStatus::ENQUEUED);
    created_statuses.push_back(Fred::ContactCheckStatus::RUNNING);
    created_statuses.push_back(Fred::ContactCheckStatus::AUTO_OK);
    created_statuses.push_back(Fred::ContactCheckStatus::AUTO_FAIL);
    created_statuses.push_back(Fred::ContactCheckStatus::AUTO_TO_BE_DECIDED);
    created_statuses.push_back(Fred::ContactCheckStatus::OK);
    created_statuses.push_back(Fred::ContactCheckStatus::FAIL);
    created_statuses.push_back(Fred::ContactCheckStatus::INVALIDATED);

    std::vector<Fred::check_status> listed_statuses = Fred::list_check_statuses("en");

    BOOST_CHECK_EQUAL(created_statuses.size(), listed_statuses.size());

    std::vector<std::string> listed_status_handles;
    for(std::vector<Fred::check_status>::const_iterator it = listed_statuses.begin();
        it != listed_statuses.end();
        ++it
    ) {
        listed_status_handles.push_back(it->handle);
    }

    std::sort(listed_status_handles.begin(), listed_status_handles.end());
    std::sort(created_statuses.begin(), created_statuses.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        listed_status_handles.begin(), listed_status_handles.end(),
        created_statuses.begin(), created_statuses.end());
}

/**
 testing correct returning of existing test definitions
 @pre existing test definitions
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_test_definitions)
{
    std::vector<std::string> created_tests;

    for(int i=0; i<5; ++i) {
        created_tests.push_back(setup_testdef().testdef_handle_);
    }

    std::vector<Fred::test_definition> listed_tests = Fred::list_test_definitions("en");

    BOOST_CHECK_EQUAL(created_tests.size(), listed_tests.size());

    std::vector<std::string> listed_test_handles;
    for(std::vector<Fred::test_definition>::const_iterator it = listed_tests.begin();
        it != listed_tests.end();
        ++it
    ) {
        listed_test_handles.push_back(it->handle);
    }

    std::sort(listed_test_handles.begin(), listed_test_handles.end());
    std::sort(created_tests.begin(), created_tests.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        listed_test_handles.begin(), listed_test_handles.end(),
        created_tests.begin(), created_tests.end());
}

/**
 testing correct returning of existing testsuite definitions
 @pre existing testsuite definitions
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_testsuite_definitions)
{

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
