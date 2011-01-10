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
 *  @fred-msgd.cc
 *  implementation of registry messages server
 */

#include "messages.h"
#include "messages_filemanager.h"

#include "fredlib/db_settings.h"
#include "corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"
#include "corba/connection_releaser.h"

#include "pidfile.h"
#include "daemonize.h"

#include "cfg/config_handler.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"

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

const char* server_name = "msgd";//for logging contxt

int main(int argc, char** argv)
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::instance<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        Logging::Log::Type  log_type = static_cast<Logging::Log::Type>(CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleLoggingArgs>()->log_type);

        boost::any param;
        if (log_type == Logging::Log::LT_FILE) param = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleLoggingArgs>()->log_file;

        if (log_type == Logging::Log::LT_SYSLOG) param = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleLoggingArgs>()
            ->log_syslog_facility;

        Logging::Manager::instance_ref().get(PACKAGE)
            .addHandler(log_type, param);

        Logging::Manager::instance_ref().get(PACKAGE).setLevel(
                static_cast<Logging::Log::Level>(
                CfgArgs::instance()->get_handler_ptr_by_type
                <HandleLoggingArgs>()->log_level));

        Logging::Context ctx(server_name);

        {//db connection, test only
            Database::Connection conn = Database::Manager::acquire();
        }

        //CORBA init
        FakedArgs orb_fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
              get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
          , ns_args_ptr->nameservice_host
          , ns_args_ptr->nameservice_port
          , ns_args_ptr->nameservice_context);

        //Messages Manager
        Fred::Messages::ManagerPtr msgmgr
            = Fred::Messages::create_manager();

        //create server with poa and nameservice registration
        CorbaContainer::get_instance()
            ->register_server(new Registry_Messages_i(msgmgr), "Messages");

        CorbaContainer::get_instance()->poa_persistent->the_POAManager()
            ->activate();

        //run server
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleServerArgs>()
                ->do_daemonize)
            daemonize();
        string pidfile_name
            = CfgArgs::instance()->get_handler_ptr_by_type<HandleServerArgs>()
                                ->pidfile_name;
        if (!pidfile_name.empty())
            PidFileNS::PidFileS::writePid(getpid(), pidfile_name);

        CorbaContainer::get_instance()->orb->run();
        CorbaContainer::get_instance()->orb->destroy();

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
