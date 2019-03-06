/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
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
#define BOOST_TEST_MODULE Test Server

#include <utility>

#include "test/deprecated/test_server.hh"

#include "config.h" // needed for CONFIG_FILE

//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleTestsArgs(CONFIG_FILE)))
(HandleArgsPtr(new HandleServerArgs))
(HandleArgsPtr(new HandleLoggingArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs))
(HandleArgsPtr(new HandleThreadGroupArgs))
(HandleArgsPtr(new HandleRegistryArgs))
(HandleArgsPtr(new HandleRifdArgs))
(HandleArgsPtr(new HandleContactVerificationArgs))
(HandleArgsPtr(new HandleMojeIdArgs))
(HandleArgsPtr(new HandleDomainBrowserArgs))
;

#include "src/util/cfg/test_custom_main.hh"

class ElapsedTimeFixture
{
    ElapsedTime et_;
public:
    ElapsedTimeFixture()
    : et_("elapsed time: ", cout_print())
    {}
};

BOOST_GLOBAL_FIXTURE(ElapsedTimeFixture);


class LoggingFixture {
public:
    LoggingFixture() {
        // setting up logger
        setup_logging(CfgArgs::instance());
    }

};

BOOST_GLOBAL_FIXTURE( LoggingFixture );



BOOST_AUTO_TEST_SUITE(TestCpp)

void test_stdex()
{
    throw std::runtime_error("test ex");
}

struct DUMMY_EXCEPTION : std::runtime_error
{
    DUMMY_EXCEPTION(const std::string& what)
            : std::runtime_error(what)
    {}
};

struct DUMMY_EXCEPTION_1 : std::runtime_error
{
    DUMMY_EXCEPTION_1()
            : std::runtime_error("error")
    {}
};


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

    {
        int DUMMY_EXCEPTION_catch_check = 0;
        try
        {
            throw DUMMY_EXCEPTION("error");
        }
        catch(const std::exception& ex)
        {
            //std::cout << "ex: " << ex.what() << std::endl;
            ++DUMMY_EXCEPTION_catch_check;
        }
        BOOST_REQUIRE_EQUAL(1, DUMMY_EXCEPTION_catch_check);
    }//DUMMY_EXCEPTION_catch_check

    {
        int DUMMY_EXCEPTION_catch_check = 0;
        try
        {
            throw DUMMY_EXCEPTION_1();
        }
        catch(const std::exception& ex)
        {
            //std::cout << "ex: " << ex.what() << std::endl;
            ++DUMMY_EXCEPTION_catch_check;
        }
        BOOST_REQUIRE_EQUAL(2, DUMMY_EXCEPTION_catch_check+1);
    }//DUMMY_EXCEPTION_catch_check


}//test_exception


class ConstInit
{
public:
    static constexpr int get_cstr()
    {
        constexpr int cstr = 5;
        return cstr;
    }
};

BOOST_AUTO_TEST_CASE( test_const_member_init )
{
    ConstInit ci;
    BOOST_REQUIRE_EQUAL(ci.get_cstr(), 5);
}

BOOST_AUTO_TEST_SUITE_END();//TestCpp

