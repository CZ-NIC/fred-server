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
 *  integration tests for CreateContactTest operation
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "random_data_generator.h"

#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestCreateContactTest_integ, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification-create_test_integ";

/**
 executing CreateContactTest with only mandatory setup
 @pre existing check handle
 @pre existing test name in testsuite of given check
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_AUTO_TEST_CASE(test_Exec_mandatory_setup)
{
    Fred::OperationContext ctx;

    setup_check check;
    setup_testdef testdef;
    setup_testdef_in_testsuite_of_check(testdef.testdef_name_, check.check_handle_);

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name_);
    std::string timezone = "UTC";

    Fred::InfoContactCheck info_check(check.check_handle_);
    Fred::InfoContactCheckOutput pre_create_test_data;

    try {
        pre_create_test_data = info_check.exec(ctx, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
    }

    try {
        create_test.exec(ctx);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to create test (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to create test (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("failed to create test (3):") + exp.what());
    }

    Fred::InfoContactCheckOutput result_data;

    try {
        result_data = info_check.exec(ctx, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
    }

    BOOST_REQUIRE(result_data.tests.size() == pre_create_test_data.tests.size() + 1);
    BOOST_CHECK_EQUAL(result_data.tests.back().test_name, testdef.testdef_name_);

    // create_time is reasonable
    ptime now = second_clock::universal_time();
    ptime create_time_min = now - minutes(1);
    ptime create_time_max = now + minutes(1);

    BOOST_CHECK_MESSAGE(
        result_data.tests.back().local_create_time > create_time_min,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_CHECK_MESSAGE(
        result_data.tests.back().local_create_time < create_time_max,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_REQUIRE(result_data.tests.back().state_history.size() == 1);
    BOOST_CHECK_EQUAL(
        result_data.tests.front().state_history.front().status_name,
        Fred::ContactTestStatus::ENQUEUED);
    BOOST_CHECK(result_data.tests.back().state_history.front().logd_request_id.isnull() );
    BOOST_CHECK(result_data.tests.back().state_history.front().error_msg.isnull() );
    BOOST_CHECK_EQUAL(
        result_data.tests.back().state_history.front().local_update_time,
        result_data.tests.back().local_create_time );
}

/**
 executing CreateContactTest with full mandatory + optional setup
 @pre existing check handle
 @pre existing test name in testsuite of given check
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_AUTO_TEST_CASE(test_Exec_optional_setup)
{
    Fred::OperationContext ctx;

    setup_check check;
    setup_testdef testdef;
    setup_testdef_in_testsuite_of_check(testdef.testdef_name_, check.check_handle_);
    setup_logd_request_id logd_request;

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name_, logd_request.logd_request_id);
    std::string timezone = "UTC";

    Fred::InfoContactCheck info_check(check.check_handle_);
    Fred::InfoContactCheckOutput pre_create_test_data;

    try {
        pre_create_test_data = info_check.exec(ctx, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
    }

    try {
        create_test.exec(ctx);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to create test (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to create test (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("failed to create test (3):") + exp.what());
    }

    Fred::InfoContactCheckOutput result_data;

    try {
        result_data = info_check.exec(ctx, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
    }

    BOOST_REQUIRE(result_data.tests.size() == pre_create_test_data.tests.size() +1);
    BOOST_CHECK_EQUAL(result_data.tests.back().test_name, testdef.testdef_name_);

    // create_time is reasonable
    ptime now = second_clock::universal_time();
    ptime create_time_min = now - minutes(1);
    ptime create_time_max = now + minutes(1);

    BOOST_CHECK_MESSAGE(
        result_data.tests.back().local_create_time > create_time_min,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_CHECK_MESSAGE(
        result_data.tests.back().local_create_time < create_time_max,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_REQUIRE(result_data.tests.back().state_history.size() == 1);
    BOOST_CHECK_EQUAL(
        result_data.tests.back().state_history.front().status_name,
        Fred::ContactTestStatus::ENQUEUED);
    BOOST_CHECK_EQUAL(result_data.tests.back().state_history.front().logd_request_id, logd_request.logd_request_id);
    BOOST_CHECK(result_data.tests.back().state_history.front().error_msg.isnull() );
    BOOST_CHECK_EQUAL(
        result_data.tests.back().state_history.front().local_update_time,
        result_data.tests.back().local_create_time );
}

/**
 setting nonexistent check handle and existing status values and executing operation
 @pre nonexistent check handle
 @pre existing test name
 @post ExceptionUnknownCheckHandle
*/
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_handle)
{
    Fred::OperationContext ctx;

    setup_nonexistent_check_handle check;
    setup_testdef testdef;

    Fred::CreateContactTest create_test(check.check_handle, testdef.testdef_name_);

    bool caught_the_right_exception = false;
    try {
        create_test.exec(ctx);
    } catch(const Fred::CreateContactTest::ExceptionUnknownCheckHandle& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting existing check handle and nonexistent test values and executing operation
 @pre existing check handle
 @pre nonexistent test name
 @post ExceptionUnknownTestName
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_test_name)
{
    Fred::OperationContext ctx;

    setup_check check;
    setup_nonexistent_testdef_name testdef;

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name);

    bool caught_the_right_exception = false;
    try {
        create_test.exec(ctx);
    } catch(const Fred::CreateContactTest::ExceptionUnknownTestName& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 * setting existing check handle and nonexistent test values and executing operation
 @pre existing check handle
 @pre test name not in testsuite of this check
 @post ExceptionUnknownTestName
 */
BOOST_AUTO_TEST_CASE(test_Exec_test_name_not_in_suite)
{
    Fred::OperationContext ctx;

    setup_check check;
    setup_testdef testdef;
    // deliberately OMITTING setup_testdef_in_testsuite_of_check(...)

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name_);

    bool caught_the_right_exception = false;
    try {
        create_test.exec(ctx);
    } catch(const Fred::CreateContactTest::ExceptionTestNotInMyTestsuite& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting existing check handle and existing test from check's testsuite which combination is already created values and executing operation
 @pre existing check handle
 @pre existing test name in testsuite of this check
 @pre already existing record related to given check and test
 @post ExceptionCheckTestPairAlreadyExists
 */
BOOST_AUTO_TEST_CASE(test_Exec_violating_unique_check_test_pair)
{
    Fred::OperationContext ctx;

    setup_check check;
    setup_testdef testdef;
    setup_testdef_in_testsuite_of_check(testdef.testdef_name_, check.check_handle_);

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name_);
    // preparation - the original previously existing record
    create_test.exec(ctx);

    bool caught_the_right_exception = false;
    try {
        create_test.exec(ctx);
    } catch(const Fred::CreateContactTest::ExceptionCheckTestPairAlreadyExists& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
