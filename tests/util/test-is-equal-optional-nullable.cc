/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  test for optional and nullable values (of the same type) comparison
 */

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "util/is_equal_optional_nullable.h"
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

    null = 5;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null) == false,
        "Unset Optional should not equal to Nullable set to value." );

    opt = 5;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null),
        "Optional and Nullable set to the same value should equal." );

    Nullable<int> null2;

    BOOST_CHECK_MESSAGE(
        Util::is_equal(opt, null2) == false,
        "Optional set to value should not equal to Nullable set NULL." );
}

BOOST_AUTO_TEST_SUITE_END();
