/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
 *  @file fred-akmd.cc
 *  implementation of automatic keyset management
 */

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_akmd_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"
#include "src/bin/corba/automatic_keyset_management/server_i.hh"
#include "src/bin/corba/connection_releaser.hh"
#include "src/bin/corba/logger_client_impl.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/documents.hh"
#include "src/util/dummy_logger.hh"
#include "src/util/corba_wrapper.hh"
#include "src/util/setup_server.hh"

#include <boost/assign/list_of.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

const std::string server_name = "fred-akmd";

// config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
    (HandleArgsPtr(new HandleServerArgs))
    (HandleArgsPtr(new HandleLoggingArgs))
    (HandleArgsPtr(new HandleDatabaseArgs))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs))
    (HandleArgsPtr(new HandleAkmdArgs))
;

int main(int argc, char* argv[])
{
    FakedArgs fa; // producing faked args with unrecognized ones
    try
    {
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        setup_logging(CfgArgs::instance());

        corba_init();

        boost::scoped_ptr<LibFred::Logger::LoggerClient> logger_client;
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleAkmdArgs>()->enable_request_logger)
        {
            logger_client.reset(new LibFred::Logger::LoggerCorbaClientImpl());
        }
        else
        {
            logger_client.reset(new LibFred::Logger::DummyLoggerImpl());
        }

        // create server object with poa and nameservice registration
        CorbaContainer::get_instance()->register_server(
                new Registry::AutomaticKeysetManagement::Server_i(
                        server_name,
                        CfgArgs::instance()->get_handler_ptr_by_type<HandleAkmdArgs>()->automatically_managed_keyset_prefix,
                        CfgArgs::instance()->get_handler_ptr_by_type<HandleAkmdArgs>()->automatically_managed_keyset_registrar,
                        CfgArgs::instance()->get_handler_ptr_by_type<HandleAkmdArgs>()->automatically_managed_keyset_tech_contact,
                        CfgArgs::instance()->get_handler_ptr_by_type<HandleAkmdArgs>()->automatically_managed_keyset_zones,
                        CfgArgs::instance()->get_handler_ptr_by_type<HandleAkmdArgs>()->disable_notifier,
                        *logger_client),
                "AutomaticKeysetManagement");
        run_server(CfgArgs::instance(), CorbaContainer::get_instance());
    }
    catch (const CORBA::TRANSIENT&)
    {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the server." << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::SystemException& e)
    {
        std::cerr << "Caught a CORBA::" << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::Exception& e)
    {
        std::cerr << "Caught CORBA::Exception: " << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const omniORB::fatalException& e)
    {
        std::cerr << "Caught omniORB::fatalException:" << std::endl;
        std::cerr << "  file: " << e.file() << std::endl;
        std::cerr << "  line: " << e.line() << std::endl;
        std::cerr << "  mesg: " << e.errmsg() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
