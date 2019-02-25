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
 *  test for optional and nullable values (of the same type) comparison
 */

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "util/is_equal_optional_nullable.hh"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestOptionalNullableComparison)

const std::string server_name = "test-is-equal-optional-nullable";

BOOST_AUTO_TEST_CASE(BasicTemplate)
{
    Optional<int> opt;
    Nullable<int> null;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null),
        "Unset Optional should equal to NULLed Nullable." );

    BOOST_CHECK_MESSAGE(
        Util::is_equal(null, opt),
        "NULLed Nullable should equal to Unset Optional." );

    null = 5;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null) == false,
        "Unset Optional should not equal to Nullable set to value." );

    BOOST_CHECK_MESSAGE(
        Util::is_equal(null, opt) == false,
        "Nullable set to value should not equal to Unset Optional." );

    opt = 5;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null),
        "Optional and Nullable set to the same value should equal." );

    BOOST_CHECK_MESSAGE(
        Util::is_equal(null, opt),
        "Nullable and Optional set to the same value should equal." );

    Nullable<int> null2;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null2) == false,
        "Optional set to value should not equal to Nullable set to NULL." );

    BOOST_CHECK_MESSAGE(
        Util::is_equal(null2, opt) == false,
        "Nullable set to NULL should not equal to Optional set to value." );

    null = 2;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null) == false,
        "Optional and Nullable set to different values should not equal." );

    BOOST_CHECK_MESSAGE(
        Util::is_equal(null, opt) == false,
        "Nullable and Optional set to different values should not equal." );
}
/**
 * test Util::is_equal comparison for both Nullable<T>
 */
BOOST_AUTO_TEST_CASE(test_nullable_comparisons)
{
    BOOST_CHECK(Util::is_equal(Nullable<std::string>(), Nullable<std::string>()));//both null
    BOOST_CHECK(Util::is_equal(Nullable<std::string>("test"), Nullable<std::string>("test")));//both not null, same value
    BOOST_CHECK(!Util::is_equal(Nullable<std::string>("test1"), Nullable<std::string>("test2")));//both not null, different value
    BOOST_CHECK(!Util::is_equal(Nullable<std::string>(), Nullable<std::string>("")));//one null, other not
    BOOST_CHECK(!Util::is_equal(Nullable<std::string>(""), Nullable<std::string>()));//one null, other not
}

/**
 * test Util::is_equal upper-case comparison for both Nullable<T>
 */
BOOST_AUTO_TEST_CASE(test_nullable_uppercase_comparisons)
{
    BOOST_CHECK(Util::is_equal_upper(Nullable<std::string>(), Nullable<std::string>()));//both null
    BOOST_CHECK(Util::is_equal_upper(Nullable<std::string>("Test"), Nullable<std::string>("test")));//both not null, same uppercase value
    BOOST_CHECK(Util::is_equal_upper(Nullable<std::string>("test"), Nullable<std::string>("Test")));//both not null, same uppercase value
    BOOST_CHECK(!Util::is_equal_upper(Nullable<std::string>("Test1"), Nullable<std::string>("test2")));//both not null, different value
    BOOST_CHECK(!Util::is_equal_upper(Nullable<std::string>(), Nullable<std::string>("")));//one null, other not
    BOOST_CHECK(!Util::is_equal_upper(Nullable<std::string>(""), Nullable<std::string>()));//one null, other not
}

/**
 * test Util::is_equal lower-case comparison for both Nullable<T>
 */
BOOST_AUTO_TEST_CASE(test_nullable_lowercase_comparisons)
{
    BOOST_CHECK(Util::is_equal_upper(Nullable<std::string>(), Nullable<std::string>()));//both null
    BOOST_CHECK(Util::is_equal_upper(Nullable<std::string>("Test"), Nullable<std::string>("test")));//both not null, same uppercase value
    BOOST_CHECK(Util::is_equal_upper(Nullable<std::string>("test"), Nullable<std::string>("Test")));//both not null, same uppercase value
    BOOST_CHECK(!Util::is_equal_upper(Nullable<std::string>("Test1"), Nullable<std::string>("test2")));//both not null, different value
    BOOST_CHECK(!Util::is_equal_upper(Nullable<std::string>(), Nullable<std::string>("")));//one null, other not
    BOOST_CHECK(!Util::is_equal_upper(Nullable<std::string>(""), Nullable<std::string>()));//one null, other not
}


BOOST_AUTO_TEST_SUITE_END();
