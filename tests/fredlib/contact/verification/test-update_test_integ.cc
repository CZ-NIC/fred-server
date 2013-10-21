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
 *  integration tests for UpdateContactTest operation
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/update_test.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/optional_nullable_equal.h"
#include "random_data_generator.h"

#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include <boost/foreach.hpp>

/* TODO - FIXME - only temporary for uuid mockup */
#include  <cstdlib>
#include "util/random_data_generator.h"

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestUpdateContactTest_integ, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification-update_test_integ";

typedef Fred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
typedef Fred::InfoContactCheckOutput::ContactTestResultData ContactTestResultData;
typedef Fred::InfoContactCheckOutput::ContactTestResultState ContactTestResultState;
typedef Fred::InfoContactCheckOutput InfoContactCheckOutput;
typedef Fred::InfoContactCheckOutput::ContactTestResultState ContactTestState;

/**
 * implementation of testcases when updated test has just been created
 */
struct setup_create_update_test : public setup_test {
    std::string old_status_;
    std::string new_status_;
    Optional<long long> old_logd_request_;
    Optional<long long> new_logd_request_;
    Optional<std::string> old_error_msg_;
    Optional<std::string> new_error_msg_;
    Fred::InfoContactCheckOutput data_pre_update_;
    Fred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    setup_create_update_test(
        Fred::OperationContext& _ctx,
        const std::string& _testdef_name,
        const std::string& _new_status,
        Optional<long long> _old_logd_request,
        Optional<long long> _new_logd_request,
        Optional<std::string> _new_error_msg,
        const std::string& _timezone = "UTC"
    ) :
        setup_test(_ctx, _testdef_name, _old_logd_request),
        old_status_(Fred::ContactTestStatus::ENQUEUED),
        new_status_(_new_status),
        old_logd_request_(_old_logd_request),
        new_logd_request_(_new_logd_request),
        old_error_msg_(Optional<std::string>()),
        new_error_msg_(_new_error_msg),
        timezone_(_timezone)
    {
        Fred::InfoContactCheck info_check(check_handle_);

        try {
            data_pre_update_ = info_check.exec(_ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
           BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        Fred::UpdateContactTest update(check_handle_, testdef_name_, new_status_, new_logd_request_, new_error_msg_);
        try {
            update.exec(_ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp)
                +"\n" + data_pre_update_.to_string()
                +"\n" + testdef_name_
                +"\n" + new_status_);
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update test (3):") + exp.what());
        }

        try {
            data_post_update_ = info_check.exec(_ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }
    }
};

/**
 * implementation of testcases when updated test has already been updated before
 */
struct setup_create_update_update_test : public virtual setup_test {
    const std::string status1_;
    std::string status2_;
    std::string status3_;
    Optional<long long> logd_request1_;
    Optional<long long> logd_request2_;
    Optional<long long> logd_request3_;
    const Optional<std::string> error_msg1_;
    Optional<std::string> error_msg2_;
    Optional<std::string> error_msg3_;
    Fred::InfoContactCheckOutput data_post_create_;
    Fred::InfoContactCheckOutput data_post_reset_;
    Fred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    setup_create_update_update_test(
        Fred::OperationContext& _ctx,
        const std::string& _test_name,
        const std::string& _status2,
        const std::string& _status3,
        Optional<long long> _logd_request1,
        Optional<long long> _logd_request2,
        Optional<long long> _logd_request3,
        Optional<std::string> _error_msg2,
        Optional<std::string> _error_msg3,
        const std::string& _timezone = "UTC"
    ) :
        setup_test(_ctx, _test_name, _logd_request1),
        status1_(Fred::ContactCheckStatus::ENQUEUED),
        status2_(_status2),
        status3_(_status3),
        logd_request1_(_logd_request1),
        logd_request2_(_logd_request2),
        logd_request3_(_logd_request3),
        error_msg2_(_error_msg2),
        error_msg3_(_error_msg3),
        timezone_(_timezone)
    {
        Fred::InfoContactCheck info_check(check_handle_);

        try {
            data_post_create_ = info_check.exec(_ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
           BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        Fred::UpdateContactTest reset(check_handle_, testdef_name_, status2_, logd_request2_, error_msg2_);
        try {
            reset.exec(_ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update test (3):") + exp.what());
        }

        try {
            data_post_reset_ = info_check.exec(_ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        Fred::UpdateContactTest update(check_handle_, testdef_name_, status3_, logd_request3_, error_msg3_);
        try {
            update.exec(_ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update test (3):") + exp.what());
        }

        try {
            data_post_update_ = info_check.exec(_ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }
    }
};

/**
 * testing logic for whole testsuite
 * @param data_pre_update check (yes, whole check) data before update operation has been executed
 * @param data_post_update check (yes, whole check) data after update operation has been executed
 * @param old_status status of test before update operation has been executed
 * @param new_status status set in update operation (input param of UpdateContactTest operation)
 * @param old_logd_request logd request related to test before update operation has been executed
 * @param new_logd_request logd request related to test set in update operation (input param of UpdateContactTest operation)
 * @param old_error_msg error message related to test before update operation has been executed
 * @param new_error_msg error message related to test set in update operation (input param of UpdateContactTest operation)
 */
void check(const std::string& testname, const InfoContactCheckOutput& data_pre_update, const InfoContactCheckOutput& data_post_update, const std::string& old_status, const std::string& new_status, Optional<long long> old_logd_request, Optional<long long> new_logd_request, Optional<std::string> old_error_msg, Optional<std::string> new_error_msg) {
    // everything is the same except the last state in history
    BOOST_CHECK_EQUAL( data_pre_update.contact_history_id, data_post_update.contact_history_id );
    BOOST_CHECK_EQUAL( data_pre_update.handle, data_post_update.handle );
    BOOST_CHECK_EQUAL( data_pre_update.local_create_time, data_post_update.local_create_time );
    BOOST_CHECK_EQUAL( data_pre_update.testsuite_name, data_post_update.testsuite_name );
    BOOST_CHECK_EQUAL( data_pre_update.check_state_history.size(), data_post_update.check_state_history.size() );
    for(
        std::vector<ContactCheckState>::const_iterator it = data_pre_update.check_state_history.begin(), post_it = data_post_update.check_state_history.begin();
        it != data_pre_update.check_state_history.end();
        ++it, ++post_it)
    {
        BOOST_CHECK_MESSAGE(
            it->to_string() == post_it->to_string(),
            data_pre_update.to_string() + "\n" + data_post_update.to_string()
            );
    }

    BOOST_CHECK_EQUAL( data_pre_update.tests.size(), data_post_update.tests.size());
    std::vector<ContactTestResultData>::const_iterator post_it = data_post_update.tests.begin();
    for(std::vector<ContactTestResultData>::const_iterator it = data_pre_update.tests.begin(); it != data_pre_update.tests.end(); ++it, ++post_it) {
        if(it->test_name != testname) {
            BOOST_CHECK_EQUAL(it->to_string(), post_it->to_string());
        } else {
            if(old_status != new_status || old_logd_request != new_logd_request || old_error_msg.print_quoted() != new_error_msg.print_quoted() ) {

                BOOST_CHECK_EQUAL( it->state_history.size() + 1, post_it->state_history.size());
                for(
                    std::vector<ContactTestState>::const_iterator
                        pre_history_it = it->state_history.begin(),
                        post_history_it = post_it->state_history.begin();
                    pre_history_it != it->state_history.end();
                   ++pre_history_it, ++post_history_it
                ) {
                   BOOST_CHECK_EQUAL(pre_history_it->to_string(), post_history_it->to_string() );
                }

                // new state in history
                // update_time is reasonable
                ptime now = second_clock::universal_time();
                ptime update_time_min = now - minutes(1);
                ptime update_time_max = now + minutes(1);

                BOOST_CHECK_MESSAGE(
                    post_it->state_history.back().local_update_time > update_time_min,
                    "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(post_it->state_history.back().local_update_time)
                    + " 'now' is:" + boost::posix_time::to_simple_string(now) );
                BOOST_CHECK_MESSAGE(
                    post_it->state_history.back().local_update_time < update_time_max,
                    "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(post_it->state_history.back().local_update_time)
                    + " 'now' is:" + boost::posix_time::to_simple_string(now) );
                BOOST_CHECK_EQUAL(post_it->state_history.back().status_name, new_status);
                BOOST_CHECK_EQUAL(post_it->state_history.back().logd_request_id, new_logd_request);
                BOOST_CHECK_MESSAGE(
                    equal(post_it->state_history.back().error_msg, new_error_msg ),
                    std::string("difference in post_it->state_history.back() and new_error_msg")
                    + post_it->state_history.back().error_msg.print_quoted() + "\n"
                    + new_error_msg.print_quoted()
                );
            } else {
                BOOST_CHECK_EQUAL( it->state_history.size(), post_it->state_history.size());
                for(
                    std::vector<ContactTestState>::const_iterator
                        pre_history_it = it->state_history.begin(),
                        post_history_it = post_it->state_history.begin();
                    pre_history_it != it->state_history.end();
                    ++pre_history_it, ++post_history_it
                ) {
                    BOOST_CHECK_EQUAL(pre_history_it->to_string(), post_history_it->to_string() );
                }
            }
        }
    }
}

/**
 combinations of exec() with different original test state and different new values:
 @pre same/different original->new status
 @pre same/different original->new logd request (including NULL values)
 @pre same/different original->new error message (including NULL values)
 */
BOOST_AUTO_TEST_CASE(test_Update)
{
    Fred::OperationContext ctx;

    setup_test_status status1(ctx);
    setup_test_status status2(ctx);

    std::vector<std::string> status_post_created;
    std::vector<std::pair<std::string, std::string> > status_post_reset;
    status_post_created.push_back(Fred::ContactTestStatus::ENQUEUED);
    status_post_reset.push_back(std::make_pair(status1.status_name_, status1.status_name_));
    status_post_created.push_back(status1.status_name_);
    status_post_reset.push_back(std::make_pair(status1.status_name_, status2.status_name_));

    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id3 = RandomDataGenerator().xuint();
    std::vector<std::pair<Optional<long long>, Optional<long long> > > logd_request_post_created;
    std::vector<boost::tuple<Optional<long long>, Optional<long long>, Optional<long long> > > logd_request_post_reset;
    logd_request_post_created.push_back( std::make_pair( Optional<long long>(), Optional<long long>() ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, Optional<long long>() ) );
    logd_request_post_created.push_back( std::make_pair( Optional<long long>(), logd_request_id1 ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, logd_request_id1 ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(Optional<long long>(), Optional<long long>(), Optional<long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, Optional<long long>(), Optional<long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, Optional<long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, Optional<long long>(), logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, logd_request_id3 ) );

    Optional<std::string> error_msg_id1 = RandomDataGenerator().xnstring(20);
    Optional<std::string> error_msg_id2 = RandomDataGenerator().xnstring(20);
    std::vector<Optional<std::string> > error_msg_post_created;
    std::vector<std::pair<Optional<std::string>, Optional<std::string> > > error_msg_post_reset;
    error_msg_post_created.push_back( Optional<std::string>() );
    error_msg_post_created.push_back( error_msg_id1 );
    error_msg_post_reset.push_back( std::make_pair(Optional<std::string>(), Optional<std::string>() ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, error_msg_id2 ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, Optional<std::string>() ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, error_msg_id1 ) );
    error_msg_post_reset.push_back( std::make_pair(Optional<std::string>(), error_msg_id1 ) );

    typedef std::pair< Optional<long long>, Optional<long long> > logd_request_pair;

    BOOST_FOREACH(const std::string& status, status_post_created) {
        BOOST_FOREACH(logd_request_pair& logd_request, logd_request_post_created) {
            BOOST_FOREACH(const Optional<std::string>& error_msg, error_msg_post_created) {
                setup_testdef def(ctx);
                setup_create_update_test testcase(ctx, def.testdef_name_, status, logd_request.first, logd_request.second, error_msg );
                check(testcase.testdef_name_, testcase.data_pre_update_, testcase.data_post_update_, testcase.old_status_, testcase.new_status_, testcase.old_logd_request_, testcase.new_logd_request_, testcase.old_error_msg_, testcase.new_error_msg_);
            }
        }
    }
    typedef std::pair<std::string, std::string> pair_string;
    typedef boost::tuple< Optional<long long>, Optional<long long>, Optional<long long> > logd_request_tuple;
    typedef std::pair<Optional<std::string>, Optional<std::string> > pair_optional_string;
    BOOST_FOREACH(pair_string& status, status_post_reset) {
        BOOST_FOREACH(logd_request_tuple& logd_request, logd_request_post_reset) {
            BOOST_FOREACH(pair_optional_string& error_msg, error_msg_post_reset) {
                setup_testdef def(ctx);
                setup_create_update_update_test testcase(ctx, def.testdef_name_, status.first, status.second, logd_request.get<0>(), logd_request.get<1>(), logd_request.get<2>(), error_msg.first, error_msg.second );
                check(testcase.testdef_name_, testcase.data_post_reset_, testcase.data_post_update_, testcase.status2_, testcase.status3_, testcase.logd_request2_, testcase.logd_request3_, testcase.error_msg2_, testcase.error_msg3_);
            }
        }
    }
}

/**
 setting nonexistent check handle, existing test and existing status values and executing operation
 @pre nonexistent check handle
 @pre existing test name
 @pre existing status name
 @post ExceptionUnknownCheckHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_handle)
{
    Fred::OperationContext ctx;
    setup_nonexistent_check_handle check(ctx);
    setup_testdef testdef(ctx);
    setup_test_status status(ctx);

    Fred::UpdateContactTest dummy(check.check_handle, testdef.testdef_name_, status.status_name_);

    bool caught_the_right_exception = false;
    try {
        dummy.exec(ctx);
    } catch(const Fred::UpdateContactTest::ExceptionUnknownCheckHandle& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting nonexistent check handle, existing test and existing status values and executing operation
 @pre existent check handle
 @pre existing test name
 @pre nonexistent check, test pair record in contact_test_result
 @pre existing status name
 @post ExceptionUnknownCheckHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_test_pair)
{
    Fred::OperationContext ctx;
    setup_check check(ctx);
    setup_testdef testdef(ctx);
    setup_test_status status(ctx);

    Fred::UpdateContactTest dummy(check.check_handle_, testdef.testdef_name_, status.status_name_);

    bool caught_the_right_exception = false;
    try {
        dummy.exec(ctx);
    } catch(const Fred::UpdateContactTest::ExceptionUnknownCheckTestPair& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting existing check handle and nonexistent status values and executing operation
 @pre existing check handle
 @pre nonexistent test name
 @pre existing status name
 @post ExceptionUnknownTestName
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_test_name)
{
    Fred::OperationContext ctx;
    setup_check check(ctx);
    setup_nonexistent_testdef_name test(ctx);
    setup_test_status status(ctx);

    Fred::UpdateContactTest dummy(check.check_handle_, test.testdef_name, status.status_name_);

    bool caught_the_right_exception = false;
    try {
        dummy.exec(ctx);
    } catch(const Fred::UpdateContactTest::ExceptionUnknownTestName& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting existing check handle and nonexistent status values and executing operation
 @pre existing check handle
 @pre existing test name
 @pre existing test defined by check handle and test name
 @pre nonexistentstatus name
 @post ExceptionUnknownStatusName
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_status_name)
{
    Fred::OperationContext ctx;
    setup_testdef testdef(ctx);
    setup_test test(ctx, testdef.testdef_name_);
    setup_nonexistent_test_status_name status(ctx);

    Fred::UpdateContactTest dummy(test.check_handle_, test.testdef_name_, status.status_name_);

    bool caught_the_right_exception = false;
    try {
        dummy.exec(ctx);
    } catch(const Fred::UpdateContactTest::ExceptionUnknownStatusName& exp) {
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
