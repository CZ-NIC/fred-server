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
#include <set>
#include "libfred/object/generate_authinfo_password.hh"

BOOST_AUTO_TEST_SUITE(TestGenerateAuthInfoPassword)

BOOST_AUTO_TEST_CASE(test_no_exception)
{
    BOOST_CHECK_NO_THROW(
        ::LibFred::generate_authinfo_pw()
    );
}

BOOST_AUTO_TEST_CASE(test_variability_naive)
{
    /* This test can fail because of probability (bad luck).*/
    std::set<std::string> gen_authinfo_pw_duplicity;
    for(unsigned i = 0; i < 100; ++i) {//check that generated passwords are unique(no guarantee)
        const ::LibFred::GeneratedAuthInfoPassword temp = ::LibFred::generate_authinfo_pw();
        BOOST_CHECK(gen_authinfo_pw_duplicity.insert(temp.password_).second);
    }
}

BOOST_AUTO_TEST_CASE(test_gen_authinfo_len)
{
    const int authinfo_pw_length = 8;
    for(unsigned i = 0; i < 1000; ++i) {
        BOOST_CHECK(::LibFred::generate_authinfo_pw().password_.length() == authinfo_pw_length);
    }
}

BOOST_AUTO_TEST_CASE(test_gen_authinfo_chars)
{
    for(unsigned i = 0; i < 1000; ++i) {
        BOOST_CHECK(::LibFred::generate_authinfo_pw().password_.find_first_not_of(
            ::LibFred::get_chars_allowed_in_generated_authinfopw()) == std::string::npos);
    }
}

BOOST_AUTO_TEST_SUITE_END();
