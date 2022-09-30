/*
 * Copyright (C) 2010-2022  CZ.NIC, z. s. p. o.
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

#include <boost/test/included/unit_test.hpp>

#include "test/poc/parallel-tests/setup/cfg.hh"
#include "test/poc/parallel-tests/fixtures/has_fresh_database.hh"

#include "src/util/cfg/handle_args.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_domainbrowser_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/corba_wrapper.hh"
#include "src/util/setup_server.hh"

#include "util/log/logger.hh"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <utility>

namespace {

class ElapsedTime
{
public:
    ElapsedTime() : start_{std::chrono::system_clock::now()} { }
    ~ElapsedTime()
    {
        const auto dt = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_).count();
        const auto seconds = dt / 1'000'000;
        const auto microseconds = dt % 1'000'000;
        std::cout << "Elapsed time: ";
        if (3600 <= seconds)
        {
            std::cout << (seconds / 3600) << ":"
                      << std::setw(2) << std::setfill('0') << ((seconds / 60) % 60);
        }
        else
        {
            std::cout << (seconds / 60);
        }
        std::cout << ":" << std::setw(2) << std::setfill('0') << (seconds % 60) << "."
                  << std::setw(6) << std::setfill('0') << microseconds << std::endl;
    }
    std::chrono::time_point<std::chrono::system_clock> start_;
};

void dummy_setup() { }

class GlobalFixture : ElapsedTime
{
public:
    GlobalFixture()
        : database_administrator_{
                []()
                {
                    const HandlerPtrVector config_handlers = {
                            std::make_shared<HandleLoggingArgs>(),
                            std::make_shared<HandleServerArgs>(),
                            std::make_shared<HandleDatabaseArgs>(),
                            std::make_shared<HandleCorbaNameServiceArgs>(),
                            std::make_shared<HandleThreadGroupArgs>(),
                            std::make_shared<HandleRegistryArgs>(),
                            std::make_shared<HandleRifdArgs>(),
                            std::make_shared<HandleContactVerificationArgs>(),
                            std::make_shared<HandleDomainBrowserArgs>(),
                            std::make_shared<HandleMojeIdArgs>()};
                    return Test::Cfg::handle_command_line_args(config_handlers, dummy_setup);
                }()}
    {
        Test::HasFreshDatabase::set_restore_test_database_procedure(database_administrator_.get_restore_test_database_procedure());
        ::LOGGER.info("tests start");
    }
    ~GlobalFixture()
    {
        try
        {
            ::LOGGER.info("tests done");
            Test::HasFreshDatabase::clear_restore_test_database_procedure();
        }
        catch (...) { }
    }
private:
    Test::Cfg::DatabaseAdministrator database_administrator_;
};

}//namespace {anonymous}

BOOST_GLOBAL_FIXTURE(GlobalFixture);


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

BOOST_AUTO_TEST_SUITE_END()//TestCpp
