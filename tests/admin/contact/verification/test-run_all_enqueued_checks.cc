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

#include "src/admin/contact/verification/run_all_enqueued_checks.h"
#include "src/admin/contact/verification/fill_check_queue.h"
#include <fredlib/admin_contact_verification.h>
#include "src/fredlib/opexception.h"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <map>

#include "tests/admin/contact/verification/setup_utils.h"
#include "util/random.h"


//not using UTF defined main
#define BOOST_TEST_NO_MAIN


struct dummy_testsuite : public setup_empty_testsuite {

    std::map< std::string, boost::shared_ptr<Admin::ContactVerification::Test> > test_impls_;

    dummy_testsuite (const std::vector<std::string>& _test_return_statuses) {
        std::string handle;

        BOOST_FOREACH(const std::string& status, _test_return_statuses) {
            boost::shared_ptr<Admin::ContactVerification::Test> temp_ptr(new DummyTestReturning(status));
            handle = dynamic_cast<DummyTestReturning*>(temp_ptr.get())->get_handle();

            test_impls_[handle] = temp_ptr;
            setup_testdef_in_testsuite(handle, testsuite_handle);
        }
    }
};

void test_Resulting_check_status_impl(std::vector<std::string> _test_statuses, const std::string& _resulting_check_status) {
    dummy_testsuite suite(_test_statuses);
    setup_check check(suite.testsuite_handle);

    Fred::OperationContext ctx1;
    Fred::UpdateContactCheck(
        uuid::from_string(check.check_handle_),
        Fred::ContactCheckStatus::ENQUEUED
    ).exec(ctx1);
    ctx1.commit_transaction();

    try {
        Admin::run_all_enqueued_checks( const_cast<const std::map< std::string, boost::shared_ptr<Admin::ContactVerification::Test> >& > (suite.test_impls_) );
    } catch (...) {
        BOOST_FAIL("exception during Admin::run_all_enqueued_checks");
    }

    Fred::InfoContactCheckOutput final_check_state;

    try {
        Fred::OperationContext ctx2;
        final_check_state = Fred::InfoContactCheck(
            uuid::from_string( check.check_handle_)
        ).exec(ctx2);

    } catch (...) {
        BOOST_FAIL("exception during Fred::InfoContactCheck");
    }


    BOOST_CHECK_EQUAL(_test_statuses.size(), final_check_state.tests.size());

    std::map<std::string, int> occurences;
    for(std::vector<std::string>::const_iterator it = _test_statuses.begin();
        it != _test_statuses.end();
        ++it
    ) {
        if(occurences.find(*it) == occurences.end()) {
            occurences[*it] = 1;
        } else {
            occurences[*it]++;
        }
    }

    // decreasing related occurences so in case of equality should get map with zeroed members
    for(std::vector<Fred::InfoContactCheckOutput::ContactTestResultData>::const_iterator it_test = final_check_state.tests.begin();
        it_test != final_check_state.tests.end();
        ++it_test
    ) {
        occurences[it_test->state_history.back().status_handle]--;
    }

    for(std::map<std::string, int>::const_iterator it = occurences.begin();
        it != occurences.end();
        ++it
    ) {
        BOOST_CHECK_MESSAGE(
            it->second == 0,
            boost::lexical_cast<std::string>(it->second)
            + " difference in expected and resulting occurences of status "
            + it->first
        );
    }

    BOOST_CHECK_EQUAL(final_check_state.check_state_history.back().status_handle, _resulting_check_status);
}

namespace Test = Fred::ContactTestStatus;
namespace Check = Fred::ContactCheckStatus;

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestRunEnqueuedChecks, autoclean_contact_verification_db)

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
    test_Resulting_check_status_impl(statuses, Check::AUTO_OK);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_OK);

    statuses = boost::assign::list_of(Test::OK)(Test::SKIPPED);
    test_Resulting_check_status_impl(statuses, Check::AUTO_OK);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::SKIPPED);
    test_Resulting_check_status_impl(statuses, Check::AUTO_OK);

    statuses = boost::assign::list_of(Test::OK)(Test::SKIPPED)(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_OK);

    // resulting in fail
    statuses = boost::assign::list_of(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::SKIPPED);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::SKIPPED)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::FAIL)(Test::FAIL)(Test::SKIPPED);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::SKIPPED)(Test::FAIL)(Test::FAIL)(Test::FAIL)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::OK)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::OK)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::FAIL)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::FAIL)(Test::SKIPPED)(Test::OK)(Test::OK)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::SKIPPED)(Test::OK)(Test::SKIPPED)(Test::OK)(Test::OK)(Test::OK)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    statuses = boost::assign::list_of(Test::OK)(Test::SKIPPED)(Test::FAIL)(Test::OK)(Test::OK)(Test::SKIPPED)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_FAIL);

    // resulting in to_be_decided
    statuses = boost::assign::list_of(Test::ERROR);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::ERROR);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::ERROR)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::ERROR);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::ERROR)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::MANUAL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::MANUAL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::OK)(Test::OK)(Test::OK)(Test::MANUAL)(Test::OK)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::MANUAL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::FAIL)(Test::MANUAL)(Test::FAIL)(Test::FAIL);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::SKIPPED);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);

    statuses = boost::assign::list_of(Test::FAIL)(Test::MANUAL)(Test::SKIPPED)(Test::FAIL)(Test::ERROR)(Test::OK);
    test_Resulting_check_status_impl(statuses, Check::AUTO_TO_BE_DECIDED);
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

        boost::shared_ptr<Admin::ContactVerification::Test> temp_ptr(
            new DummyTestReturning(Test::ENQUEUED));

        std::string handle = dynamic_cast<DummyTestReturning*>(temp_ptr.get())->get_handle();

        std::map< std::string, boost::shared_ptr<Admin::ContactVerification::Test> > test_impls_;
        test_impls_[handle] = temp_ptr;

        setup_empty_testsuite testsuite;
        setup_testdef_in_testsuite(handle, testsuite.testsuite_handle);

        setup_check check(testsuite.testsuite_handle);

        ctx.commit_transaction();

        try {
            Admin::run_all_enqueued_checks( test_impls_ );
        } catch(...) {
            BOOST_FAIL("should have been swallowed");
        }
    }

    // returning RUNNING
    {
        Fred::OperationContext ctx;

        boost::shared_ptr<Admin::ContactVerification::Test> temp_ptr(
            new DummyTestReturning(Test::RUNNING));

        std::string handle = dynamic_cast<DummyTestReturning*>(temp_ptr.get())->get_handle();

        std::map< std::string, boost::shared_ptr<Admin::ContactVerification::Test> > test_impls_;
        test_impls_[handle] = temp_ptr;

        setup_empty_testsuite testsuite;
        setup_testdef_in_testsuite(handle, testsuite.testsuite_handle);
        setup_check check(testsuite.testsuite_handle);

        try {
            Admin::run_all_enqueued_checks( test_impls_ );
        } catch(...) {
            BOOST_FAIL("should have been swallowed");
        }
    }
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
    boost::shared_ptr<Admin::ContactVerification::Test> temp_ptr(new DummyThrowingTest);
    std::string handle = dynamic_cast<DummyThrowingTest*>(temp_ptr.get())->get_handle();

    std::map< std::string, boost::shared_ptr<Admin::ContactVerification::Test> > test_impls_;
    test_impls_[handle] = temp_ptr;

    setup_empty_testsuite testsuite;
    setup_testdef_in_testsuite(handle, testsuite.testsuite_handle);
    setup_check check(testsuite.testsuite_handle);

    Fred::OperationContext ctx1;
    Fred::UpdateContactCheck(
        uuid::from_string(check.check_handle_),
        Fred::ContactCheckStatus::ENQUEUED
    ).exec(ctx1);
    ctx1.commit_transaction();

    try {
        Admin::run_all_enqueued_checks( test_impls_ );
    } catch (...) {
        BOOST_FAIL("should have been swallowed");
    }

    Fred::InfoContactCheckOutput final_check_state;

    try {
        Fred::OperationContext ctx2;
        final_check_state = Fred::InfoContactCheck( uuid::from_string( check.check_handle_) ).exec(ctx2);
    } catch (...) {
        BOOST_FAIL("exception during Fred::InfoContactCheck");
    }

    BOOST_CHECK_EQUAL(final_check_state.tests.front().state_history.back().status_handle, Test::ERROR);

    BOOST_CHECK_EQUAL(final_check_state.check_state_history.back().status_handle, Check::AUTO_TO_BE_DECIDED);
}

/**
checks/tests created/changed at once has same logd_request_id
@pre enqueued check
@post InfoContactCheck output has same logd_request_ids for changes on check and tests done by same operation
*/
BOOST_AUTO_TEST_CASE(test_Logd_request_id_of_related_changes)
{
    typedef Fred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
    typedef Fred::InfoContactCheckOutput::ContactTestResultData ContactTestResultData;
    typedef Fred::InfoContactCheckOutput::ContactTestResultState ContactTestResultState;

    // creating checkdef
    Fred::OperationContext ctx1;

    boost::shared_ptr<Admin::ContactVerification::Test> temp_ptr1(
        new DummyTestReturning(Test::OK));
    std::string handle1 = dynamic_cast<DummyTestReturning*>(temp_ptr1.get())->get_handle();

    boost::shared_ptr<Admin::ContactVerification::Test> temp_ptr2(
        new DummyTestReturning(Test::FAIL));
    std::string handle2 = dynamic_cast<DummyTestReturning*>(temp_ptr2.get())->get_handle();

    boost::shared_ptr<Admin::ContactVerification::Test> temp_ptr3(
        new DummyTestReturning(Test::MANUAL));
    std::string handle3 = dynamic_cast<DummyTestReturning*>(temp_ptr3.get())->get_handle();

    std::map< std::string, boost::shared_ptr<Admin::ContactVerification::Test> > test_impls_;
    test_impls_[handle1] = temp_ptr1;
    test_impls_[handle2] = temp_ptr2;
    test_impls_[handle3] = temp_ptr3;

    setup_empty_testsuite testsuite;
    setup_testdef_in_testsuite(handle1, testsuite.testsuite_handle);
    setup_testdef_in_testsuite(handle2, testsuite.testsuite_handle);
    setup_testdef_in_testsuite(handle3, testsuite.testsuite_handle);

    setup_check check(testsuite.testsuite_handle);

    ctx1.commit_transaction();

    // effectively creates tests and updates check at once
    Admin::ContactVerificationQueue::fill_check_queue(testsuite.testsuite_handle, 1).exec();
    unsigned long long logd_request_id = Random::integer(0, 2147483647);
    Admin::run_all_enqueued_checks(test_impls_, logd_request_id);

    Fred::InfoContactCheck info_op( uuid::from_string( check.check_handle_) );
    Fred::InfoContactCheckOutput info;
    try {
        Fred::OperationContext ctx2;
        info = info_op.exec(ctx2);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to get check info (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to get check info (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("failed to get check info (3):") + exp.what());
    }

    bool first_check_state = true;
    for( std::vector<ContactCheckState>::const_iterator
            it = info.check_state_history.begin();
            it != info.check_state_history.end();
            ++it
    ) {
        if(first_check_state) {
            first_check_state = false;
            continue;
        }

        BOOST_CHECK_EQUAL(it->logd_request_id.get_value_or_default(), logd_request_id);
    }
    for( std::vector<ContactTestResultData>::const_iterator
            test_it = info.tests.begin();
            test_it != info.tests.end();
            ++test_it
    ) {
        for( std::vector<ContactTestResultState>::const_iterator
                state_it = test_it->state_history.begin();
                state_it != test_it->state_history.end();
                ++state_it
        ) {
            BOOST_CHECK_EQUAL(
                state_it->logd_request_id.get_value_or_default(),
                logd_request_id);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
