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
#include "src/bin/corba/domain_browser/domain_browser_i.hh"

#include <iostream>
#include <string>

#include <boost/assign/list_of.hpp>
#include <utility>

#include "src/bin/corba/connection_releaser.hh"
#include "libfred/db_settings.hh"
#include "src/util/corba_wrapper.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"

#include "src/util/setup_server.hh"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_domainbrowser_args.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_server_args.hh"


using namespace std;

const string server_name = "fred-dbifd";

//config args processing
HandlerPtrVector global_hpv =
        boost::assign::list_of
                // clang-format off
                (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
                (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
                (HandleArgsPtr(new HandleServerArgs))
                (HandleArgsPtr(new HandleLoggingArgs))
                (HandleArgsPtr(new HandleDatabaseArgs))
                (HandleArgsPtr(new HandleCorbaNameServiceArgs))
                (HandleArgsPtr(new HandleRegistryArgs))
                (HandleArgsPtr(new HandleMojeIdArgs))
                (HandleArgsPtr(new HandleDomainBrowserArgs));
                // clang-format on

int main(int argc, char* argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    { //config
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        setup_logging(CfgArgs::instance());

        Logging::Context ctx(server_name);

        const DestroyCorbaContainerInMyDestructor prevent_usage_of_destroyed_logging_singleton;

        // config dump
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump)
        {
            for (std::string config_item = AccumulatedConfig::get_instance().pop_front();
                    !config_item.empty();
                    config_item = AccumulatedConfig::get_instance().pop_front())
            {
                Logging::Manager::instance_ref().debug(config_item);
            }
        }

        //CORBA init
        corba_init();

        //MojeId registrar used for updates in domain browser
        std::string update_registrar_handle = CfgArgs::instance()
                                                      ->get_handler_ptr_by_type<HandleMojeIdArgs>()
                                                      ->registrar_handle;

        //domain list chunk size
        unsigned int domain_list_limit = CfgArgs::instance()
                                                 ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()
                                                 ->domain_list_limit;

        //nsset list chunk size
        unsigned int nsset_list_limit = CfgArgs::instance()
                                                ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()
                                                ->nsset_list_limit;

        //keyset list chunk size
        unsigned int keyset_list_limit = CfgArgs::instance()
                                                 ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()
                                                 ->keyset_list_limit;

        //contact list chunk size
        unsigned int contact_list_limit = CfgArgs::instance()
                                                  ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()
                                                  ->contact_list_limit;

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()->register_server(new CorbaConversion::DomainBrowser::Server_i(
                                                                server_name,
                                                                update_registrar_handle,
                                                                domain_list_limit,
                                                                nsset_list_limit,
                                                                keyset_list_limit,
                                                                contact_list_limit),
                "DomainBrowser");
        run_server(CfgArgs::instance(), CorbaContainer::get_instance());

    } //try
    catch (CORBA::TRANSIENT&)
    {
        cerr << "Caught system exception TRANSIENT -- unable to contact the "
             << "server." << endl;
        return EXIT_FAILURE;
    }
    catch (CORBA::SystemException& ex)
    {
        cerr << "Caught a CORBA::" << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch (CORBA::Exception& ex)
    {
        cerr << "Caught CORBA::Exception: " << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch (omniORB::fatalException& fe)
    {
        cerr << "Caught omniORB::fatalException:" << endl;
        cerr << "  file: " << fe.file() << endl;
        cerr << "  line: " << fe.line() << endl;
        cerr << "  mesg: " << fe.errmsg() << endl;
        return EXIT_FAILURE;
    }

    catch (const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch (exception& ex)
    {
        cerr << "Error: " << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cerr << "Unknown Error" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
