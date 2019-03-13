/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "src/util/setup_server_decl.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"


#include "src/bin/corba/whois/whois_impl.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>
#include <utility>

BOOST_AUTO_TEST_SUITE(TestCorba)

const std::string server_name = "test-corbans";

BOOST_AUTO_TEST_CASE( test_corba_nameservice )
{
    //CORBA init
    corba_init();

    CorbaContainer::get_instance()->register_server(
        new ccReg_Whois_i(
        CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()
                ->get_conn_info(), server_name, true)
        , "TestWhois");

    CorbaContainer::get_instance()->register_server_process_object(
            new ccReg_Whois_i(
            CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()
                    ->get_conn_info(), server_name, true)
            , "TestServer", "Whois");

    CorbaContainer::get_instance()->poa_persistent->the_POAManager()->activate();

    //run orb in thread
    ThreadPtr orb_thread( CorbaContainer::get_instance()->run_orb_thread());

    //try to resolve
    ccReg::Whois_var whois1_ref;
    whois1_ref = ccReg::Whois::_narrow(
            CorbaContainer::get_instance()->nsresolve("TestWhois"));

    ccReg::Whois_var whois2_ref;
        whois2_ref = ccReg::Whois::_narrow(
    CorbaContainer::get_instance()->nsresolve_process_object(
            "TestServer", "Whois"));

    //test call
        ccReg::WhoisRegistrar_var registrar
            = whois2_ref->getRegistrarByHandle("REG-FRED_A");

        BOOST_CHECK_EQUAL(static_cast<std::string>(registrar->country).compare("CZ"), 0);

    CorbaContainer::get_instance()->orb->shutdown(true);
    orb_thread->join();//wait for thread end
}

BOOST_AUTO_TEST_SUITE_END();//TestCorba
