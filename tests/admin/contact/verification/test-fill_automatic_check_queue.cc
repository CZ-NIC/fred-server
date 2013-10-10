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
 *  integration tests for admin/contact/verification/fill_automatic_check_queue.cc
 */

#include "admin/contact/verification/fill_automatic_check_queue.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/verification/enum_testsuite_name.h"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "tests/admin/contact/verification/setup_utils.h"
#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

namespace Test = Fred::ContactTestStatus;
namespace Check = Fred::ContactCheckStatus;

void clean_queue() {
    Fred::OperationContext ctx;
    ctx.get_conn().exec_params(
        "DELETE FROM contact_test_result "
        "   WHERE enum_contact_test_status_id = "
        "       (SELECT id FROM enum_contact_test_status WHERE name=$1::varchar);",
        Database::query_param_list(Test::ENQUEUED) );

    ctx.get_conn().exec_params(
        "DELETE FROM contact_check "
        "   WHERE enum_contact_check_status_id = "
        "       (SELECT id FROM enum_contact_check_status WHERE name=$1::varchar);",
        Database::query_param_list(Check::ENQUEUED) );

    ctx.commit_transaction();
}

int get_queue_length() {
    Fred::OperationContext ctx;
    Database::Result res = ctx.get_conn().exec_params(
        "SELECT COUNT(id) AS count_ "
        "   FROM contact_check "
        "   WHERE enum_contact_check_status_id = "
        "       (SELECT id FROM enum_contact_check_status WHERE name=$1::varchar);",
        Database::query_param_list(Test::ENQUEUED) );

    if(res.size() != 1) {
        throw std::runtime_error("invalid query result");
    }
    return static_cast<int>(res[0]["count_"]);

}

BOOST_AUTO_TEST_SUITE(TestFillAutomaticQueue)

const std::string server_name = "test-contact_verification_integration-fill_automatic_check_queue";

/**
parameter of fill_automatic_check_queue must correctly affect number of newly enqueued checks
@pre clean queue
@post correct number of enqueued checks
 */
BOOST_AUTO_TEST_CASE(test_Max_queue_lenght_parameter)
{
    clean_queue();

    Fred::OperationContext ctx;
    setup_contact contact(ctx);
    setup_testdef test(ctx);
    setup_testdef_in_testsuite (ctx, test.testdef_name_, Fred::TestsuiteName::AUTOMATIC);
    ctx.commit_transaction();

    Admin::fill_automatic_check_queue(10);
    BOOST_CHECK_EQUAL(get_queue_length(), 10);

    Admin::fill_automatic_check_queue(30);
    BOOST_CHECK_EQUAL(get_queue_length(), 30);

    Admin::fill_automatic_check_queue(20);
    BOOST_CHECK_EQUAL(get_queue_length(), 30);
}

/**
 when queue is full new checks mustn't be created
 @pre full queue (relative to max_queue_lenght value)
 @post no new checks enqueued
 */
BOOST_AUTO_TEST_CASE(test_Try_fill_full_queue)
{
    clean_queue();

    Fred::OperationContext ctx;
    setup_contact contact(ctx);
    setup_testdef test(ctx);
    setup_testdef_in_testsuite (ctx, test.testdef_name_, Fred::TestsuiteName::AUTOMATIC);
    ctx.commit_transaction();

    Admin::fill_automatic_check_queue(101);

    Admin::fill_automatic_check_queue(101);
    BOOST_CHECK_EQUAL(get_queue_length(), 101);

    Admin::fill_automatic_check_queue(101);
    BOOST_CHECK_EQUAL(get_queue_length(), 101);
}

BOOST_AUTO_TEST_SUITE_END();
