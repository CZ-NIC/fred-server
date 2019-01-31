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
 *  @fred-logd.cc
 *  implementation of logger interface
 */

#include "config.h"
#include "src/bin/corba/Logger.hh"
#include "src/bin/corba/admin/admin_impl.hh"
#include "src/bin/corba/log/log_impl_wrap.hh"

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>
#include <utility>

#include "libfred/db_settings.hh"
#include "src/util/corba_wrapper.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/bin/corba/connection_releaser.hh"
#include "src/util/setup_server.hh"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_logd_args.hh"

using namespace std;

const string server_name = "fred-logd";

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
    (HandleArgsPtr(new HandleServerArgs))
    (HandleArgsPtr(new HandleLoggingArgs))
    (HandleArgsPtr(new HandleDatabaseArgs))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs))
    (HandleArgsPtr(new HandleLogdArgs))
    ;

int main(int argc, char *argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        setup_logging(CfgArgs::instance());

        Logging::Context ctx(server_name);

        // config dump
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump)
        {
            for (std::string config_item = AccumulatedConfig::get_instance().pop_front();
                !config_item.empty(); config_item = AccumulatedConfig::get_instance().pop_front())
            {
                Logging::Manager::instance_ref().debug(config_item);
            }
        }

        //CORBA init
        corba_init();

        //conf pointers
        HandleDatabaseArgs* db_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleDatabaseArgs>();
        HandleLogdArgs* logd_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleLogdArgs>();

        std::unique_ptr<ccReg_Log_i> myccReg_Log_i (
            new ccReg_Log_i(db_args_ptr->get_conn_info()
                            , logd_args_ptr->logd_monitoring_hosts_file));

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()
            ->register_server(myccReg_Log_i.release(), "Logger");

        run_server(CfgArgs::instance(), CorbaContainer::get_instance());

    }//try
    catch(CORBA::TRANSIENT&)
    {
        cerr << "Caught system exception TRANSIENT -- unable to contact the "
             << "server." << endl;
        return EXIT_FAILURE;
    }
    catch(CORBA::SystemException& ex)
    {
        cerr << "Caught a CORBA::" << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch(CORBA::Exception& ex)
    {
        cerr << "Caught CORBA::Exception: " << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch(omniORB::fatalException& fe)
    {
        cerr << "Caught omniORB::fatalException:" << endl;
        cerr << "  file: " << fe.file() << endl;
        cerr << "  line: " << fe.line() << endl;
        cerr << "  mesg: " << fe.errmsg() << endl;
        return EXIT_FAILURE;
    }

    catch(const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch(exception& ex)
    {
        cerr << "Error: " << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        cerr << "Unknown Error" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
