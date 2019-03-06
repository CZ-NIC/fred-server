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
 *  integration tests for UpdateContactCheck operation
 */

#include <vector>
#include <utility>
#include <string>

#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/db_settings.hh"
#include "util/optional_value.hh"
#include "util/random_data_generator.hh"

#include "test/libfred/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestUpdateContactCheck_integ, Test::instantiate_db_template)

const std::string server_name = "test-contact_verification-update_check_integ";

typedef ::LibFred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
typedef ::LibFred::InfoContactCheckOutput::ContactTestResultData ContactTestResultData;
typedef ::LibFred::InfoContactCheckOutput::ContactTestResultState ContactTestResultState;
typedef ::LibFred::InfoContactCheckOutput InfoContactCheckOutput;

/**
 * implementation of testcases when updated check has just been created
 */
struct setup_create_update_check {
    std::string old_status_;
    std::string new_status_;
    Optional<unsigned long long> old_logd_request_;
    Optional<unsigned long long> new_logd_request_;
    ::LibFred::InfoContactCheckOutput data_pre_update_;
    ::LibFred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    setup_create_update_check(
        const std::string& _new_status,
        Optional<unsigned long long> _old_logd_request,
        Optional<unsigned long long> _new_logd_request,
        const std::string& _timezone = "UTC"
    ) :
        old_status_(::LibFred::ContactCheckStatus::ENQUEUE_REQ),
        new_status_(_new_status),
        old_logd_request_(_old_logd_request),
        new_logd_request_(_new_logd_request),
        timezone_(_timezone)
    {
        setup_empty_testsuite suite;
        setup_testdef testdef;
        setup_testdef_in_testsuite(testdef.testdef_handle_, suite.testsuite_handle);
        setup_check check(suite.testsuite_handle, _old_logd_request);

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

        ::LibFred::UpdateContactCheck update(
            uuid::from_string( check.check_handle_ ),
            new_status_,
            new_logd_request_);

        try {
            ::LibFred::OperationContextCreator ctx2;
            update.exec(ctx2);
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
 * implementation of testcases when updated check has already been updated before
 */
struct setup_create_update_update_check  {
    std::string status1_;
    std::string status2_;
    std::string status3_;
    Optional<unsigned long long> logd_request1_;
    Optional<unsigned long long> logd_request2_;
    Optional<unsigned long long> logd_request3_;
    ::LibFred::InfoContactCheckOutput data_post_create_;
    ::LibFred::InfoContactCheckOutput data_post_reset_;
    ::LibFred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    setup_create_update_update_check(
        const std::string& _status2,
        const std::string& _status3,
        Optional<unsigned long long> _logd_request1,
        Optional<unsigned long long> _logd_request2,
        Optional<unsigned long long> _logd_request3,
        const std::string& _timezone = "UTC"
    ) :
        status1_(::LibFred::ContactCheckStatus::ENQUEUE_REQ),
        status2_(_status2),
        status3_(_status3),
        logd_request1_(_logd_request1),
        logd_request2_(_logd_request2),
        logd_request3_(_logd_request3),
        timezone_(_timezone)
    {
        setup_empty_testsuite suite;
        setup_testdef testdef;
        setup_testdef_in_testsuite(testdef.testdef_handle_, suite.testsuite_handle);
        setup_check check(suite.testsuite_handle, _logd_request1);

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

        ::LibFred::UpdateContactCheck reset(
            uuid::from_string( check.check_handle_),
            status2_,
            logd_request2_);

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

        ::LibFred::UpdateContactCheck update(
            uuid::from_string(check.check_handle_),
            status3_,
            logd_request3_);

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
 * @param data_pre_update check data before update operation has been executed
 * @param data_post_update check data after update operation has been executed
 * @param old_status status of check before update operation has been executed
 * @param new_status status set in update operation (input param of UpdateContactCheck operation)
 * @param old_logd_request logd request related to check before update operation has been executed
 * @param new_logd_request logd request related to check set in update operation (input param of UpdateContactCheck operation)
 */
void check(
    const InfoContactCheckOutput& data_pre_update,
    const InfoContactCheckOutput& data_post_update,
    const std::string& old_status,
    const std::string& new_status,
    Optional<unsigned long long> old_logd_request,
    Optional<unsigned long long> new_logd_request
) {
    // everything is the same except the last state in history
    BOOST_CHECK_EQUAL( data_pre_update.contact_history_id, data_post_update.contact_history_id );
    BOOST_CHECK_EQUAL( data_pre_update.handle, data_post_update.handle );
    BOOST_CHECK_EQUAL( data_pre_update.local_create_time, data_post_update.local_create_time );
    BOOST_CHECK_EQUAL( data_pre_update.testsuite_handle, data_post_update.testsuite_handle );
    {
        BOOST_CHECK_EQUAL( data_pre_update.tests.size(), data_post_update.tests .size());
        std::vector<ContactTestResultData>::const_iterator post_it = data_post_update.tests.begin();
        for(std::vector<ContactTestResultData>::const_iterator it = data_pre_update.tests.begin(); it != data_pre_update.tests.end(); ++it, ++post_it) {
            BOOST_CHECK_EQUAL(it->to_string(), post_it->to_string());
        }
    }

    if(old_status != new_status || old_logd_request != new_logd_request) {
        BOOST_CHECK_EQUAL( data_pre_update.check_state_history.size() + 1, data_post_update.check_state_history.size() );
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

        // new state in history
        // update_time is reasonable
        ptime now = second_clock::universal_time();
        ptime update_time_min = now - minutes(1);
        ptime update_time_max = now + minutes(1);

        BOOST_CHECK_MESSAGE(
            data_post_update.check_state_history.back().local_update_time > update_time_min,
            "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(data_post_update.check_state_history.back().local_update_time)
            + " 'now' is:" + boost::posix_time::to_simple_string(now) );
        BOOST_CHECK_MESSAGE(
            data_post_update.check_state_history.back().local_update_time < update_time_max,
            "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(data_post_update.check_state_history.back().local_update_time)
            + " 'now' is:" + boost::posix_time::to_simple_string(now) );
        BOOST_CHECK_EQUAL(data_post_update.check_state_history.back().status_handle, new_status);
        BOOST_CHECK_EQUAL(data_post_update.check_state_history.back().logd_request_id.get_value_or_default(), new_logd_request.get_value_or_default());
    } else {
        BOOST_CHECK_EQUAL( data_pre_update.check_state_history.size(), data_post_update.check_state_history.size() );
        std::vector<ContactCheckState>::const_iterator post_it = data_post_update.check_state_history.begin();
        for(std::vector<ContactCheckState>::const_iterator it = data_pre_update.check_state_history.begin(); it != data_pre_update.check_state_history.end(); ++it, ++post_it) {
            BOOST_CHECK_EQUAL(it->to_string(), post_it->to_string());
        }
    }
}

/**
 @pre handle of existing contact_check with status=X and logd_request=1
 @post correct values present in InfoContactCheck output
 @post no change in history values in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_request1_to_statusX_logd_request1)
{
    setup_check_status status;

    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id3 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        ::LibFred::ContactCheckStatus::ENQUEUE_REQ,
        logd_request_id1, logd_request_id1);
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status.status_handle, status.status_handle,
        logd_request_id2, logd_request_id3, logd_request_id3);
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=1
 @pre existing status handle Y different from status X set to check right now
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_request1_to_statusY_logd_request1)
{
    setup_check_status status1;
    setup_check_status status2;

    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id3 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        status2.status_handle,
        logd_request_id1, logd_request_id1);
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status1.status_handle, status2.status_handle,
        logd_request_id2, logd_request_id3, logd_request_id3);
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=1
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_request1_to_statusX_logd_requestNULL)
{
    setup_check_status status;

    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id2 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        ::LibFred::ContactCheckStatus::ENQUEUE_REQ,
        logd_request_id1, Optional<unsigned long long>());
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status.status_handle, status.status_handle,
        Optional<unsigned long long>(), logd_request_id2, Optional<unsigned long long>());
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=1
 @pre existing status handle Y different from status X set to check right now
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_request1_to_statusY_logd_requestNULL)
{
    setup_check_status status1;
    setup_check_status status2;

    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id2 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        status1.status_handle,
        logd_request_id1, Optional<unsigned long long>());
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status1.status_handle, status2.status_handle,
        Optional<unsigned long long>(), logd_request_id2, Optional<unsigned long long>());
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @pre valid logd request 1
 @post correct values present in InfoContactCheck output
 @post no change in history values in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_requestNULL_to_statusX_logd_request1)
{
    setup_check_status status;

    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id3 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        ::LibFred::ContactCheckStatus::ENQUEUE_REQ,
        Optional<unsigned long long>(), logd_request_id1 );
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status.status_handle, status.status_handle,
        logd_request_id2, Optional<unsigned long long>(), logd_request_id3);
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @pre existing status handle Y different from status X set to check right now
 @pre valid logd request 1
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_requestNULL_to_statusY_logd_request1)
{
    setup_check_status status1;
    setup_check_status status2;

    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<unsigned long long> logd_request_id3 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        status1.status_handle,
        Optional<unsigned long long>(), logd_request_id1 );
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status1.status_handle, status2.status_handle,
        logd_request_id2, Optional<unsigned long long>(), logd_request_id3);
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @post correct values present in InfoContactCheck output
 @post no change in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_requestNULL_to_statusX_logd_requestNULL)
{
    setup_check_status status;
    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        ::LibFred::ContactCheckStatus::ENQUEUE_REQ,
        Optional<unsigned long long>(), Optional<unsigned long long>() );
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status.status_handle, status.status_handle,
        logd_request_id1, Optional<unsigned long long>(), Optional<unsigned long long>());
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @pre existing status handle Y different from status X set to check right now
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_requestNULL_to_statusY_logd_requestNULL)
{
    setup_check_status status1;
    setup_check_status status2;
    Optional<unsigned long long> logd_request_id1 = RandomDataGenerator().xuint();

    setup_create_update_check testcase1(
        status1.status_handle,
        Optional<unsigned long long>(), Optional<unsigned long long>() );
    check(testcase1.data_pre_update_, testcase1.data_post_update_, testcase1.old_status_, testcase1.new_status_, testcase1.old_logd_request_, testcase1.new_logd_request_);

    setup_create_update_update_check testcase2(
        status1.status_handle, status2.status_handle,
        logd_request_id1, Optional<unsigned long long>(), Optional<unsigned long long>());
    check(testcase2.data_post_reset_, testcase2.data_post_update_, testcase2.status2_, testcase2.status3_, testcase2.logd_request2_, testcase2.logd_request3_);
}

/**
 setting nonexistent check handle and existing status values and executing operation
 @pre nonexistent check handle
 @pre existing status handle
 @post ExceptionUnknownCheckHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_handle)
{
    setup_nonexistent_check_handle handle;
    setup_check_status status;

    ::LibFred::UpdateContactCheck dummy(
        uuid::from_string(handle.check_handle),
        status.status_handle);

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
 setting existing check handle and nonexistent status values and executing operation
 @pre existing check handle
 @pre nonexistent status handle
 @post ExceptionUnknownStatusHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_status_handle)
{
    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);
    setup_nonexistent_check_status_handle nonexistent_status;

    ::LibFred::UpdateContactCheck dummy(
        uuid::from_string(check.check_handle_),
        nonexistent_status.status_handle);

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx;
        dummy.exec(ctx);
        ctx.commit_transaction();
    } catch(const ::LibFred::ExceptionUnknownCheckStatusHandle& exp) {
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
