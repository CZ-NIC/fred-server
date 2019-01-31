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

#include "libfred/db_settings.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"

#include "util/random_data_generator.hh"
#include "src/util/concurrent_queue.hh"
#include "src/bin/corba/admin/common.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>
#include <utility>

BOOST_AUTO_TEST_SUITE(TestAdmin)

BOOST_AUTO_TEST_CASE( test_bank_accounts )
{
    //CORBA init
    FakedArgs fa = CfgArgs::instance()->fa;
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
        , ns_args_ptr->nameservice_host
        , ns_args_ptr->nameservice_port
        , ns_args_ptr->nameservice_context);

    BOOST_TEST_MESSAGE( "ccReg::Admin::_narrow" );
    ccReg::Admin_var admin_ref;
    admin_ref = ccReg::Admin::_narrow(CorbaContainer::get_instance()->nsresolve("Admin"));

    //get db connection
    Database::Connection conn = Database::Manager::acquire();

    ccReg::EnumList_var el = admin_ref->getBankAccounts();

    BOOST_CHECK (el->length() > 0);

}

BOOST_AUTO_TEST_SUITE_END();//TestAdmin

