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
 *  @fred-pifd.cc
 *  implementation of public interface
 */

#include "config.h"
#include "src/corba/Admin.hh"
#include "src/corba/admin/admin_impl.h"
#include "src/corba/whois/whois_impl.h"
#include "src/corba/whois/whois2_impl.h"
#include "src/corba/public_request/server_i.h"
#include "src/corba/contact_verification/contact_verification_i.h"

#include "src/fredlib/db_settings.h"
#include "util/corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"
#include "src/corba/connection_releaser.h"
#include "setup_server.h"

#include "cfg/config_handler.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_adifd_args.h"
#include "cfg/handle_contactverification_args.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdlib>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>


const std::string server_name = "fred-pifd";

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
    (HandleArgsPtr(new HandleAdifdArgs))
    (HandleArgsPtr(new HandleContactVerificationArgs));

int main(int argc, char *argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        setup_logging(CfgArgs::instance());

        Logging::Context ctx(server_name);

        //config dump
        for(std::string config_item = AccumulatedConfig::get_instance().pop_front();
            !config_item.empty(); config_item = AccumulatedConfig::get_instance().pop_front())
        {
            Logging::Manager::instance_ref().get(PACKAGE).debug(config_item);
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

        std::auto_ptr<ccReg_Admin_i> myccReg_Admin_i(new ccReg_Admin_i(
                    db_args_ptr->get_conn_info(),
                    CorbaContainer::get_instance()->getNS(),
                    registry_args_ptr->restricted_handles,
                    registry_args_ptr->docgen_path,
                    registry_args_ptr->docgen_template_path,
                    registry_args_ptr->docgen_domain_count_limit,
                    registry_args_ptr->fileclient_path,
                    adifd_args_ptr->adifd_session_max,
                    adifd_args_ptr->adifd_session_timeout,
                    adifd_args_ptr->adifd_session_garbage,
                    false));

        std::auto_ptr<ccReg_Whois_i> myccReg_Whois_i(new ccReg_Whois_i(
                db_args_ptr->get_conn_info(),
                server_name,
                registry_args_ptr->restricted_handles));

        std::auto_ptr<Registry::Whois::Server_impl> my_whois2(new Registry::Whois::Server_impl("Whois2"));

        std::auto_ptr<Registry::PublicRequest::Server_i> my_public_request(
                new Registry::PublicRequest::Server_i(server_name));

        std::auto_ptr<Registry::Contact::Verification::ContactVerification_i> contact_vrf_iface(
                new Registry::Contact::Verification::ContactVerification_i("fred-pifd-cv"));

            // create session use values from config
            LOGGER(PACKAGE).info(boost::format(
                    "sessions max: %1%; timeout: %2%")
                    % adifd_args_ptr->adifd_session_max
                    % adifd_args_ptr->adifd_session_timeout);

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()
            ->register_server(myccReg_Admin_i.release(), "WebWhois");

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
    }//try
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
