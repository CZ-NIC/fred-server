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

#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/db_settings.hh"
#include "util/db/nullable.hh"
#include "util/random_data_generator.hh"

#include "test/libfred/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestCreateContactTest_integ, Test::instantiate_db_template)

const std::string server_name = "test-contact_verification-create_test_integ";

/**
 executing CreateContactTest with only mandatory setup
 @pre existing check handle
 @pre existing test handle in testsuite of given check
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_AUTO_TEST_CASE(test_Exec_mandatory_setup)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_testdef testdef;
    setup_testdef_in_testsuite_of_check(testdef.testdef_handle_, check.check_handle_);

    ::LibFred::CreateContactTest create_test(uuid::from_string(check.check_handle_), testdef.testdef_handle_);
    std::string timezone = "UTC";

    ::LibFred::InfoContactCheck info_check( uuid::from_string( check.check_handle_) );
    ::LibFred::InfoContactCheckOutput pre_create_test_data;

    try {
        ::LibFred::OperationContextCreator ctx1;
        pre_create_test_data = info_check.exec(ctx1, timezone);
    } catch(const ::LibFred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    try {
        ::LibFred::OperationContextCreator ctx2;
        create_test.exec(ctx2);
        ctx2.commit_transaction();
    } catch(const ::LibFred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    ::LibFred::InfoContactCheckOutput result_data;

    try {
        ::LibFred::OperationContextCreator ctx3;
        result_data = info_check.exec(ctx3, timezone);
    } catch(const ::LibFred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    BOOST_REQUIRE(result_data.tests.size() == pre_create_test_data.tests.size() + 1);
    BOOST_CHECK_EQUAL(result_data.tests.back().test_handle, testdef.testdef_handle_);

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
        result_data.tests.front().state_history.front().status_handle,
        ::LibFred::ContactTestStatus::ENQUEUED);
    BOOST_CHECK(result_data.tests.back().state_history.front().logd_request_id.isnull() );
    BOOST_CHECK(result_data.tests.back().state_history.front().error_msg.isnull() );
    BOOST_CHECK_EQUAL(
        result_data.tests.back().state_history.front().local_update_time,
        result_data.tests.back().local_create_time );
}

/**
 executing CreateContactTest with full mandatory + optional setup
 @pre existing check handle
 @pre existing test handle in testsuite of given check
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_AUTO_TEST_CASE(test_Exec_optional_setup)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_testdef testdef;
    setup_testdef_in_testsuite_of_check(testdef.testdef_handle_, check.check_handle_);
    setup_logd_request_id logd_request;

    ::LibFred::CreateContactTest create_test(uuid::from_string(check.check_handle_), testdef.testdef_handle_, logd_request.logd_request_id);
    std::string timezone = "UTC";

    ::LibFred::InfoContactCheck info_check( uuid::from_string( check.check_handle_) );
    ::LibFred::InfoContactCheckOutput pre_create_test_data;

    try {
        ::LibFred::OperationContextCreator ctx1;
        pre_create_test_data = info_check.exec(ctx1, timezone);
    } catch(const ::LibFred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    try {
        ::LibFred::OperationContextCreator ctx2;
        create_test.exec(ctx2);
        ctx2.commit_transaction();
    } catch(const ::LibFred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    ::LibFred::InfoContactCheckOutput result_data;

    try {
        ::LibFred::OperationContextCreator ctx3;
        result_data = info_check.exec(ctx3, timezone);
    } catch(const ::LibFred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    BOOST_REQUIRE(result_data.tests.size() == pre_create_test_data.tests.size() +1);
    BOOST_CHECK_EQUAL(result_data.tests.back().test_handle, testdef.testdef_handle_);

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
        result_data.tests.back().state_history.front().status_handle,
        ::LibFred::ContactTestStatus::ENQUEUED);
    BOOST_CHECK_EQUAL(result_data.tests.back().state_history.front().logd_request_id, logd_request.logd_request_id);
    BOOST_CHECK(result_data.tests.back().state_history.front().error_msg.isnull() );
    BOOST_CHECK_EQUAL(
        result_data.tests.back().state_history.front().local_update_time,
        result_data.tests.back().local_create_time );
}

/**
 setting nonexistent check handle and existing status values and executing operation
 @pre nonexistent check handle
 @pre existing test handle
 @post ExceptionUnknownCheckHandle
*/
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_handle)
{
    setup_nonexistent_check_handle check;
    setup_testdef testdef;

    ::LibFred::CreateContactTest create_test(uuid::from_string(check.check_handle), testdef.testdef_handle_);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx1;
        create_test.exec(ctx1);
        ctx1.commit_transaction();
    } catch(const ::LibFred::ExceptionUnknownCheckHandle& exp) {
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
 @pre nonexistent test handle
 @post ExceptionUnknownTestHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_test_handle)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_nonexistent_testdef_handle testdef;

    ::LibFred::CreateContactTest create_test(uuid::from_string(check.check_handle_), testdef.testdef_handle);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx;
        create_test.exec(ctx);
        ctx.commit_transaction();
    } catch(const ::LibFred::ExceptionUnknownTestHandle& exp) {
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
 @pre test handle not in testsuite of this check
 @post ExceptionUnknownTestHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_test_handle_not_in_suite)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_testdef testdef;
    // deliberately OMITTING setup_testdef_in_testsuite_of_check(...)

    ::LibFred::CreateContactTest create_test(uuid::from_string(check.check_handle_), testdef.testdef_handle_);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx;
        create_test.exec(ctx);
        ctx.commit_transaction();
    } catch(const ::LibFred::ExceptionTestNotInMyTestsuite& exp) {
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
 @pre existing test handle in testsuite of this check
 @pre already existing record related to given check and test
 @post ExceptionCheckTestPairAlreadyExists
 */
BOOST_AUTO_TEST_CASE(test_Exec_violating_unique_check_test_pair)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_testdef testdef;
    setup_testdef_in_testsuite_of_check(testdef.testdef_handle_, check.check_handle_);

    ::LibFred::CreateContactTest create_test(uuid::from_string(check.check_handle_), testdef.testdef_handle_);
    // preparation - the original previously existing record
    ::LibFred::OperationContextCreator ctx1;
    create_test.exec(ctx1);
    ctx1.commit_transaction();

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx2;
        create_test.exec(ctx2);
        ctx2.commit_transaction();
    } catch(const ::LibFred::ExceptionCheckTestPairAlreadyExists& exp) {
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
