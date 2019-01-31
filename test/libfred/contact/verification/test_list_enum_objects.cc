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


#include "libfred/registrable_object/contact/verification/list_enum_objects.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "test/libfred/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <algorithm>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestListEnumObjects_integ, Test::instantiate_db_template)

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

    created_statuses.push_back(::LibFred::ContactTestStatus::ENQUEUED);
    created_statuses.push_back(::LibFred::ContactTestStatus::RUNNING);
    created_statuses.push_back(::LibFred::ContactTestStatus::OK);
    created_statuses.push_back(::LibFred::ContactTestStatus::SKIPPED);
    created_statuses.push_back(::LibFred::ContactTestStatus::FAIL);
    created_statuses.push_back(::LibFred::ContactTestStatus::MANUAL);
    created_statuses.push_back(::LibFred::ContactTestStatus::ERROR);

    std::vector<::LibFred::test_result_status> listed_statuses = ::LibFred::list_test_result_statuses("en");

    BOOST_CHECK_EQUAL(created_statuses.size(), listed_statuses.size());

    std::vector<std::string> listed_status_handles;
    for(std::vector<::LibFred::test_result_status>::const_iterator it = listed_statuses.begin();
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

    created_statuses = ::LibFred::ContactCheckStatus::get_all();

    for(int i=0; i<5; ++i) {
        created_statuses.push_back(setup_check_status().status_handle);
    }

    std::vector<::LibFred::check_status> listed_statuses = ::LibFred::list_check_statuses("en");

    BOOST_CHECK_EQUAL(created_statuses.size(), listed_statuses.size());

    std::vector<std::string> listed_status_handles;
    for(std::vector<::LibFred::check_status>::const_iterator it = listed_statuses.begin();
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
    std::vector<std::string> existing_tests;
    std::vector<::LibFred::test_definition> preexisting_tests = ::LibFred::list_test_definitions("en");

    for(std::vector<::LibFred::test_definition>::const_iterator it = preexisting_tests.begin();
        it != preexisting_tests.end();
        ++it
    ) {
        existing_tests.push_back(it->handle);
    }

    // creating new tests
    for(int i=0; i<5; ++i) {
        existing_tests.push_back(setup_testdef().testdef_handle_);
    }

    std::vector<::LibFred::test_definition> listed_tests = ::LibFred::list_test_definitions("en");

    BOOST_CHECK_EQUAL(existing_tests.size(), listed_tests.size());

    std::vector<std::string> listed_test_handles;
    for(std::vector<::LibFred::test_definition>::const_iterator it = listed_tests.begin();
        it != listed_tests.end();
        ++it
    ) {
        listed_test_handles.push_back(it->handle);
    }

    std::sort(listed_test_handles.begin(), listed_test_handles.end());
    std::sort(existing_tests.begin(), existing_tests.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        listed_test_handles.begin(), listed_test_handles.end(),
        existing_tests.begin(), existing_tests.end());
}

/**
 testing correct returning of existing testsuite definitions
 @pre existing testsuite definitions
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_testsuite_definitions)
{
    std::vector<std::string> created_testsuites;

    for(int i=0; i<5; ++i) {
        created_testsuites.push_back(setup_testsuite().testsuite_handle);
    }

    created_testsuites.push_back(::LibFred::TestsuiteHandle::AUTOMATIC);
    created_testsuites.push_back(::LibFred::TestsuiteHandle::MANUAL);
    created_testsuites.push_back(::LibFred::TestsuiteHandle::THANK_YOU);

    std::vector<::LibFred::testsuite_definition> listed_testsuites = ::LibFred::list_testsuite_definitions("en");

    BOOST_CHECK_EQUAL(created_testsuites.size(), listed_testsuites.size());

    std::vector<std::string> listed_testsuite_handles;
    for(std::vector<::LibFred::testsuite_definition>::const_iterator it = listed_testsuites.begin();
        it != listed_testsuites.end();
        ++it
    ) {
        listed_testsuite_handles.push_back(it->handle);
    }

    std::sort(listed_testsuite_handles.begin(), listed_testsuite_handles.end());
    std::sort(created_testsuites.begin(), created_testsuites.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        listed_testsuite_handles.begin(), listed_testsuite_handles.end(),
        created_testsuites.begin(), created_testsuites.end());
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

