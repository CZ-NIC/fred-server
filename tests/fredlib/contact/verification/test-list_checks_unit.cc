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
 *  unit tests for ListContactChecks operation
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/list_checks.h"
#include "fredlib/db_settings.h"
#include "util/db/nullable.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_AUTO_TEST_SUITE(TestListContactChecks_unit)

const std::string server_name = "test-contact_verification-list_checks_unit";

/**
 passing mandatory values to constructor
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Ctor_mandatory)
{
    std::vector<unsigned long> testdata;
    testdata.push_back( 10 );
    testdata.push_back( 1 );
    testdata.push_back( 100 );
    testdata.push_back( 13787343 );

    for( std::vector<unsigned long>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::ListContactChecks dummy(*it);
        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("max_item_count_: " + boost::lexical_cast<std::string>(*it) ) != std::string::npos,
            "Cannot find max item count \"" + boost::lexical_cast<std::string>(*it) + "\" in " + serialized + ".");
    }
}

/**
 passing mandatory and optional values to constructor
 @pre syntactically valid c-tor arguments
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Ctor_optional)
{
    typedef boost::tuple<unsigned long, Optional<std::string>, Optional<unsigned long long> > testcase_set;
    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple(114, "dummy_testsuite", 346) );
    testdata.push_back( boost::make_tuple(114, Optional<std::string>(), 346) );
    testdata.push_back( boost::make_tuple(114, "dummy_testsuite", Optional<unsigned long long>()) );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::ListContactChecks dummy(it->get<0>(), it->get<1>(), it->get<2>());
        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("max_item_count_: " + boost::lexical_cast<std::string>(it->get<0>()) ) != std::string::npos,
            "Cannot find max item count \"" + boost::lexical_cast<std::string>(it->get<0>()) + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("testsuite_name_: " + it->get<1>().print_quoted() ) != std::string::npos,
            "Cannot find testsuite name \"" + it->get<1>().print_quoted() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("contact_id_: " + it->get<2>().print_quoted() ) != std::string::npos,
            "Cannot find contact id \"" + it->get<2>().print_quoted() + "\" in " + serialized + ".");
    }
}

/**
 passing mandatory values to constructor and optional values to setter
 @pre syntactically valid c-tor and setters arguments
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Setter)
{
    typedef boost::tuple<unsigned long, Optional<std::string>, Optional<unsigned long long> > testcase_set;
    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple(114, "dummy_testsuite1", 346 ));
    testdata.push_back( boost::make_tuple(987, "dummy_testsuite2", 1534 ));
    testdata.push_back( boost::make_tuple(221, "dummy_testsuite3", 9834 ));

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::ListContactChecks dummy(it->get<0>());
        dummy.set_testsuite_name(it->get<1>());
        dummy.set_contact_id(it->get<2>());

        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("max_item_count_: " + boost::lexical_cast<std::string>(it->get<0>()) ) != std::string::npos,
            "Cannot find max item count \"" + boost::lexical_cast<std::string>(it->get<0>()) + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("testsuite_name_: " + it->get<1>().print_quoted() ) != std::string::npos,
            "Cannot find testsuite name \"" + it->get<1>().print_quoted() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("contact_id_: " + it->get<2>().print_quoted() ) != std::string::npos,
            "Cannot find contact id \"" + it->get<2>().print_quoted() + "\" in " + serialized + ".");
    }
}

/**
 passing mandatory and optional values to constructor and resetting optional values by setter
 @pre syntactically valid c-tor and setters arguments
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Ctor_versus_Setter)
{
    typedef boost::tuple<
        unsigned long,
        Optional<std::string>,
        Optional<unsigned long long>,
        Optional<std::string>,
        Optional<unsigned long long>
    > testcase_set;

    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple(114, "dummy_testsuite1", 346, "dummy_testsuite2", 537837) );
    testdata.push_back( boost::make_tuple(2454, Optional<std::string>(), 346, "dummy_testsuite452", 5757) );
    testdata.push_back( boost::make_tuple(23, "dummy_testsuite0", Optional<unsigned long long>(), "dummy_testsuite4543", 3234) );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::ListContactChecks dummy(it->get<0>(), it->get<1>(), it->get<2>());
        dummy.set_testsuite_name(it->get<3>());
        dummy.set_contact_id(it->get<4>());

        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("max_item_count_: " + boost::lexical_cast<std::string>(it->get<0>()) ) != std::string::npos,
            "Cannot find max item count \"" + boost::lexical_cast<std::string>(it->get<0>()) + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("testsuite_name_: " + it->get<3>().print_quoted() ) != std::string::npos,
            "Cannot find testsuite name \"" + it->get<3>().print_quoted() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("contact_id_: " + it->get<4>().print_quoted() ) != std::string::npos,
            "Cannot find contact id \"" + it->get<4>().print_quoted() + "\" in " + serialized + ".");
    }
}

/**
 passing optional value to setter and resetting it by second call to another value
 @pre syntactically valid c-tor and setters arguments
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Setter_reset)
{
    typedef boost::tuple<
        unsigned long,
        Optional<std::string>,
        Optional<unsigned long long>,
        Optional<std::string>,
        Optional<unsigned long long>
    > testcase_set;

    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple(114, "dummy_testsuite1", 346, "dummy_testsuite21", 537837) );
    testdata.push_back( boost::make_tuple(2454, Optional<std::string>(), 37, "dummy_testsuite22", 5757) );
    testdata.push_back( boost::make_tuple(23, "dummy_testsuite0", Optional<unsigned long long>(), "dummy_testsuite23", 5775 ) );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::ListContactChecks dummy(it->get<0>());
        dummy.set_testsuite_name(it->get<1>());
        dummy.set_contact_id(it->get<2>());

        dummy.set_testsuite_name(it->get<3>());
        dummy.set_contact_id(it->get<4>());

        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("max_item_count_: " + boost::lexical_cast<std::string>(it->get<0>()) ) != std::string::npos,
            "Cannot find max item count \"" + boost::lexical_cast<std::string>(it->get<0>()) + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("testsuite_name_: " + it->get<3>().print_quoted() ) != std::string::npos,
            "Cannot find testsuite name \"" + it->get<3>().print_quoted() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("contact_id_: " + it->get<4>().print_quoted() ) != std::string::npos,
            "Cannot find contact id \"" + it->get<4>().print_quoted() + "\" in " + serialized + ".");
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
