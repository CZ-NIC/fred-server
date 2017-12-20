/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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
 *  @file
 *  implementation of registry record statement server
 */


#include "src/bin/corba/connection_releaser.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/corba/record_statement/record_statement_i.hh"
#include "src/libfred/documents.hh"
#include "src/libfred/db_settings.hh"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

#include "src/util/corba_wrapper.hh"
#include "src/util/log/logger.hh"
#include "src/util/log/context.hh"
#include "src/util/setup_server.hh"

#include <boost/assign/list_of.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>


const std::string server_name = "fred-rsifd";

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
;


int main(int argc, char *argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        setup_logging(CfgArgs::instance());

        //CORBA init
        corba_init();

        boost::shared_ptr<LibFred::Mailer::Manager> mailer_manager(
            new MailerManager(CorbaContainer::get_instance()->getNS()));


        boost::shared_ptr<LibFred::Document::Manager>  doc_manager(
            LibFred::Document::Manager::create(
                CfgArgs::instance()->get_handler_ptr_by_type< HandleRegistryArgs >()->docgen_path,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->docgen_template_path,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->fileclient_path,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()->get_nameservice_host_port()
            ).release());



        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()
            ->register_server(new Registry::RecordStatement::Server_i(server_name, doc_manager, mailer_manager,
                    CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->registry_timezone)
            , "RecordStatement");
        run_server(CfgArgs::instance(), CorbaContainer::get_instance());

    }//try
    catch(const CORBA::TRANSIENT&)
    {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the "
             << "server." << std::endl;
        return EXIT_FAILURE;
    }
    catch(const CORBA::SystemException& ex)
    {
        std::cerr << "Caught a CORBA::" << ex._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch(const CORBA::Exception& ex)
    {
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch(const omniORB::fatalException& fe)
    {
        std::cerr << "Caught omniORB::fatalException:" << std::endl;
        std::cerr << "  file: " << fe.file() << std::endl;
        std::cerr << "  line: " << fe.line() << std::endl;
        std::cerr << "  mesg: " << fe.errmsg() << std::endl;
        return EXIT_FAILURE;
    }

    catch(const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch(std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
