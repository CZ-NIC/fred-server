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
 *  @fred-rifd.cc
 *  implementation of registrar interface
 */

#include "src/libfred/db_settings.hh"
#include "src/util/corba_wrapper.hh"
#include "src/util/log/logger.hh"
#include "src/util/log/context.hh"
#include "src/bin/corba/connection_releaser.hh"
#include "src/util/setup_server.hh"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"

#include "config.h"
#include "src/bin/corba/EPP.hh"
#include "src/bin/corba/TechCheck.hh"
#include "src/bin/corba/epp/epp_impl.hh"
#include "src/bin/corba/mailer_manager.hh"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

const std::string server_name = "fred-rifd";

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE)))
    (HandleArgsPtr(new HandleServerArgs))
    (HandleArgsPtr(new HandleLoggingArgs))
    (HandleArgsPtr(new HandleDatabaseArgs))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs))
    (HandleArgsPtr(new HandleRegistryArgs))
    (HandleArgsPtr(new HandleRifdArgs));

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
                Logging::Manager::instance_ref().get(PACKAGE).debug(config_item);
            }
        }

        //CORBA init
        corba_init();

        MailerManager mm(CorbaContainer::get_instance()->getNS());

        //conf pointers
        HandleDatabaseArgs* const db_args_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>();
        HandleRegistryArgs* const registry_args_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleRifdArgs* const rifd_args_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleRifdArgs>();

        auto myccReg_EPP_i = std::make_unique<ccReg_EPP_i>(
                db_args_ptr->get_conn_info(),
                &mm,
                CorbaContainer::get_instance()->getNS(),
                registry_args_ptr->restricted_handles,
                registry_args_ptr->disable_epp_notifier,
                registry_args_ptr->lock_epp_commands,
                registry_args_ptr->nsset_level,
                registry_args_ptr->nsset_min_hosts,
                registry_args_ptr->nsset_max_hosts,
                registry_args_ptr->docgen_path,
                registry_args_ptr->docgen_template_path,
                registry_args_ptr->fileclient_path,
                registry_args_ptr->disable_epp_notifier_cltrid_prefix,
                rifd_args_ptr->rifd_session_max,
                rifd_args_ptr->rifd_session_timeout,
                rifd_args_ptr->rifd_session_registrar_max,
                rifd_args_ptr->rifd_epp_update_domain_keyset_clear,
                rifd_args_ptr->rifd_epp_operations_charging,
                rifd_args_ptr->epp_update_contact_enqueue_check,
                rifd_args_ptr->rifd_check);

        // create session use values from config
        LOGGER(PACKAGE).info(boost::format(
                "sessions max: %1%; timeout: %2%")
                % rifd_args_ptr->rifd_session_max
                % rifd_args_ptr->rifd_session_timeout);

        ccReg::timestamp_var ts;
        char* const version = myccReg_EPP_i->version(ts);
        LOGGER(PACKAGE).info(boost::format("RIFD server version: %1% (%2%)")
                                              % version
                                              % ts);
        CORBA::string_free(version);

        // load error messages to memory
        if (myccReg_EPP_i->LoadErrorMessages() <= 0)
        {
            LOGGER(PACKAGE).alert("database error: load error messages");
            std::exit(-6);
        }

        // load reason messages to memory
        if (myccReg_EPP_i->LoadReasonMessages() <= 0)
        {
            LOGGER(PACKAGE).alert("database error: load reason messages" );
            std::exit(-7);
        }

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()->register_server(myccReg_EPP_i.release(), "EPP");

        run_server(CfgArgs::instance(), CorbaContainer::get_instance());
        return EXIT_SUCCESS;
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
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }
}
