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
 *  @fred-adifd.cc
 *  implementation of admin interface
 */

#include "config.h"
#include "src/bin/corba/Admin.hh"
#include "src/bin/corba/admin/admin_impl.hh"
#include "src/bin/corba/admin_block/server_i.hh"
#include "src/bin/corba/Notification.hh"
#include "src/bin/corba/notification/server_i.hh"

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
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_adifd_args.hh"

#include "src/bin/corba/admin_contact_verification/server_i.hh"

using namespace std;

const string server_name = "fred-adifd";

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
    (HandleArgsPtr(new HandleServerArgs))
    (HandleArgsPtr(new HandleLoggingArgs))
    (HandleArgsPtr(new HandleDatabaseArgs))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs))
    (HandleArgsPtr(new HandleRegistryArgs))
    (HandleArgsPtr(new HandleAdifdArgs));

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
        HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleAdifdArgs* adifd_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleAdifdArgs>();

        std::unique_ptr<ccReg_Admin_i> myccReg_Admin_i ( new ccReg_Admin_i(
                    db_args_ptr->get_conn_info()
                    , CorbaContainer::get_instance()->getNS()
                    , registry_args_ptr->restricted_handles
                    , registry_args_ptr->docgen_path
                    , registry_args_ptr->docgen_template_path
                    , registry_args_ptr->docgen_domain_count_limit
                    , registry_args_ptr->fileclient_path
                    , adifd_args_ptr->adifd_session_max
                    , adifd_args_ptr->adifd_session_timeout
                    , adifd_args_ptr->adifd_session_garbage
            ));

        std::unique_ptr<CorbaConversion::AdminContactVerification::Server_i>
            admin_contact_verification_server(new(CorbaConversion::AdminContactVerification::Server_i));

            // create session use values from config
            LOGGER.info(boost::format(
                    "sessions max: %1%; timeout: %2%")
                    % adifd_args_ptr->adifd_session_max
                    % adifd_args_ptr->adifd_session_timeout);

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()
            ->register_server(myccReg_Admin_i.release(), "Admin");
        CorbaContainer::get_instance()
            ->register_server(admin_contact_verification_server.release(), "AdminContactVerification");

        CorbaContainer::get_instance()
            ->register_server(new CorbaConversion::AdministrativeBlocking::Server_i(server_name), "AdminBlocking");

        CorbaContainer::get_instance()
            ->register_server(new Registry::Notification::Server_i(server_name), "Notification");

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
