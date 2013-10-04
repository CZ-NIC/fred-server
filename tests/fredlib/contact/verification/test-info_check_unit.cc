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
 *  unit tests for InfoContactCheck operation
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/info_check.h"
#include "fredlib/db_settings.h"
#include "util/db/nullable.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_AUTO_TEST_SUITE(TestInfoContactCheck_unit)

const std::string server_name = "test-contact_verification-info_check_unit";

/**
 passing mandatory values to constructor
 @pre valid check handle
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Ctor)
{
    std::vector<std::string> testdata;
    testdata.push_back( "check_handle15747" );

    for( std::vector<std::string>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::InfoContactCheck dummy(*it);
        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("check_handle_: " + *it ) != std::string::npos,
            "Cannot find handle \"" + *it + "\" in " + serialized + ".");
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
