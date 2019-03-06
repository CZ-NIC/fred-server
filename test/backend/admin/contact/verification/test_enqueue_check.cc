/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
 *  integration tests for admin/contact/verification/enqueue_check.cc
 */

#include "src/backend/admin/contact/verification/enqueue_check.hh"

#include <boost/test/unit_test.hpp>


#include "test/backend/admin/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestEnqueueCheck, Test::instantiate_db_template)

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
    Test::contact contact;
    unsigned long long logd_request_id = RandomDataGenerator().xuint();

    ::LibFred::OperationContextCreator ctx;
    std::string check_handle = Fred::Backend::Admin::Contact::Verification::enqueue_check(
        ctx,
        contact.info_data.id,
        testsuite.testsuite_handle,
        logd_request_id);

    ::LibFred::InfoContactCheckOutput check_info = ::LibFred::InfoContactCheck( uuid::from_string( check_handle) ).exec(ctx);

    BOOST_CHECK_EQUAL(check_info.check_state_history.size(), 1);
    BOOST_CHECK_EQUAL(check_info.testsuite_handle, testsuite.testsuite_handle);
    BOOST_CHECK_EQUAL(
        check_info.check_state_history.begin()->logd_request_id.get_value_or_default(),
        logd_request_id);
    BOOST_CHECK_EQUAL(
        check_info.check_state_history.begin()->status_handle,
        ::LibFred::ContactCheckStatus::ENQUEUE_REQ);

}

/**
testing whether old enqueud checks are correctly invalidated
@pre existing contact with enqueued checks
@post correct data set to previous check
*/
BOOST_AUTO_TEST_CASE(test_Invalidating_old_checks)
{
    setup_testsuite testsuite;
    Test::contact contact;

    ::LibFred::OperationContextCreator ctx;

    std::string invalidated_check_handle_1 = ::LibFred::CreateContactCheck(
        contact.info_data.id,
        testsuite.testsuite_handle
    ).exec(ctx);

    ::LibFred::UpdateContactCheck(
        uuid::from_string(invalidated_check_handle_1),
        ::LibFred::ContactCheckStatus::ENQUEUED
    ).exec(ctx);

    std::string invalidated_check_handle_2 = ::LibFred::CreateContactCheck(
        contact.info_data.id,
        testsuite.testsuite_handle
    ).exec(ctx);

    ::LibFred::UpdateContactCheck(
        uuid::from_string(invalidated_check_handle_2),
        ::LibFred::ContactCheckStatus::ENQUEUED
    ).exec(ctx);

    std::string new_check_handle(
        Fred::Backend::Admin::Contact::Verification::request_check_enqueueing(
            ctx,
            contact.info_data.id,
            testsuite.testsuite_handle,
            RandomDataGenerator().xuint()
    ));

    Fred::Backend::Admin::Contact::Verification::confirm_check_enqueueing(
        ctx,
        uuid::from_string(new_check_handle));

    ::LibFred::InfoContactCheckOutput invalidated_check_info_1 =
        ::LibFred::InfoContactCheck(
            uuid::from_string( invalidated_check_handle_1)
        ).exec(ctx);

    ::LibFred::InfoContactCheckOutput invalidated_check_info_2 =
        ::LibFred::InfoContactCheck(
            uuid::from_string( invalidated_check_handle_2)
        ).exec(ctx);

    ::LibFred::InfoContactCheckOutput new_check_info =
        ::LibFred::InfoContactCheck(
            uuid::from_string( new_check_handle)
        ).exec(ctx);

    BOOST_CHECK_EQUAL(
        invalidated_check_info_1.check_state_history.rbegin()->status_handle,
        ::LibFred::ContactCheckStatus::INVALIDATED);

    BOOST_CHECK_EQUAL(
        invalidated_check_info_2.check_state_history.rbegin()->status_handle,
        ::LibFred::ContactCheckStatus::INVALIDATED);

    BOOST_CHECK_EQUAL(
        invalidated_check_info_1.check_state_history.rbegin()->logd_request_id,
        new_check_info.check_state_history.rbegin()->logd_request_id);

    BOOST_CHECK_EQUAL(
        invalidated_check_info_2.check_state_history.rbegin()->logd_request_id,
        new_check_info.check_state_history.rbegin()->logd_request_id);

}

/**
testing correct exception throw after unknown contact id is passed
@pre existing testsuite_handle
@pre nonexistent contact id
@post correct exception thrown
*/
BOOST_AUTO_TEST_CASE(test_ExceptionUnknownContactId)
{
    ::LibFred::OperationContextCreator ctx;

    bool correct_exception_thrown = false;

    try {
        Fred::Backend::Admin::Contact::Verification::enqueue_check(
            ctx,
            Test::get_nonexistent_object_id(ctx),
            setup_testsuite().testsuite_handle,
            Optional<unsigned long long>()
        );
    } catch (const Fred::Backend::Admin::Contact::Verification::ExceptionUnknownContactId& ) {
        correct_exception_thrown = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception thrown");
    }

    BOOST_CHECK(correct_exception_thrown);
}

/**
testing correct exception throw after unknown testsuite handle is passed
@pre existing contact id
@pre nonexistent testsuite_handle
@post correct exception thrown
*/
BOOST_AUTO_TEST_CASE(test_ExceptionUnknownTestsuiteHandle)
{
    ::LibFred::OperationContextCreator ctx;

    bool correct_exception_thrown = false;

    try {
        Fred::Backend::Admin::Contact::Verification::enqueue_check(
            ctx,
            Test::contact().info_data.id,
            setup_nonexistent_testsuite_handle().testsuite_handle,
            Optional<unsigned long long>()
        );
    } catch (const Fred::Backend::Admin::Contact::Verification::ExceptionUnknownTestsuiteHandle& ) {
        correct_exception_thrown = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception thrown");
    }

    BOOST_CHECK(correct_exception_thrown);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
