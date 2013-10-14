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
 *  integration tests for admin/contact/verification/run_all_enqueued_checks.cc
 */

#include "admin/contact/verification/run_all_enqueued_checks.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/opexception.h"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "tests/admin/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

namespace AdminTests {
    struct dummy_testsuite : public setup_testsuite {

        std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > test_impls_;

        dummy_testsuite (Fred::OperationContext& _ctx, const std::vector<std::string>& _test_return_statuses)
            : setup_testsuite(_ctx)
        {
            BOOST_FOREACH(const std::string& status, _test_return_statuses) {
                boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(new AdminTests::DummyTestReturning(_ctx, status));
                test_impls_[temp_ptr->get_name()] = temp_ptr;
                setup_testdef_in_testsuite(_ctx, temp_ptr->get_name(), testsuite_name);
            }
        }
    };
}

void test_Resulting_check_status_impl(std::vector<std::string> _test_statuses, const std::string& _resulting_check_status) {
    Fred::OperationContext ctx1;
    AdminTests::dummy_testsuite suite(ctx1, _test_statuses);
    AdminTests::setup_check check(ctx1, suite.testsuite_name);

    ctx1.commit_transaction();

    try {
        Admin::run_all_enqueued_checks( const_cast<const std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> >& > (suite.test_impls_) );
    } catch (...) {
        BOOST_FAIL("exception during Admin::run_all_enqueued_checks");
    }

    Fred::InfoContactCheckOutput final_check_state;

    try {
        Fred::OperationContext ctx2;
        final_check_state = Fred::InfoContactCheck(check.check_handle_).exec(ctx2);
    } catch (...) {
        BOOST_FAIL("exception during Fred::InfoContactCheck");
    }


    BOOST_CHECK_EQUAL(_test_statuses.size(), final_check_state.tests.size());

    std::vector<std::string>::const_iterator it_status = _test_statuses.begin();
    for(std::vector<Fred::InfoContactCheckOutput::ContactTestResultData>::const_iterator it_test = final_check_state.tests.begin();
        it_test != final_check_state.tests.end();
        ++it_test, ++it_status
    ) {
        BOOST_CHECK_EQUAL(it_test->state_history.back().status_name, *it_status);
    }

    BOOST_CHECK_EQUAL(final_check_state.check_state_history.back().status_name, _resulting_check_status);
}

namespace Test = Fred::ContactTestStatus;
namespace Check = Fred::ContactCheckStatus;

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestRunEnqueuedChecks, AdminTests::contact_garbage_collector)

const std::string server_name = "test-contact_verification_integration-run_all_enqueued_checks";

/**
 enqueueing and running check (with dummy tests) checking statuses after tests completed
 @pre existing testsuite of dummy tests with implementations
 @post correct values present in InfoContactCheck output
  - check result must be in accordance to known dummy tests fixed results
  - test results must relate to dummy test results

 */
BOOST_AUTO_TEST_CASE(test_Resulting_check_status)
{
    std::vector<std::string> statuses;

    // resulting in ok
    statuses = boost::assign::list_of(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::OK);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::OK);

    // resulting in fail
    statuses = boost::assign::list_of(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::OK)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::FAIL);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::FAIL);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::FAIL)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::FAIL);

    // resulting in to_be_decided
    statuses = boost::assign::list_of(Test::ERROR);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::ERROR);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::ERROR)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::ERROR);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::ERROR)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::MANUAL);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::MANUAL);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::MANUAL)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::MANUAL);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::MANUAL)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::TO_BE_DECIDED);

    AdminTests::delete_all_checks_etc();
}

/**
 exception should be thrown whenever any test returns ENQUEUED or RUNNING status
 @pre existing testsuite containing dummy test returning ENQUEUED or RUNNING status
 @post {{{InternalError}}} exception
 */
BOOST_AUTO_TEST_CASE(test_Incorrect_test_return_handling)
{
    // returning ENQUEUED
    {
        Fred::OperationContext ctx;

        boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(
            new AdminTests::DummyTestReturning(ctx, Test::ENQUEUED));

        std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > test_impls_;
        test_impls_[temp_ptr->get_name()] = temp_ptr;

        AdminTests::setup_testsuite testsuite(ctx);
        AdminTests::setup_testdef_in_testsuite(ctx, temp_ptr->get_name(), testsuite.testsuite_name);

        AdminTests::setup_check check(ctx, testsuite.testsuite_name);

        ctx.commit_transaction();

        bool caught_the_right_exception = false;
        try {
            Admin::run_all_enqueued_checks( test_impls_ );
        } catch(Fred::InternalError& ) {
            caught_the_right_exception = true;
        } catch(...) { }

        BOOST_CHECK_EQUAL(caught_the_right_exception, true);
    }

    // returning RUNNING
    {
        Fred::OperationContext ctx;

        boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(
            new AdminTests::DummyTestReturning(ctx, Test::RUNNING));

        std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > test_impls_;
        test_impls_[temp_ptr->get_name()] = temp_ptr;

        AdminTests::setup_testsuite testsuite(ctx);
        AdminTests::setup_testdef_in_testsuite(ctx, temp_ptr->get_name(), testsuite.testsuite_name);

        AdminTests::setup_check check(ctx, testsuite.testsuite_name);

        ctx.commit_transaction();

        bool caught_the_right_exception = false;
        try {
            Admin::run_all_enqueued_checks( test_impls_ );
        } catch(Fred::InternalError& ) {
            caught_the_right_exception = true;
        } catch(...) { }

        BOOST_CHECK_EQUAL(caught_the_right_exception, true);
    }

    AdminTests::delete_all_checks_etc();
}

/**
 when an exception is thrown during test execution status of that test is ERROR and status of check is TO_BE_DECIDED
 @pre existing testsuite containing dummy test throwing an exception
 @post test status ERROR
 @post check status TO_BE_DECIDED
 @post exception is propagated further up
 */
BOOST_AUTO_TEST_CASE(test_Throwing_test_handling)
{
    Fred::OperationContext ctx;

    boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(new AdminTests::DummyThrowingTest(ctx));

    std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > test_impls_;
    test_impls_[temp_ptr->get_name()] = temp_ptr;

    AdminTests::setup_testsuite testsuite(ctx);
    AdminTests::setup_testdef_in_testsuite(ctx, temp_ptr->get_name(), testsuite.testsuite_name);

    AdminTests::setup_check check(ctx, testsuite.testsuite_name);

    ctx.commit_transaction();

    bool caught_some_exception = false;
    try {
        Admin::run_all_enqueued_checks( test_impls_ );
    } catch (...) {
        caught_some_exception = true;
    }
    BOOST_CHECK_EQUAL(caught_some_exception, true);

    Fred::InfoContactCheckOutput final_check_state;

    try {
        Fred::OperationContext ctx2;
        final_check_state = Fred::InfoContactCheck(check.check_handle_).exec(ctx2);
    } catch (...) {
        BOOST_FAIL("exception during Fred::InfoContactCheck");
    }

    BOOST_CHECK_EQUAL(final_check_state.tests.front().state_history.back().status_name, Test::ERROR);

    BOOST_CHECK_EQUAL(final_check_state.check_state_history.back().status_name, Check::TO_BE_DECIDED);

    AdminTests::delete_all_checks_etc();
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
