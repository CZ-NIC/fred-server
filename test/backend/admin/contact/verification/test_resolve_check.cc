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

#include "src/backend/admin/contact/verification/resolve_check.hh"
#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "libfred/registrable_object/contact/verification/exceptions.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/list_checks.hh"
#include "libfred/registrable_object/contact/verification/list_enum_objects.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"
#include "libfred/registrable_object/contact/verification/update_test.hh"
#include "test/backend/admin/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestResolveCheck, Test::instantiate_db_template)

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

    ::LibFred::OperationContextCreator ctx;

    ::LibFred::UpdateContactCheck(
        uuid::from_string( check.check_handle_ ),
        ::LibFred::ContactCheckStatus::AUTO_OK,
        Optional<unsigned long long>()
    ).exec(ctx);

    Fred::Backend::Admin::Contact::Verification::resolve_check(
        uuid::from_string( check.check_handle_ ),
        ::LibFred::ContactCheckStatus::OK,
        Optional<unsigned long long>()
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        ::LibFred::ContactCheckStatus::OK,
        ::LibFred::InfoContactCheck(
            uuid::from_string( check.check_handle_ )
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

    ::LibFred::OperationContextCreator ctx;

    ::LibFred::UpdateContactCheck(
        uuid::from_string( check.check_handle_ ),
        ::LibFred::ContactCheckStatus::AUTO_OK,
        36477
    ).exec(ctx);

    Fred::Backend::Admin::Contact::Verification::resolve_check(
        uuid::from_string( check.check_handle_ ),
        ::LibFred::ContactCheckStatus::OK,
        36478
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        36478,
        ::LibFred::InfoContactCheck(
            uuid::from_string( check.check_handle_ )
        ).exec(ctx)
            .check_state_history
                .rbegin()
                    ->logd_request_id.get_value_or_default()

    );
}

static std::vector<unsigned long long> get_related_object_state_requests(
    ::LibFred::OperationContext& ctx,
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
    setup_check fail_check(::LibFred::TestsuiteHandle::MANUAL);
    setup_check ok_check(::LibFred::TestsuiteHandle::MANUAL);

    ::LibFred::OperationContextCreator ctx;


    ::LibFred::UpdateContactCheck(
        uuid::from_string( fail_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::AUTO_FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    Fred::Backend::Admin::Contact::Verification::resolve_check(
        uuid::from_string( fail_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        get_related_object_state_requests(ctx, fail_check.check_handle_).size(),
        1 );


    ::LibFred::UpdateContactCheck(
        uuid::from_string( ok_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::AUTO_OK,
        Optional<unsigned long long>()
    ).exec(ctx);

    Fred::Backend::Admin::Contact::Verification::resolve_check(
        uuid::from_string( ok_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::OK,
        Optional<unsigned long long>()
    ).exec(ctx);

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
    setup_check fail_check(::LibFred::TestsuiteHandle::AUTOMATIC);
    setup_check ok_check(::LibFred::TestsuiteHandle::AUTOMATIC);

    ::LibFred::OperationContextCreator ctx;

    ::LibFred::UpdateContactCheck(
        uuid::from_string( fail_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::AUTO_FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    Fred::Backend::Admin::Contact::Verification::resolve_check(
        uuid::from_string( fail_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        get_related_object_state_requests(ctx, fail_check.check_handle_).size(),
        0 );


    ::LibFred::UpdateContactCheck(
        uuid::from_string( ok_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::AUTO_OK,
        Optional<unsigned long long>()
    ).exec(ctx);

    Fred::Backend::Admin::Contact::Verification::resolve_check(
        uuid::from_string( ok_check.check_handle_ ),
        ::LibFred::ContactCheckStatus::OK,
        Optional<unsigned long long>()
    ).exec(ctx);

    BOOST_CHECK_EQUAL(
        get_related_object_state_requests(ctx, ok_check.check_handle_).size(),
        1 );
}



BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
