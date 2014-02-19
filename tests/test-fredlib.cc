/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#define BOOST_TEST_MODULE Test Fredlib

#include <memory>
#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include "src/fredlib/db_settings.h"
//#include "util/corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
//#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_rifd_args.h"
#include "cfg/handle_contactverification_args.h"
#include "cfg/handle_mojeid_args.h"


//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleServerArgs))
(HandleArgsPtr(new HandleLoggingArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
//(HandleArgsPtr(new HandleCorbaNameServiceArgs))
(HandleArgsPtr(new HandleThreadGroupArgs))
(HandleArgsPtr(new HandleRegistryArgs))
(HandleArgsPtr(new HandleRifdArgs))
(HandleArgsPtr(new HandleContactVerificationArgs))
(HandleArgsPtr(new HandleMojeIDArgs))
;

#include "cfg/test_custom_main.h"

class ElapsedTimeFixture
{
    ElapsedTime et_;
public:
    ElapsedTimeFixture()
    : et_("elapsed time: ", cout_print())
    {}
};

BOOST_GLOBAL_FIXTURE(ElapsedTimeFixture);


void setup_logging(CfgArgs * cfg_instance_ptr)
{
    // setting up logger
    Logging::Log::Type  log_type = static_cast<Logging::Log::Type>(
        cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>()
            ->log_type);

    boost::any param;
    if (log_type == Logging::Log::LT_FILE) param = cfg_instance_ptr
        ->get_handler_ptr_by_type<HandleLoggingArgs>()->log_file;

    if (log_type == Logging::Log::LT_SYSLOG) param = cfg_instance_ptr
        ->get_handler_ptr_by_type<HandleLoggingArgs>()
        ->log_syslog_facility;

    Logging::Manager::instance_ref().get(PACKAGE)
        .addHandler(log_type, param);

    Logging::Manager::instance_ref().get(PACKAGE).setLevel(
        static_cast<Logging::Log::Level>(
        cfg_instance_ptr->get_handler_ptr_by_type
            <HandleLoggingArgs>()->log_level));
}

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

BOOST_AUTO_TEST_SUITE_END();//TestCpp

