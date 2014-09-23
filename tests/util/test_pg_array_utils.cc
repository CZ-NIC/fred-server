/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#include "util/db/pg_array_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN


#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPgArrayUtils)

const std::string server_name = "test-pg-array-utils";


BOOST_AUTO_TEST_CASE(test)
{
    std::vector<Nullable<std::string> > result1 = PgArray(
        "{\"breakfast\",consulting,NULL,\"null\",test+test,test2;test2,\"\\\"b\\\"\",\"test\\\\test\"}")
        .parse();
    BOOST_CHECK(!result1.at(0).isnull());
    BOOST_CHECK(result1.at(0).get_value() == "breakfast");
    BOOST_CHECK(!result1.at(1).isnull());
    BOOST_CHECK(result1.at(1).get_value() == "consulting");
    BOOST_CHECK(result1.at(2).isnull());
    BOOST_CHECK(!result1.at(3).isnull());
    BOOST_CHECK(result1.at(3).get_value() == "null");
    BOOST_CHECK(!result1.at(4).isnull());
    BOOST_CHECK(result1.at(4).get_value() == "test+test");
    BOOST_CHECK(!result1.at(5).isnull());
    BOOST_CHECK(result1.at(5).get_value() == "test2;test2");
    BOOST_CHECK(!result1.at(6).isnull());
    BOOST_CHECK(result1.at(6).get_value() == "\"b\"");
    BOOST_CHECK(!result1.at(7).isnull());
    BOOST_CHECK(result1.at(7).get_value() == "test\\test");


    std::vector<Nullable<std::string> > result2 = PgArray(
        "{{1,2},{3,4},{5,6}}")
        .parse();

    BOOST_CHECK(!result2.at(0).isnull());
    BOOST_CHECK(result2.at(0).get_value() == "{1,2}");
    BOOST_CHECK(!result2.at(1).isnull());
    BOOST_CHECK(result2.at(1).get_value() == "{3,4}");
    BOOST_CHECK(!result2.at(2).isnull());
    BOOST_CHECK(result2.at(2).get_value() == "{5,6}");

    std::vector<Nullable<std::string> > result3 = PgArray(
        result2.at(0).get_value())
            .parse();

    BOOST_CHECK(!result3.at(0).isnull());
    BOOST_CHECK(result3.at(0).get_value() == "1");
    BOOST_CHECK(!result3.at(1).isnull());
    BOOST_CHECK(result3.at(1).get_value() == "2");

    std::vector<Nullable<std::string> > result4 = PgArray(
        result2.at(1).get_value())
            .parse();

    BOOST_CHECK(!result4.at(0).isnull());
    BOOST_CHECK(result4.at(0).get_value() == "3");
    BOOST_CHECK(!result4.at(1).isnull());
    BOOST_CHECK(result4.at(1).get_value() == "4");

    std::vector<Nullable<std::string> > result5 = PgArray(
        result2.at(2).get_value())
            .parse();

    BOOST_CHECK(!result5.at(0).isnull());
    BOOST_CHECK(result5.at(0).get_value() == "5");
    BOOST_CHECK(!result5.at(1).isnull());
    BOOST_CHECK(result5.at(1).get_value() == "6");
}

BOOST_AUTO_TEST_SUITE_END();//TestPgArrayUtils
