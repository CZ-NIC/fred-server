/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include <boost/test/unit_test.hpp>

#include "src/fredlib/object/generated_authinfo_password.h"
#include "src/fredlib/object/generated_authinfo_password_exception.h"

BOOST_AUTO_TEST_SUITE(TestGeneratedAuthInfoPassword)

BOOST_AUTO_TEST_CASE(test_valid_input)
{
    BOOST_CHECK_NO_THROW(
        Fred::GeneratedAuthInfoPassword("abcde23456")
    );
}

BOOST_AUTO_TEST_CASE(test_empty_input)
{
    BOOST_CHECK_THROW(
        Fred::GeneratedAuthInfoPassword(""),
        Fred::InvalidGeneratedAuthInfoPassword
    );
}

BOOST_AUTO_TEST_CASE(test_allowed_chars)
{
    BOOST_CHECK(
        ! Fred::get_chars_allowed_in_generated_authinfopw().empty()
    );
}

BOOST_AUTO_TEST_SUITE_END();