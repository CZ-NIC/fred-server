/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include <boost/test/unit_test.hpp>

#include "libfred/object/generated_authinfo_password.hh"
#include "libfred/object/generated_authinfo_password_exception.hh"

BOOST_AUTO_TEST_SUITE(TestGeneratedAuthInfoPassword)

BOOST_AUTO_TEST_CASE(test_valid_input)
{
    BOOST_CHECK_NO_THROW(
        ::LibFred::GeneratedAuthInfoPassword("abcde23456")
    );
}

BOOST_AUTO_TEST_CASE(test_empty_input)
{
    BOOST_CHECK_THROW(
        ::LibFred::GeneratedAuthInfoPassword(""),
        ::LibFred::InvalidGeneratedAuthInfoPassword
    );
}

BOOST_AUTO_TEST_CASE(test_allowed_chars)
{
    BOOST_CHECK(
        ! ::LibFred::get_chars_allowed_in_generated_authinfopw().empty()
    );
}

BOOST_AUTO_TEST_SUITE_END();
