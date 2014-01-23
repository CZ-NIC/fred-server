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
 *  integration tests for ListContactChecks operation
 */


#include "src/fredlib/contact/verification/list_checks.h"
#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestListContactChecks_integ, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification-list_checks_integ";

/**
 testing number of returned objects with different numbers of pre-created checks and different given value of parameter
 @pre existing check handles
 @post correct values present in exec() output
 */
BOOST_AUTO_TEST_CASE(test_Exec_max_count)
{
    // TODO (obviously)
}

/**
 testing filters on testsuite and/or contact
 @pre existing check handles related to various testsuites and contacts
 @pre existing testsuite name
 @pre existing contact id
 @post correct values present in exec() output
 */
BOOST_AUTO_TEST_CASE(test_Exec_filters)
{
    // TODO (obviously)
}

/**
 testing filters on testsuite and/or contact
 @pre existing check handles related to various testsuites and contacts
 @pre non-existent testsuite name
 @pre non-existent contact id
 @post empty exec() output (and no exception thrown)
 */
BOOST_AUTO_TEST_CASE(test_Exec_filters_nonexistent_values)
{
    // TODO (obviously)
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
