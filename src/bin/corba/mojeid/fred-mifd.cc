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

#include "src/bin/corba/connection_releaser.hh"
#include "src/bin/corba/mojeid/server_i.hh"
#include "libfred/db_settings.hh"
#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_messenger_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/corba_wrapper.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"
#include "src/util/setup_server.hh"

#include <boost/assign/list_of.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>

const std::string server_name = "fred-mifd";

//config args processing
const HandlerPtrVector global_hpv =
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
                (HandleArgsPtr(new HandleContactVerificationArgs))
                (HandleArgsPtr(new HandleMessengerArgs));
                // clang-format on

int main(int argc, char* argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {
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

        corba_init();

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()->register_server(
                new Registry::MojeId::Server_i(server_name),
                Registry::MojeId::service_name);
        run_server(CfgArgs::instance(), CorbaContainer::get_instance());
        return EXIT_SUCCESS;
    }
    catch (const CORBA::TRANSIENT&)
    {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the server." << std::endl;
        LOGGER.error("Caught system exception TRANSIENT -- unable to contact the server");
        return EXIT_FAILURE;
    }
    catch (const CORBA::SystemException& e)
    {
        std::cerr << "Caught a CORBA::" << e._name() << std::endl;
        LOGGER.error("Caught a CORBA::" + std::string(e._name()));
        return EXIT_FAILURE;
    }
    catch (const CORBA::Exception& e)
    {
        std::cerr << "Caught CORBA::Exception: " << e._name() << std::endl;
        LOGGER.error("Caught CORBA::Exception: " + std::string(e._name()));
        return EXIT_FAILURE;
    }
    catch (const omniORB::fatalException& e)
    {
        std::cerr << "Caught omniORB::fatalException:" << std::endl
                  << "  file: " << e.file() << std::endl
                  << "  line: " << e.line() << std::endl
                  << "  mesg: " << e.errmsg() << std::endl;
        LOGGER.error("Caught omniORB::fatalException: "
                     "file: " + std::string(e.file()) + " "
                     "line: " + std::to_string(e.line()) + " "
                     "mesg: " + e.errmsg());
        return EXIT_FAILURE;
    }
    catch (const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        LOGGER.error("Error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Error" << std::endl;
        LOGGER.error("Unknown Error");
        return EXIT_FAILURE;
    }
}
