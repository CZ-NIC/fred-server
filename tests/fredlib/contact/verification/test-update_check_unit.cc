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
 *  unit tests for UpdateContactCheck operation
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/update_check.h"
#include "fredlib/db_settings.h"
#include "util/db/nullable.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestUpdateContactCheck_unit)

const std::string server_name = "test-contact_verification-update_check_unit";

/**
 passing mandatory values to constructor
 @pre valid check handle
 @pre valid status name
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Ctor_mandatory)
{
    typedef std::pair<std::string, std::string> testcase_set;
    std::vector< testcase_set > testdata;
    testdata.push_back( std::make_pair("check_handle15747", "status32174") );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::UpdateContactCheck dummy(it->first, it->second);
        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("check_handle_: " + it->first ) != std::string::npos,
            "Cannot find handle \"" + it->first + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("status_name_: " + it->second ) != std::string::npos,
            "Cannot find testname \"" + it->second + "\" in " + serialized + ".");
    }
}

/**
 passing mandatory and optional values to constructor
 @pre valid check handle
 @pre valid status name
 @pre valid logd request id
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Ctor_optional)
{
    typedef boost::tuple<std::string, std::string, long long> testcase_set;
    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple("check_handle33440", "status24274", 678643) );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::UpdateContactCheck dummy(it->get<0>(), it->get<1>(), it->get<2>());
        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("check_handle_: " + it->get<0>() ) != std::string::npos,
            "Cannot find handle \"" + it->get<0>() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("status_name_: " + it->get<1>() ) != std::string::npos,
            "Cannot find testname \"" + it->get<1>() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("logd_request_id_: " + Nullable<long long>( it->get<2>() ).print_quoted() ) != std::string::npos,
            "Cannot find logd request id \"" + Nullable<long long>( it->get<2>() ).print_quoted() + "\" in " + serialized + ".");
    }
}

/**
 passing mandatory values to constructor and optional values to setter
 @pre valid check handle
 @pre valid status name
 @pre valid logd request id
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Setter)
{
    typedef boost::tuple<std::string, std::string, long long> testcase_set;
    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple("check_handle33440", "status24274", 678643) );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::UpdateContactCheck dummy(it->get<0>(), it->get<1>());
        dummy.set_logd_request_id(it->get<2>());

        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("check_handle_: " + it->get<0>() ) != std::string::npos,
            "Cannot find handle \"" + it->get<0>() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("status_name_: " + it->get<1>() ) != std::string::npos,
            "Cannot find testname \"" + it->get<1>() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("logd_request_id_: " + Nullable<long long>( it->get<2>() ).print_quoted() ) != std::string::npos,
            "Cannot find logd request id \"" + Nullable<long long>( it->get<2>() ).print_quoted() + "\" in " + serialized + ".");
    }
}

/**
 passing mandatory and optional values to constructor and resetting optional values by setter
 @pre valid check handle
 @pre valid status name
 @pre valid logd request id
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Ctor_versus_Setter)
{
    typedef boost::tuple<std::string, std::string, long long> testcase_set;
    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple("check_handle33440", "status24274", 678643) );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::UpdateContactCheck etalon(it->get<0>(), it->get<1>(), it->get<2>());
        Fred::UpdateContactCheck dummy(it->get<0>(), it->get<1>());
        dummy.set_logd_request_id(it->get<2>());

        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized == etalon.to_string(),
            "Operation with logd_request_id set in C'tor \"" + etalon.to_string() + "\" differs from operation with logd_request_id set by method " + serialized + ".");
    }
}

/**
 passing optional value to setter and resetting it by second call to another value
 @pre valid check handle
 @pre valid status name
 @pre valid logd request id
 @pre another valid logd request id
 @post correct values present in to_string() output
 */
BOOST_AUTO_TEST_CASE(test_Setter_reset)
{
    typedef boost::tuple<std::string, std::string, long long, long long> testcase_set;
    std::vector<testcase_set> testdata;
    testdata.push_back( boost::make_tuple("check_handle33440", "status24274", 678643, 354315) );

    for( std::vector<testcase_set>::iterator it = testdata.begin(); it != testdata.end(); ++it) {
        Fred::UpdateContactCheck dummy(it->get<0>(), it->get<1>());
        dummy.set_logd_request_id(it->get<2>());
        dummy.set_logd_request_id(it->get<3>());

        std::string serialized = dummy.to_string();
        BOOST_CHECK_MESSAGE(
            serialized.find("check_handle_: " + it->get<0>() ) != std::string::npos,
            "Cannot find handle \"" + it->get<0>() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("status_name_: " + it->get<1>() ) != std::string::npos,
            "Cannot find testname \"" + it->get<1>() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("logd_request_id_: " + Nullable<long long>( it->get<2>() ).print_quoted() ) == std::string::npos,
            "Found invalid logd request id \"" + Nullable<long long>( it->get<2>() ).print_quoted() + "\" in " + serialized + ".");
        BOOST_CHECK_MESSAGE(
            serialized.find("logd_request_id_: " + Nullable<long long>( it->get<3>() ).print_quoted() ) != std::string::npos,
            "Cannot find logd request id \"" + Nullable<long long>( it->get<3>() ).print_quoted() + "\" in " + serialized + ".");
    }
}

BOOST_AUTO_TEST_SUITE_END();
