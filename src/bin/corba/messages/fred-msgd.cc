/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @fred-msgd.cc
 *  implementation of registry messages server
 */

#include <utility>

#include "src/bin/corba/messages/messages.hh"
#include "src/bin/corba/messages/messages_filemanager.hh"

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

using namespace std;

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: fred-msgd <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
    (HandleArgsPtr(new HandleServerArgs))
    (HandleArgsPtr(new HandleLoggingArgs))
    (HandleArgsPtr(new HandleDatabaseArgs))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs));

const char* server_name = "fred-msgd";//for logging contxt

int main(int argc, char** argv)
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config processing
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        setup_logging(CfgArgs::instance());

        Logging::Context ctx(server_name);

        const DestroyCorbaContainerInMyDestructor prevent_usage_of_destroyed_logging_singleton;

        // config dump
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump)
        {
            for (std::string config_item = AccumulatedConfig::get_instance().pop_front();
                !config_item.empty(); config_item = AccumulatedConfig::get_instance().pop_front())
            {
                Logging::Manager::instance_ref().debug(config_item);
            }
        }
        {//db connection, test only
            Database::Connection conn = Database::Manager::acquire();
            Database::Manager::release();
        }

        corba_init();

        //Messages Manager
        LibFred::Messages::ManagerPtr msgmgr
            = LibFred::Messages::create_manager();

        //register server with poa and nameservice
        CorbaContainer::get_instance()
            ->register_server(new Registry_Messages_i(msgmgr), "Messages");

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
}//main
