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
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"

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

static std::vector<unsigned long long> get_related_object_state_requests(
    Fred::OperationContext& ctx,
    const std::string& contact_check_handle
) {
    Database::Result requests_res = ctx.get_conn().exec_params(
        "SELECT map_.object_state_request_id AS id_ "
        "   FROM contact_check_object_state_request_map AS map_ "
        "       JOIN contact_check AS c_ch ON map_.contact_check_id = c_ch.id "
        "   WHERE c_ch.handle = $1::uuid ",
        Database::query_param_list
            (contact_check_handle)
    );

    std::vector<unsigned long long> result;

    for(Database::Result::Iterator it = requests_res.begin();
        it != requests_res.end();
        ++it
    ) {
        result.push_back(
            static_cast<unsigned long long>((*it)["id_"])
        );
    }

    return result;
}

/**
testing if manual check postprocessing creates related object_state_request
@pre existing check with manual testsuite
@post new record for this check in contact_check_object_state_request_map
*/
BOOST_AUTO_TEST_CASE(test_Resolving_manual_suite_postprocessing)
{
    setup_check fail_check(Fred::TestsuiteHandle::MANUAL);
    setup_check ok_check(Fred::TestsuiteHandle::MANUAL);

    Fred::OperationContext ctx;

    Admin::resolve_check(
        fail_check.check_handle_,
        Fred::ContactCheckStatus::FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    Admin::resolve_check(
        ok_check.check_handle_,
        Fred::ContactCheckStatus::OK,
        Optional<unsigned long long>()
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        get_related_object_state_requests(ctx, fail_check.check_handle_).size(),
        0 );

    BOOST_CHECK_EQUAL(
        get_related_object_state_requests(ctx, ok_check.check_handle_).size(),
        1 );
}

/**
testing if manual check postprocessing creates related object_state_request
@pre existing check with manual testsuite
@post new record for this check in contact_check_object_state_request_map
*/
BOOST_AUTO_TEST_CASE(test_Resolving_automatic_suite_postprocessing)
{
    setup_check fail_check(Fred::TestsuiteHandle::AUTOMATIC);
    setup_check ok_check(Fred::TestsuiteHandle::AUTOMATIC);

    Fred::OperationContext ctx;

    Admin::resolve_check(
        fail_check.check_handle_,
        Fred::ContactCheckStatus::FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    Admin::resolve_check(
        ok_check.check_handle_,
        Fred::ContactCheckStatus::OK,
        Optional<unsigned long long>()
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        get_related_object_state_requests(ctx, fail_check.check_handle_).size(),
        0 );

    BOOST_CHECK_EQUAL(
        get_related_object_state_requests(ctx, ok_check.check_handle_).size(),
        0 );
}



BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
