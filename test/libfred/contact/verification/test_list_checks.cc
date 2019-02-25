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
 *  integration tests for ListContactChecks operation
 */


#include "libfred/registrable_object/contact/verification/list_checks.hh"
#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "test/libfred/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestListContactChecks_integ, Test::instantiate_db_template)

const std::string server_name = "test-contact_verification-list_checks_integ";

/**
 testing number of returned objects with different numbers of pre-created checks and different given value of parameter
 @pre existing check handles
 @post correct values present in exec() output
 */
BOOST_AUTO_TEST_CASE(test_Exec_max_count)
{
    setup_testsuite testsuite;
    for(int i=0; i<20; ++i) {
        setup_check(testsuite.testsuite_handle);
    }

    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK_EQUAL(::LibFred::ListContactChecks().set_max_item_count(10).exec(ctx).size(), 10);
}

/**
 testing filters on testsuite
 @pre existing check handles related to various testsuites
 @post correct values present in exec() output
 */
BOOST_AUTO_TEST_CASE(test_Exec_testsuite_filter)
{
    setup_testsuite testsuite1;
    setup_testsuite testsuite2;
    setup_testsuite testsuite3;

    std::map<std::string, bool> handles_to_be_listed;

    for(int i=0; i<10; ++i) {
        setup_check(testsuite1.testsuite_handle);
        handles_to_be_listed[setup_check(testsuite2.testsuite_handle).check_handle_] = false;
        setup_check(testsuite3.testsuite_handle);
    }

    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::ListChecksItem> list = ::LibFred::ListContactChecks()
        .set_max_item_count(100)
        .set_testsuite_handle(testsuite2.testsuite_handle)
        .exec(ctx);

    for(std::vector<::LibFred::ListChecksItem>::const_iterator it = list.begin();
        it != list.end();
        ++it
    ) {
        if(handles_to_be_listed.find(it->check_handle) != handles_to_be_listed.end()) {
            handles_to_be_listed.find(it->check_handle)->second = true;
        } else {
            BOOST_FAIL("Invalid check listed.");
            break;
        }
    }

    for(std::map<std::string, bool>::const_iterator it = handles_to_be_listed.begin();
        it != handles_to_be_listed.end();
        ++it
    ) {
        if(it->second == false) {
            BOOST_FAIL("Check not listed.");
        }
    }
}

/**
 testing filters on contact
 @pre existing check handles related to various contacts
 @post correct values present in exec() output
 */
BOOST_AUTO_TEST_CASE(test_Exec_contact_filter)
{
    setup_testsuite testsuite;

    std::vector<std::string> handles_to_be_listed;

    for(int i=0; i<20; ++i) {
        setup_check(testsuite.testsuite_handle);
    }

    Test::contact the_contact;

    ::LibFred::OperationContextCreator ctx;
    std::string the_check_handle = ::LibFred::CreateContactCheck(
        the_contact.info_data.id,
        testsuite.testsuite_handle
    ).exec(ctx);

    std::vector<::LibFred::ListChecksItem> list = ::LibFred::ListContactChecks()
        .set_max_item_count(100)
        .set_contact_id(the_contact.info_data.id)
        .exec(ctx);

    BOOST_CHECK_EQUAL(list.size(), 1);
    BOOST_CHECK_EQUAL(list.begin()->check_handle, the_check_handle);
}

/**
 testing filters on testsuite and/or contact
 @pre existing check handles related to various testsuites and contacts
 @pre non-existent testsuite handle
 @pre non-existent contact id
 @post empty exec() output (and no exception thrown)
 */
BOOST_AUTO_TEST_CASE(test_Exec_filters_nonexistent_values)
{
    setup_testsuite testsuite;
    for(int i=0; i<10; ++i) {
        setup_check(testsuite.testsuite_handle);
    }

    setup_nonexistent_testsuite_handle nonexistent_testsuite;
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK_EQUAL(
        ::LibFred::ListContactChecks()
            .set_max_item_count(100)
            .set_testsuite_handle(nonexistent_testsuite.testsuite_handle)
            .exec(ctx)
                .size(),
        0
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::ListContactChecks()
            .set_max_item_count(100)
            .set_contact_id(Test::get_nonexistent_object_id(ctx))
            .exec(ctx)
                .size(),
        0
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
