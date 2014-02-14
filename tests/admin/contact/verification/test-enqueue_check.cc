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
 *  integration tests for admin/contact/verification/enqueue_check.cc
 */

#include "src/admin/contact/verification/enqueue_check.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"

#include <boost/test/unit_test.hpp>


#include "tests/admin/contact/verification/setup_utils.h"
#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestEnqueueCheck, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification_integration-enqueue_check";

/**
testing validity of newly created check
@pre existing contact
@pre existing testsuite
@post correct data set to check
*/
BOOST_AUTO_TEST_CASE(test_Enqueued_check_data)
{
    setup_testsuite testsuite;
    setup_contact contact;
    unsigned long long logd_request_id = RandomDataGenerator().xuint();

    Fred::OperationContext ctx;
    std::string check_handle = Admin::enqueue_check(
        ctx,
        contact.contact_id_,
        testsuite.testsuite_handle,
        logd_request_id);

    Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(check_handle).exec(ctx);

    BOOST_CHECK_EQUAL(check_info.check_state_history.size(), 1);
    BOOST_CHECK_EQUAL(check_info.testsuite_handle, testsuite.testsuite_handle);
    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>(
            check_info.check_state_history.begin()->logd_request_id),
        logd_request_id);
    BOOST_CHECK_EQUAL(
        check_info.check_state_history.begin()->status_handle,
        Fred::ContactCheckStatus::ENQUEUED);

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
