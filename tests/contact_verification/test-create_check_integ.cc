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

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "random_data_generator.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"

BOOST_AUTO_TEST_SUITE(TestCreateContactCheck_integ)

const std::string server_name = "test-contact_verification-create_check_integ";

struct fixture_has_ctx {
    Fred::OperationContext ctx;
};

struct fixture_get_registrar_handle : public virtual fixture_has_ctx
{
    std::string registrar_handle;

    fixture_get_registrar_handle() {
        registrar_handle = static_cast<std::string>(
            ctx.get_conn().exec("SELECT handle FROM registrar LIMIT 1;")[0][0] );

        BOOST_REQUIRE(registrar_handle.empty() != true);
    }
};

struct fixture_create_contact : public fixture_get_registrar_handle, public virtual fixture_has_ctx {
    std::string contact_handle;

    fixture_create_contact() {
        contact_handle = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6);
        Fred::CreateContact create(contact_handle, registrar_handle);
        create.exec(ctx);
        ctx.commit_transaction();
    }
};

struct fixture_create_nonexistent_contact_handle : public virtual fixture_has_ctx {
    std::string contact_handle;

    fixture_create_nonexistent_contact_handle() {
        Database::Result res;
        do {
            contact_handle = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(10);
            res = ctx.get_conn().exec(
                "SELECT name "
                "   FROM object_registry "
                "   WHERE name='"+contact_handle+"'"
                "       AND type=1;" );
        } while(res.size() != 0);
    }
};

struct fixture_create_testsuite : public virtual fixture_has_ctx {
    long testsuite_id;
    std::string testsuite_name;
    std::string testsuite_description;

    fixture_create_testsuite() {
        testsuite_name = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6) + "_TESTSUITE_NAME";
        testsuite_description = testsuite_name + "_DESCRIPTION abrakadabra";
        testsuite_id = static_cast<long>(
            ctx.get_conn().exec(
                "INSERT INTO enum_contact_testsuite "
                "   (name, description)"
                "   VALUES ('"+testsuite_name+"', '"+testsuite_description+"')"
                "   RETURNING id;"
            )[0][0]);
        ctx.commit_transaction();
    }
};

struct fixture_create_nonexistent_testsuite_name : public virtual fixture_has_ctx {
    std::string testsuite_name;

    fixture_create_nonexistent_testsuite_name() {
        Database::Result res;
        do {
            testsuite_name = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(10) + "_TESTSUITE_NAME";
            res = ctx.get_conn().exec(
                "SELECT name FROM enum_contact_testsuite WHERE name='"+testsuite_name+"';" );
        } while(res.size() != 0);
    }
};

struct fixture_create_logd_request_id {
    long long logd_request_id;

    fixture_create_logd_request_id() {
        logd_request_id = RandomDataGenerator().xuint();
    }
};

struct fixture_mandatory_input : public fixture_create_contact, public fixture_create_testsuite {};

/**
 executing CreateContactCheck with only mandatory setup
 @pre valid contact handle
 @pre valid testsuite name
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_mandatory_setup, fixture_mandatory_input)
{
    Fred::CreateContactCheck create_check(contact_handle, testsuite_name);
    std::string handle;
    std::string timezone = "UTC";

    try {
        handle = create_check.exec(ctx);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to create check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to create check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("failed to create check (3):") + exp.what());
    }

    Fred::InfoContactCheck info_check(handle);
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

    Database::Result contact_history_validity_interval = ctx.get_conn().exec_params(
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

    BOOST_CHECK_MESSAGE(
        from_time < result_data.local_create_time &&
        ( to_time > result_data.local_create_time || to_is_null),
        "invalid history_id - not valid at check create_time." +
        result_data.to_string()
    );

    // logd_request_id

    BOOST_CHECK_MESSAGE(
        result_data.check_state_history.begin()->logd_request_id.isnull() == true,
        "logd_request_id should be NULL"
    );

    // testsuite_id
    BOOST_CHECK_MESSAGE(
        result_data.testsuite_name == testsuite_name,
        "testsuite name differs in CreateContactCheck ("+ testsuite_name +") & InfoContactCheck ("+ testsuite_name +")"
    );

    // create_time
    BOOST_CHECK_MESSAGE(
         result_data.local_create_time == result_data.check_state_history.begin()->local_update_time,
         "create_time ("+ boost::posix_time::to_simple_string(result_data.local_create_time) +") differs from first update_time ("+ boost::posix_time::to_simple_string(result_data.check_state_history.begin()->local_update_time) +")"
    );

    // status name is 'enqueued'
    BOOST_CHECK_MESSAGE(
         result_data.check_state_history.begin()->status_name == Fred::ContactCheckStatus::ENQUEUED,
         "status of create check ("+ result_data.check_state_history.begin()->status_name +")should be enqueued"
    );
}

struct fixture_all_input : public fixture_mandatory_input, public fixture_create_logd_request_id {};

/**
 executing CreateContactCheck with full mandatory + optional setup
 @pre valid contact handle
 @pre valid testsuite name
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_optional_setup, fixture_all_input)
{
    Fred::CreateContactCheck create_check(contact_handle, testsuite_name, logd_request_id);
    std::string handle;
    std::string timezone = "UTC";

    try {
        handle = create_check.exec(ctx);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to create check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to create check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("failed to create check (3):") + exp.what());
    }

    Fred::InfoContactCheck info_check(handle);
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

    Database::Result contact_history_validity_interval = ctx.get_conn().exec_params(
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

    BOOST_CHECK_MESSAGE(
        from_time < result_data.local_create_time &&
        ( to_time > result_data.local_create_time || to_is_null),
        "invalid history_id - not valid at check create_time." +
        result_data.to_string()
    );

    // logd_request_id

    BOOST_CHECK_MESSAGE(
        static_cast<long long>(result_data.check_state_history.begin()->logd_request_id) == logd_request_id,
        std::string("logd_request_id differs in CreateContactCheck (")+ boost::lexical_cast<std::string>(logd_request_id) +") & InfoContactCheck ("+ result_data.check_state_history.begin()->logd_request_id.print_quoted() +")"
    );

    // testsuite_id
    BOOST_CHECK_MESSAGE(
        result_data.testsuite_name == testsuite_name,
        "testsuite name differs in CreateContactCheck ("+ testsuite_name +") & InfoContactCheck ("+ testsuite_name +")"
    );

    // create_time
    BOOST_CHECK_MESSAGE(
         result_data.local_create_time == result_data.check_state_history.begin()->local_update_time,
         "create_time ("+ boost::posix_time::to_simple_string(result_data.local_create_time) +") differs from first update_time ("+ boost::posix_time::to_simple_string(result_data.check_state_history.begin()->local_update_time) +")"
    );

    // status name is 'enqueued'
    BOOST_CHECK_MESSAGE(
         result_data.check_state_history.begin()->status_name == Fred::ContactCheckStatus::ENQUEUED,
         "status of create check ("+ result_data.check_state_history.begin()->status_name +")should be enqueued"
    );
}

struct fixture_nonexistent_contact_handle_valid_testsuite_name : public fixture_create_nonexistent_contact_handle, fixture_create_testsuite {};
/**
 setting nonexistent contact handle value and executing operation
 @pre nonexistent contact handle
 @pre valid testsuite name
 @post Fred::CreateContactCheck::ExceptionUnknownContactHandle
*/
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_contact_handle, fixture_nonexistent_contact_handle_valid_testsuite_name)
{
    Fred::CreateContactCheck create_check(contact_handle, testsuite_name);
    std::string handle;

    bool caught_the_right_exception = false;
    try {
        handle = create_check.exec(ctx);
    } catch(const Fred::CreateContactCheck::ExceptionUnknownContactHandle& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

struct fixture_valid_contact_handle_nonexistent_testsuite_name : public fixture_create_contact, fixture_create_nonexistent_testsuite_name {};
/**
 setting nonexistent testsuite name value and executing operation
 @pre valid contact handle
 @pre nonexistent testsuite name
 @post Fred::CreateContactCheck::ExceptionUnknownTestsuiteName
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_testsuite_name, fixture_valid_contact_handle_nonexistent_testsuite_name)
{
    Fred::CreateContactCheck create_check(contact_handle, testsuite_name);
    std::string handle;

    bool caught_the_right_exception = false;
    try {
        handle = create_check.exec(ctx);
    } catch(const Fred::CreateContactCheck::ExceptionUnknownTestsuiteName& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

BOOST_AUTO_TEST_SUITE_END();
