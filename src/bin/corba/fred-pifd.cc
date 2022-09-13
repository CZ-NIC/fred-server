/*
 * Copyright (C) 2011-2022  CZ.NIC, z. s. p. o.
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
 *  @fred-pifd.cc
 *  implementation of public interface
 */

#include "config.h"
#include "src/bin/corba/contact_verification/contact_verification_i.hh"
#include "src/bin/corba/public_request/server_i.hh"
#include "src/bin/corba/whois/whois2_impl.hh"
#include "src/bin/corba/whois/whois_impl.hh"

#include "src/bin/corba/connection_releaser.hh"
#include "libfred/db_settings.hh"
#include "src/util/corba_wrapper.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"
#include "src/util/setup_server.hh"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_server_args.hh"

#include <boost/assign/list_of.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

const std::string server_name = "fred-pifd";

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
                (HandleArgsPtr(new HandleContactVerificationArgs));
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

        //conf pointers
        HandleDatabaseArgs* db_args_ptr = CfgArgs::instance()
                                                  ->get_handler_ptr_by_type<HandleDatabaseArgs>();
        HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
                                                        ->get_handler_ptr_by_type<HandleRegistryArgs>();

        std::unique_ptr<ccReg_Whois_i> myccReg_Whois_i(new ccReg_Whois_i(
                db_args_ptr->get_conn_info(),
                server_name,
                registry_args_ptr->restricted_handles));

        std::unique_ptr<CorbaConversion::Whois::Server_impl> my_whois2(new CorbaConversion::Whois::Server_impl("Whois2"));

        auto my_public_request = std::make_unique<CorbaConversion::PublicRequest::Server_i>(
                server_name,
                registry_args_ptr->system_registrar);

        std::unique_ptr<CorbaConversion::Contact::Verification::ContactVerification_i> contact_vrf_iface(
                new CorbaConversion::Contact::Verification::ContactVerification_i("fred-pifd-cv"));

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()
                ->register_server(myccReg_Whois_i.release(), "Whois");

        CorbaContainer::get_instance()
                ->register_server(my_whois2.release(), "Whois2");

        CorbaContainer::get_instance()
                ->register_server(my_public_request.release(), "PublicRequest");

        CorbaContainer::get_instance()
                ->register_server(contact_vrf_iface.release(), "ContactVerification");

        run_server(CfgArgs::instance(), CorbaContainer::get_instance());
        return EXIT_SUCCESS;
    } //try
    catch (const CORBA::TRANSIENT&)
    {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the server." << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::SystemException& ex)
    {
        std::cerr << "Caught a CORBA::" << ex._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::Exception& ex)
    {
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const omniORB::fatalException& fe)
    {
        std::cerr << "Caught omniORB::fatalException:" << std::endl
                  << "  file: " << fe.file() << std::endl
                  << "  line: " << fe.line() << std::endl
                  << "  mesg: " << fe.errmsg() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }
}
