/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
 *  integration tests for UpdateContactTest operation
 */

#include <limits>
#include <vector>
#include <utility>
#include <string>

#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/update_test.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/db_settings.hh"
#include "util/optional_value.hh"
#include "util/is_equal_optional_nullable.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"

#include "test/libfred/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/foreach.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestUpdateContactTest_integ, Test::instantiate_db_template)

const std::string server_name = "test-contact_verification-update_test_integ";

typedef ::LibFred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
typedef ::LibFred::InfoContactCheckOutput::ContactTestResultData ContactTestResultData;
typedef ::LibFred::InfoContactCheckOutput::ContactTestResultState ContactTestResultState;
typedef ::LibFred::InfoContactCheckOutput InfoContactCheckOutput;
typedef ::LibFred::InfoContactCheckOutput::ContactTestResultState ContactTestState;

/**
 * implementation of testcases when updated test has just been created
 */
struct setup_create_update_test {
    std::string test_handle_;
    std::string old_status_;
    std::string new_status_;
    Optional<unsigned long long> old_logd_request_;
    Optional<unsigned long long> new_logd_request_;
    Optional<std::string> old_error_msg_;
    Optional<std::string> new_error_msg_;
    ::LibFred::InfoContactCheckOutput data_pre_update_;
    ::LibFred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    setup_create_update_test(
        const std::string& _new_status,
        Optional<unsigned long long> _old_logd_request,
        Optional<unsigned long long> _new_logd_request,
        Optional<std::string> _new_error_msg,
        const std::string& _timezone = "UTC"
    ) :
        old_status_(::LibFred::ContactTestStatus::ENQUEUED),
        new_status_(_new_status),
        old_logd_request_(_old_logd_request),
        new_logd_request_(_new_logd_request),
        old_error_msg_(Optional<std::string>()),
        new_error_msg_(_new_error_msg),
        timezone_(_timezone)
    {
        setup_empty_testsuite suite;
        setup_testdef testdef;
        setup_testdef_in_testsuite(testdef.testdef_handle_, suite.testsuite_handle);
        setup_check check(suite.testsuite_handle, old_logd_request_);
        setup_test(check.check_handle_, testdef.testdef_handle_, old_logd_request_);
        test_handle_ = testdef.testdef_handle_;

        ::LibFred::InfoContactCheck info_check( uuid::from_string( check.check_handle_) );

        try {
            ::LibFred::OperationContextCreator ctx1;
            data_pre_update_ = info_check.exec(ctx1, timezone_);
        } catch(const ::LibFred::InternalError& exp) {
           BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("exception (3):") + exp.what());
        }

        ::LibFred::UpdateContactTest update(
            uuid::from_string(check.check_handle_),
            test_handle_,
            new_status_,
            new_logd_request_,
            new_error_msg_);

        try {
            ::LibFred::OperationContextCreator ctx2;
            update.exec(ctx2);
            ctx2.commit_transaction();
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp)
                +"\n" + data_pre_update_.to_string()
                +"\n" + testdef.testdef_handle_
                +"\n" + new_status_);
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update test (3):") + exp.what());
        }

        try {
            ::LibFred::OperationContextCreator ctx3;
            data_post_update_ = info_check.exec(ctx3, timezone_);
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("exception (3):") + exp.what());
        }
    }
};

/**
 * implementation of testcases when updated test has already been updated before
 */
struct setup_create_update_update_test {
    std::string test_handle_;
    const std::string status1_;
    std::string status2_;
    std::string status3_;
    Optional<unsigned long long> logd_request1_;
    Optional<unsigned long long> logd_request2_;
    Optional<unsigned long long> logd_request3_;
    const Optional<std::string> error_msg1_;
    Optional<std::string> error_msg2_;
    Optional<std::string> error_msg3_;
    ::LibFred::InfoContactCheckOutput data_post_create_;
    ::LibFred::InfoContactCheckOutput data_post_reset_;
    ::LibFred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    setup_create_update_update_test(
        const std::string& _status2,
        const std::string& _status3,
        Optional<unsigned long long> _logd_request1,
        Optional<unsigned long long> _logd_request2,
        Optional<unsigned long long> _logd_request3,
        Optional<std::string> _error_msg2,
        Optional<std::string> _error_msg3,
        const std::string& _timezone = "UTC"
    ) :
        status1_(::LibFred::ContactCheckStatus::ENQUEUE_REQ),
        status2_(_status2),
        status3_(_status3),
        logd_request1_(_logd_request1),
        logd_request2_(_logd_request2),
        logd_request3_(_logd_request3),
        error_msg2_(_error_msg2),
        error_msg3_(_error_msg3),
        timezone_(_timezone)
    {
        setup_testsuite suite;
        setup_testdef testdef;
        setup_testdef_in_testsuite(testdef.testdef_handle_, suite.testsuite_handle);
        setup_check check(suite.testsuite_handle);
        test_handle_ = testdef.testdef_handle_;
        setup_test(check.check_handle_, test_handle_, _logd_request1);

        ::LibFred::InfoContactCheck info_check( uuid::from_string( check.check_handle_) );

        try {
            ::LibFred::OperationContextCreator ctx1;
            data_post_create_ = info_check.exec(ctx1, timezone_);
        } catch(const ::LibFred::InternalError& exp) {
           BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("exception (3):") + exp.what());
        }

        ::LibFred::UpdateContactTest reset(
            uuid::from_string(check.check_handle_),
            test_handle_,
            status2_,
            logd_request2_,
            error_msg2_);

        try {
            ::LibFred::OperationContextCreator ctx2;
            reset.exec(ctx2);
            ctx2.commit_transaction();
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("exception (3):") + exp.what());
        }

        try {
            ::LibFred::OperationContextCreator ctx3;
            data_post_reset_ = info_check.exec(ctx3, timezone_);
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("exception (3):") + exp.what());
        }

        ::LibFred::UpdateContactTest update(
            uuid::from_string(check.check_handle_),
            testdef.testdef_handle_,
            status3_,
            logd_request3_,
            error_msg3_);

        try {
            ::LibFred::OperationContextCreator ctx4;
            update.exec(ctx4);
            ctx4.commit_transaction();
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("exception (3):") + exp.what());
        }

        try {
            ::LibFred::OperationContextCreator ctx5;
            data_post_update_ = info_check.exec(ctx5, timezone_);
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("exception (3):") + exp.what());
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
void check(
    const std::string& test_handle,
    const InfoContactCheckOutput& data_pre_update,
    const InfoContactCheckOutput& data_post_update,
    const std::string& old_status,
    const std::string& new_status,
    Optional<unsigned long long> old_logd_request,
    Optional<unsigned long long> new_logd_request,
    Optional<std::string> old_error_msg,
    Optional<std::string> new_error_msg
) {
    // everything is the same except the last state in history
    BOOST_CHECK_EQUAL( data_pre_update.contact_history_id, data_post_update.contact_history_id );
    BOOST_CHECK_EQUAL( data_pre_update.handle, data_post_update.handle );
    BOOST_CHECK_EQUAL( data_pre_update.local_create_time, data_post_update.local_create_time );
    BOOST_CHECK_EQUAL( data_pre_update.testsuite_handle, data_post_update.testsuite_handle );
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
        if(it->test_handle != test_handle) {
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
                BOOST_CHECK_EQUAL(post_it->state_history.back().status_handle, new_status);
                BOOST_CHECK_EQUAL(post_it->state_history.back().logd_request_id.get_value_or_default(), new_logd_request.get_value_or_default());
                BOOST_CHECK_MESSAGE(
                    Util::is_equal(post_it->state_history.back().error_msg, new_error_msg ),
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
    setup_test_status status1;
    setup_test_status status2;

    std::vector<std::string> status_post_created;
    std::vector<std::pair<std::string, std::string> > status_post_reset;
    status_post_created.push_back(::LibFred::ContactTestStatus::ENQUEUED);
    status_post_reset.push_back(std::make_pair(status1.status_handle_, status1.status_handle_));
    status_post_created.push_back(status1.status_handle_);
    status_post_reset.push_back(std::make_pair(status1.status_handle_, status2.status_handle_));

    Optional<unsigned long long> logd_request_id1 = Random::Generator().get(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    Optional<unsigned long long> logd_request_id2 = Random::Generator().get(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    Optional<unsigned long long> logd_request_id3 = Random::Generator().get(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    std::vector<std::pair<Optional<unsigned long long>, Optional<unsigned long long> > > logd_request_post_created;
    std::vector<boost::tuple<Optional<unsigned long long>, Optional<unsigned long long>, Optional<unsigned long long> > > logd_request_post_reset;
    logd_request_post_created.push_back( std::make_pair( Optional<unsigned long long>(), Optional<unsigned long long>() ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, Optional<long long>() ) );
    logd_request_post_created.push_back( std::make_pair( Optional<unsigned long long>(), logd_request_id1 ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, logd_request_id1 ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(Optional<unsigned long long>(), Optional<unsigned long long>(), Optional<unsigned long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, Optional<unsigned long long>(), Optional<unsigned long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, Optional<unsigned long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, Optional<unsigned long long>(), logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, logd_request_id3 ) );

    Optional<std::string> error_msg_id1 = Random::Generator().get_seq(Random::CharSet::letters_and_digits(), 20);
    Optional<std::string> error_msg_id2 = Random::Generator().get_seq(Random::CharSet::letters_and_digits(), 20);
    std::vector<Optional<std::string> > error_msg_post_created;
    std::vector<std::pair<Optional<std::string>, Optional<std::string> > > error_msg_post_reset;
    error_msg_post_created.push_back( Optional<std::string>() );
    error_msg_post_created.push_back( error_msg_id1 );
    error_msg_post_reset.push_back( std::make_pair(Optional<std::string>(), Optional<std::string>() ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, error_msg_id2 ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, Optional<std::string>() ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, error_msg_id1 ) );
    error_msg_post_reset.push_back( std::make_pair(Optional<std::string>(), error_msg_id1 ) );

    typedef std::pair< Optional<unsigned long long>, Optional<unsigned long long> > logd_request_pair;

    BOOST_FOREACH(const std::string& status, status_post_created) {
        BOOST_FOREACH(logd_request_pair& logd_request, logd_request_post_created) {
            BOOST_FOREACH(const Optional<std::string>& error_msg, error_msg_post_created) {
                setup_create_update_test testcase(status, logd_request.first, logd_request.second, error_msg );
                check(testcase.test_handle_, testcase.data_pre_update_, testcase.data_post_update_, testcase.old_status_, testcase.new_status_, testcase.old_logd_request_, testcase.new_logd_request_, testcase.old_error_msg_, testcase.new_error_msg_);
            }
        }
    }
    typedef std::pair<std::string, std::string> pair_string;
    typedef boost::tuple< Optional<unsigned long long>, Optional<unsigned long long>, Optional<unsigned long long> > logd_request_tuple;
    typedef std::pair<Optional<std::string>, Optional<std::string> > pair_optional_string;
    BOOST_FOREACH(pair_string& status, status_post_reset) {
        BOOST_FOREACH(logd_request_tuple& logd_request, logd_request_post_reset) {
            BOOST_FOREACH(pair_optional_string& error_msg, error_msg_post_reset) {
                setup_create_update_update_test testcase(status.first, status.second, logd_request.get<0>(), logd_request.get<1>(), logd_request.get<2>(), error_msg.first, error_msg.second );
                check(testcase.test_handle_, testcase.data_post_reset_, testcase.data_post_update_, testcase.status2_, testcase.status3_, testcase.logd_request2_, testcase.logd_request3_, testcase.error_msg2_, testcase.error_msg3_);
            }
        }
    }
}

/**
 setting nonexistent check handle, existing test and existing status values and executing operation
 @pre nonexistent check handle
 @pre existing test handle
 @pre existing status handle
 @post ExceptionUnknownCheckHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_handle)
{
    setup_nonexistent_check_handle check;
    setup_testdef testdef;
    setup_test_status status;

    ::LibFred::UpdateContactTest dummy(
        uuid::from_string(check.check_handle),
        testdef.testdef_handle_,
        status.status_handle_);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx;
        dummy.exec(ctx);
        ctx.commit_transaction();
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
 setting nonexistent check handle, existing test and existing status values and executing operation
 @pre existent check handle
 @pre existing test handle
 @pre nonexistent check, test pair record in contact_test_result
 @pre existing status handle
 @post ExceptionUnknownCheckHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_test_pair)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_testdef testdef;
    setup_test_status status;

    ::LibFred::UpdateContactTest dummy(
        uuid::from_string(check.check_handle_),
        testdef.testdef_handle_,
        status.status_handle_);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx;
        dummy.exec(ctx);
        ctx.commit_transaction();
    } catch(const ::LibFred::ExceptionUnknownCheckTestPair& exp) {
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
 @pre nonexistent test handle
 @pre existing status handle
 @post ExceptionUnknownTestHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_test_handle)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_nonexistent_testdef_handle test;
    setup_test_status status;

    ::LibFred::UpdateContactTest dummy(
        uuid::from_string(check.check_handle_),
        test.testdef_handle,
        status.status_handle_);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx;
        dummy.exec(ctx);
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
 setting existing check handle and nonexistent status values and executing operation
 @pre existing check handle
 @pre existing test handle
 @pre existing test defined by check handle and test handle
 @pre nonexistentstatus handle
 @post ExceptionUnknownStatusHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_status_handle)
{
    setup_empty_testsuite suite;
    setup_testdef testdef;
    setup_testdef_in_testsuite(testdef.testdef_handle_, suite.testsuite_handle);
    setup_check check(suite.testsuite_handle);
    setup_nonexistent_test_status_handle status;

    ::LibFred::UpdateContactTest dummy(
        uuid::from_string(check.check_handle_),
        testdef.testdef_handle_,
        status.status_handle);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx;
        dummy.exec(ctx);
        ctx.commit_transaction();
    } catch(const ::LibFred::ExceptionUnknownTestStatusHandle& exp) {
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
