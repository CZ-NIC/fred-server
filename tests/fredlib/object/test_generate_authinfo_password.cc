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

#include "src/fredlib/object/generate_authinfo_password.h"

BOOST_AUTO_TEST_SUITE(TestGenerateAuthInfoPassword)

BOOST_AUTO_TEST_CASE(test_no_exception)
{
    BOOST_CHECK_NO_THROW(
        Fred::generate_authinfo_pw()
    );
}

BOOST_AUTO_TEST_CASE(test_variability_naive)
{
    /* This test could fail because of probability (bad luck). But 1000 idential password in a row should not happen often so better check it. */

    const Fred::GeneratedAuthInfoPassword first = Fred::generate_authinfo_pw();

    bool all_password_are_the_same = true;

    for(unsigned i = 0; i < 1000; ++i) {
        const Fred::GeneratedAuthInfoPassword temp = Fred::generate_authinfo_pw();
        if(first.password_ != temp.password_) {
            all_password_are_the_same = false;
            break;
        }

    }

    BOOST_CHECK( !all_password_are_the_same );
}

BOOST_AUTO_TEST_SUITE_END();
