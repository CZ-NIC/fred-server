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
 *  integration tests for admin/contact/verification/resolve_check.cc
 */

#include "src/admin/contact/verification/resolve_check.h"
#include "src/fredlib/contact/verification/info_check.h"

#include <boost/test/unit_test.hpp>


#include "tests/admin/contact/verification/setup_utils.h"
#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestResolveCheck, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification_integration-resolve_check";

/**
testing check gets correct status
@pre existing check
@pre existing status passed to resolve_check
@post correct status set to check
*/
BOOST_AUTO_TEST_CASE(test_Resolved_status)
{
    setup_testsuite testsuite;
    setup_check check(testsuite.testsuite_handle);
    setup_check_status status;

    Fred::OperationContext ctx;

    Admin::resolve_check(
        check.check_handle_,
        status.status_handle,
        Optional<unsigned long long>()
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        status.status_handle,
        Fred::InfoContactCheck(
            check.check_handle_
        ).exec(ctx)
            .check_state_history
                .rbegin()
                    ->status_handle
    );
}

/**
testing correct logd_request_id
@pre existing check
@pre logd_request_id to resolve_check
@post correct logd_request_id set to check
*/
BOOST_AUTO_TEST_CASE(test_Resolved_logd_request_id)
{
    setup_testsuite testsuite;
    setup_check check(testsuite.testsuite_handle);
    setup_check_status status;

    Fred::OperationContext ctx;

    Admin::resolve_check(
        check.check_handle_,
        status.status_handle,
        36478
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        36478,
        static_cast<long>(
            Fred::InfoContactCheck(
                check.check_handle_
            ).exec(ctx)
                .check_state_history
                    .rbegin()
                        ->logd_request_id
        )
    );
}



BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
