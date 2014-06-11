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
 *  integration tests for CreateContactCheck operation
 */

#include <vector>
#include <utility>
#include <string>

#include "src/fredlib/contact/verification/create_check.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "util/random_data_generator.h"

#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestCreateContactCheck_integ, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification-create_check_integ";

/**
 executing CreateContactCheck with only mandatory setup
 @pre valid contact handle
 @pre valid testsuite handle including one existing test
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_AUTO_TEST_CASE(test_Exec_mandatory_setup)
{
    setup_contact contact;
    setup_testsuite testsuite;

    Fred::CreateContactCheck create_check(contact.contact_id_, testsuite.testsuite_handle);
    std::string handle;
    std::string timezone = "UTC";

    try {
        Fred::OperationContext ctx1;
        handle = create_check.exec(ctx1);
        ctx1.commit_transaction();
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    Fred::InfoContactCheck info_check( uuid::from_string(handle) );
    Fred::InfoContactCheckOutput result_data;


    try {
        Fred::OperationContext ctx2;
        result_data = info_check.exec(ctx2, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    // create_time is reasonable

    ptime now = second_clock::universal_time();
    ptime create_time_min = now - minutes(1);
    ptime create_time_max = now + minutes(1);

    BOOST_CHECK_MESSAGE(
        result_data.local_create_time > create_time_min,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_CHECK_MESSAGE(
        result_data.local_create_time < create_time_max,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    // contact_history_id is correct regarding the create_time
    Fred::OperationContext ctx3;
    Database::Result contact_history_validity_interval = ctx3.get_conn().exec_params(
        "SELECT "
        "   valid_from AT TIME ZONE 'utc' AT TIME ZONE $1::text AS valid_from_, "
        "   valid_to  AT TIME ZONE 'utc' AT TIME ZONE $1::text AS valid_to_ "
        "   FROM history "
        "   WHERE id=$2::int; ",
        Database::query_param_list
            (timezone)
            (result_data.contact_history_id)
    );

    BOOST_REQUIRE_MESSAGE(
        contact_history_validity_interval.size() == 1,
        "failed to get appropriate history id \n"
        + result_data.to_string() );

    boost::posix_time::ptime from_time =
        boost::posix_time::time_from_string(
            static_cast<std::string> (contact_history_validity_interval[0]["valid_from_"] ));

    boost::posix_time::ptime to_time;
    bool to_is_null = contact_history_validity_interval[0]["valid_to_"].isnull();
    if(to_is_null == false) {
        to_time =
            boost::posix_time::time_from_string(
                static_cast<std::string>(
                    contact_history_validity_interval[0]["valid_to_"] ));
    }

    /* WARNING - "...OR EQUAL"
     * part of comparison is present because the contact is created in the same transaction
     * and therefore has the same value of NOW() time function in postgres
     */
    BOOST_CHECK_MESSAGE(
        from_time <= result_data.local_create_time &&
        ( to_time >= result_data.local_create_time || to_is_null),
        "invalid history_id - not valid at check create_time." +
        result_data.to_string() +
        "expected interval is: " + boost::posix_time::to_simple_string(from_time)
        + " - " + boost::posix_time::to_simple_string(to_time) + "\n"
    );

    // logd_request_id

    BOOST_CHECK_MESSAGE(
        result_data.check_state_history.begin()->logd_request_id.isnull() == true,
        "logd_request_id should be NULL"
    );

    // testsuite_id
    BOOST_CHECK_MESSAGE(
        result_data.testsuite_handle == testsuite.testsuite_handle,
        "testsuite handle differs in CreateContactCheck ("+ testsuite.testsuite_handle +") & InfoContactCheck ("+ testsuite.testsuite_handle +")"
    );

    // create_time
    BOOST_CHECK_MESSAGE(
         result_data.local_create_time == result_data.check_state_history.begin()->local_update_time,
         "create_time ("+ boost::posix_time::to_simple_string(result_data.local_create_time) +") differs from first update_time ("+ boost::posix_time::to_simple_string(result_data.check_state_history.begin()->local_update_time) +")"
    );

    // status handle is 'ENQUEUE_REQ'
    BOOST_CHECK_MESSAGE(
         result_data.check_state_history.begin()->status_handle == Fred::ContactCheckStatus::ENQUEUE_REQ,
         "status of create check ("+ result_data.check_state_history.begin()->status_handle +") should be " + Fred::ContactCheckStatus::ENQUEUE_REQ
    );
}

/**
 executing CreateContactCheck with full mandatory + optional setup
 @pre valid contact handle
 @pre valid testsuite handle including one existing test
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_AUTO_TEST_CASE(test_Exec_optional_setup)
{
    setup_contact contact;
    setup_testsuite testsuite;
    setup_logd_request_id logd_request;

    Fred::CreateContactCheck create_check(contact.contact_id_, testsuite.testsuite_handle, logd_request.logd_request_id);
    std::string handle;
    std::string timezone = "UTC";

    try {
        Fred::OperationContext ctx1;
        handle = create_check.exec(ctx1);
        ctx1.commit_transaction();
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    Fred::InfoContactCheck info_check(uuid::from_string(handle));
    Fred::InfoContactCheckOutput result_data;

    try {
        Fred::OperationContext ctx2;
        result_data = info_check.exec(ctx2, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("exception (3):") + exp.what());
    }

    // create_time is reasonable

    ptime now = second_clock::universal_time();
    ptime create_time_min = now - minutes(1);
    ptime create_time_max = now + minutes(1);

    BOOST_CHECK_MESSAGE(
        result_data.local_create_time > create_time_min,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_CHECK_MESSAGE(
        result_data.local_create_time < create_time_max,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    // contact_history_id is correct regarding the create_time

    Fred::OperationContext ctx3;
    Database::Result contact_history_validity_interval = ctx3.get_conn().exec_params(
        "SELECT "
        "   valid_from AT TIME ZONE 'utc' AT TIME ZONE $1::varchar AS valid_from_, "
        "   valid_to  AT TIME ZONE 'utc' AT TIME ZONE $1::varchar AS valid_to_ "
        "   FROM history "
        "   WHERE id=$2::int; ",
        Database::query_param_list
            (timezone)
            (result_data.contact_history_id)
    );

    BOOST_REQUIRE_MESSAGE(
        contact_history_validity_interval.size() == 1,
        "failed to get appropriate history id \n"
        + result_data.to_string() );

    boost::posix_time::ptime from_time =
        boost::posix_time::time_from_string(
            static_cast<std::string> (contact_history_validity_interval[0]["valid_from_"] ));

    boost::posix_time::ptime to_time;
    bool to_is_null = contact_history_validity_interval[0]["valid_to_"].isnull();
    if(to_is_null == false)
        to_time =
            boost::posix_time::time_from_string(
                static_cast<std::string>(
                    contact_history_validity_interval[0]["valid_to_"] ));

    /* WARNING - "...OR EQUAL"
         * part of comparison is present because the contact is created in the same transaction
         * and therefore has the same value of NOW() time function in postgres
         */
    BOOST_CHECK_MESSAGE(
        from_time <= result_data.local_create_time &&
        ( to_time >= result_data.local_create_time || to_is_null),
        "invalid history_id - not valid at check create_time." +
        result_data.to_string()
    );

    // logd_request_id

    BOOST_CHECK_MESSAGE(
        result_data.check_state_history.begin()->logd_request_id.get_value_or_default() == logd_request.logd_request_id,
        std::string("logd_request_id differs in CreateContactCheck (")+ boost::lexical_cast<std::string>(logd_request.logd_request_id) +") & InfoContactCheck ("+ result_data.check_state_history.begin()->logd_request_id.print_quoted() +")"
    );

    // testsuite_id
    BOOST_CHECK_MESSAGE(
        result_data.testsuite_handle == testsuite.testsuite_handle,
        "testsuite handle differs in CreateContactCheck ("+ testsuite.testsuite_handle +") & InfoContactCheck ("+ testsuite.testsuite_handle +")"
    );

    // create_time
    BOOST_CHECK_MESSAGE(
         result_data.local_create_time == result_data.check_state_history.begin()->local_update_time,
         "create_time ("+ boost::posix_time::to_simple_string(result_data.local_create_time) +") differs from first update_time ("+ boost::posix_time::to_simple_string(result_data.check_state_history.begin()->local_update_time) +")"
    );

    // status handle is 'ENQUEUE_REQ'
    BOOST_CHECK_MESSAGE(
         result_data.check_state_history.begin()->status_handle == Fred::ContactCheckStatus::ENQUEUE_REQ,
         "status of create check ("+ result_data.check_state_history.begin()->status_handle +")should be " + Fred::ContactCheckStatus::ENQUEUE_REQ
    );
}

/**
 setting nonexistent contact handle value and executing operation
 @pre nonexistent contact handle
 @pre valid testsuite handle
 @post Fred::CreateContactCheck::ExceptionUnknownContactHandle
*/
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_contact_id)
{
    setup_nonexistent_contact_id contact;
    setup_testsuite testsuite;

    Fred::CreateContactCheck create_check(contact.contact_id_, testsuite.testsuite_handle);
    std::string handle;

    bool caught_the_right_exception = false;
    try {
        Fred::OperationContext ctx;
        handle = create_check.exec(ctx);
        ctx.commit_transaction();
    } catch(const Fred::ExceptionUnknownContactId& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting nonexistent testsuite handle value and executing operation
 @pre valid contact handle
 @pre nonexistent testsuite handle
 @post Fred::CreateContactCheck::ExceptionUnknownTestsuiteHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_testsuite_handle)
{
    setup_contact contact;
    setup_nonexistent_testsuite_handle testsuite;

    Fred::CreateContactCheck create_check(contact.contact_id_, testsuite.testsuite_handle);
    std::string handle;

    bool caught_the_right_exception = false;
    try {
        Fred::OperationContext ctx;
        handle = create_check.exec(ctx);
        ctx.commit_transaction();
    } catch(const Fred::ExceptionUnknownTestsuiteHandle& exp) {
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
