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


#include "src/fredlib/contact/verification/list_enum_objects.h"
#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestListEnumObjects_integ, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification-list_enum_objects_integ";

/**
 testing correct returning of existing test result statuses
 @pre existing test result statuses
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_test_result_statuses)
{

}

/**
 testing correct returning of existing check statuses
 @pre existing check statuses
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_check_statuses)
{

}

/**
 testing correct returning of existing test definitions
 @pre existing test definitions
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_test_definitions)
{

}

/**
 testing correct returning of existing testsuite definitions
 @pre existing testsuite definitions
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_List_testsuite_definitions)
{

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
