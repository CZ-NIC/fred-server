/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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
 *  @test_custom_main.h
 *  main fn. for manipulation with cmdline arguments in boost test
 *
 *  This version needs definition of configuration handlers in global variable
 *  possibly with some handler constructor parameters.
 *
 *  HandleGeneralArgs do:
 *   - fred server.conf reading into cmdline arguments
 *   - help print for options of others Handlers (prepared by CfgArgs class)
 *  So if you want this functionality and don't want to implement it youself
 *  it does make sense to have HandleGeneralArgs first in list.
 *
 *  Ex:
 *  HandlerPtrVector global_hpv =
 *  boost::assign::list_of
 *  (HandleArgsPtr(new HandleGeneralArgs))
 *  (HandleArgsPtr(new HandleDatabaseArgs))
 *  (HandleArgsPtr(new HandleThreadGroupArgs))
 *  (HandleArgsPtr(new HandleCorbaNameServiceArgs));
 */


#ifndef TEST_CUSTOM_MAIN_H_
#define TEST_CUSTOM_MAIN_H_
//not using UTF defined main
#define BOOST_TEST_NO_MAIN

// Sun CC doesn't handle boost::iterator_adaptor yet
#if !defined(__SUNPRO_CC) || (__SUNPRO_CC > 0x530)
#include <boost/generator_iterator.hpp>
#endif

#include "config_handler.h"
#include "time_clock.h"
#include <boost/test/included/unit_test.hpp>

int main( int argc, char* argv[] )
{

    //processing of additional program options
    //producing faked args with unrecognized ones
    FakedArgs fa;
    try
    {
        fa = CfgArgs::instance<HandleGeneralArgs>(global_hpv)->handle(argc, argv).copy_onlynospaces_args();
    }
    catch(const ReturnFromMain&)
    {
        return 0;
    }

//fn init_unit_test_suite added in 1.35.0
#if ( BOOST_VERSION > 103401 )

    // prototype for user's unit test init function
#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    extern bool init_unit_test();

    boost::unit_test::init_unit_test_func init_func = &init_unit_test;
#else
    extern ::boost::unit_test::test_suite* init_unit_test_suite( int argc, char* argv[] );

    boost::unit_test::init_unit_test_func init_func = &init_unit_test_suite;
#endif

    return ::boost::unit_test::unit_test_main( init_func, fa.get_argc(), fa.get_argv() );//using fake args
#else //1.34.1 and older
    return ::boost::unit_test::unit_test_main(  fa.get_argc(), fa.get_argv() );//using fake args
#endif //1.35.0 and newer

}//main

#endif // TEST_CUSTOM_MAIN_H_
