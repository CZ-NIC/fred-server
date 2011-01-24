/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE Test Server

#include "test-server.h"

//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs))
(HandleArgsPtr(new HandleThreadGroupArgs))
;

#include "cfg/test_custom_main.h"

BOOST_AUTO_TEST_SUITE(TestCpp)

void test_stdex()
{
    throw std::runtime_error("test ex");
}

BOOST_AUTO_TEST_CASE( test_exception )
{
    int check_counter = 0;
    try
    {
        try
        {
            test_stdex();
        }//try
        catch (const std::exception& ex)
        {
            //std::cout << "ex: " << ex.what() << std::endl;
            ++check_counter;
            throw;
        }
        catch (...)
        {
            check_counter+=3;
        }
    }//try
    catch (...)
    {}
    BOOST_REQUIRE_EQUAL(1, check_counter);
}//test_exception


class ConstInit
{
    static const int cstr = 5;
public:
    const int get_cstr() {return cstr;}

};

BOOST_AUTO_TEST_CASE( test_const_member_init )
{
    ConstInit ci;
    BOOST_REQUIRE_EQUAL(ci.get_cstr(), 5);
}

BOOST_AUTO_TEST_SUITE_END();//TestCpp

