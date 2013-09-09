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

struct setup_get_registrar_handle
{
    std::string registrar_handle;

    setup_get_registrar_handle(Fred::OperationContext& _ctx) {
        registrar_handle = static_cast<std::string>(
            _ctx.get_conn().exec("SELECT handle FROM registrar LIMIT 1;")[0][0] );

        BOOST_REQUIRE(registrar_handle.empty() != true);
    }
};

struct setup_contact : public setup_get_registrar_handle {
    std::string contact_handle;

    setup_contact(Fred::OperationContext& _ctx)
        : setup_get_registrar_handle(_ctx)
    {
        contact_handle = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6);
        Fred::CreateContact create(contact_handle, registrar_handle);
        create.exec(_ctx);
    }
};

struct setup_nonexistent_contact_handle {
    std::string contact_handle;

    setup_nonexistent_contact_handle(Fred::OperationContext& _ctx) {
        Database::Result res;
        do {
            contact_handle = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(10);
            res = _ctx.get_conn().exec(
                "SELECT name "
                "   FROM object_registry "
                "   WHERE name='"+contact_handle+"'"
                "       AND type=1;" );
        } while(res.size() != 0);
    }
};

struct setup_testsuite {
    long testsuite_id;
    std::string testsuite_name;
    std::string testsuite_description;

    setup_testsuite(Fred::OperationContext& _ctx) {
        testsuite_name = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6) + "_TESTSUITE_NAME";
        testsuite_description = testsuite_name + "_DESCRIPTION abrakadabra";
        testsuite_id = static_cast<long>(
            _ctx.get_conn().exec(
                "INSERT INTO enum_contact_testsuite "
                "   (name, description)"
                "   VALUES ('"+testsuite_name+"', '"+testsuite_description+"')"
                "   RETURNING id;"
            )[0][0]);
    }
};

struct setup_nonexistent_testsuite_name {
    std::string testsuite_name;

    setup_nonexistent_testsuite_name(Fred::OperationContext& _ctx) {
        Database::Result res;
        do {
            testsuite_name = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(10) + "_TESTSUITE_NAME";
            res = _ctx.get_conn().exec(
                "SELECT name FROM enum_contact_testsuite WHERE name='"+testsuite_name+"';" );
        } while(res.size() != 0);
    }
};

struct setup_logd_request_id {
    long long logd_request_id;

    setup_logd_request_id() {
        logd_request_id = RandomDataGenerator().xuint();
    }
};

/**
 executing CreateContactCheck with only mandatory setup
 @pre valid contact handle
 @pre valid testsuite name
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_mandatory_setup, fixture_has_ctx)
{
    setup_contact contact(ctx);
    setup_testsuite testsuite(ctx);

    Fred::CreateContactCheck create_check(contact.contact_handle, testsuite.testsuite_name);
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
        result_data.testsuite_name == testsuite.testsuite_name,
        "testsuite name differs in CreateContactCheck ("+ testsuite.testsuite_name +") & InfoContactCheck ("+ testsuite.testsuite_name +")"
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

/**
 executing CreateContactCheck with full mandatory + optional setup
 @pre valid contact handle
 @pre valid testsuite name
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_optional_setup, fixture_has_ctx)
{
    setup_contact contact(ctx);
    setup_testsuite testsuite(ctx);
    setup_logd_request_id logd_request;

    Fred::CreateContactCheck create_check(contact.contact_handle, testsuite.testsuite_name, logd_request.logd_request_id);
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
        static_cast<long long>(result_data.check_state_history.begin()->logd_request_id) == logd_request.logd_request_id,
        std::string("logd_request_id differs in CreateContactCheck (")+ boost::lexical_cast<std::string>(logd_request.logd_request_id) +") & InfoContactCheck ("+ result_data.check_state_history.begin()->logd_request_id.print_quoted() +")"
    );

    // testsuite_id
    BOOST_CHECK_MESSAGE(
        result_data.testsuite_name == testsuite.testsuite_name,
        "testsuite name differs in CreateContactCheck ("+ testsuite.testsuite_name +") & InfoContactCheck ("+ testsuite.testsuite_name +")"
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

/**
 setting nonexistent contact handle value and executing operation
 @pre nonexistent contact handle
 @pre valid testsuite name
 @post Fred::CreateContactCheck::ExceptionUnknownContactHandle
*/
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_contact_handle, fixture_has_ctx)
{
    setup_nonexistent_contact_handle contact(ctx);
    setup_testsuite testsuite(ctx);

    Fred::CreateContactCheck create_check(contact.contact_handle, testsuite.testsuite_name);
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

/**
 setting nonexistent testsuite name value and executing operation
 @pre valid contact handle
 @pre nonexistent testsuite name
 @post Fred::CreateContactCheck::ExceptionUnknownTestsuiteName
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_testsuite_name, fixture_has_ctx)
{
    setup_contact contact(ctx);
    setup_nonexistent_testsuite_name testsuite(ctx);

    Fred::CreateContactCheck create_check(contact.contact_handle, testsuite.testsuite_name);
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
