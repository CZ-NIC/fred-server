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

#include "config.h"
#include "EPP.hh"
#include "TechCheck.hh"
#include "epp/epp_impl.h"
#include "corba/mailer_manager.h"

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

#include "fredlib/db_settings.h"
#include "corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"
#include "corba/connection_releaser.h"
#include "setup_server.h"

#include "cfg/config_handler.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_rifd_args.h"

using namespace std;

const string server_name = "fred-rifd";

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
    (HandleArgsPtr(new HandleRifdArgs))
    ;

int main(int argc, char *argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::instance<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        setup_logging(CfgArgs::instance());

        //CORBA init
        corba_init();

        MailerManager mm(CorbaContainer::get_instance()->getNS());


        //conf pointers
        HandleDatabaseArgs* db_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleDatabaseArgs>();
        HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleRifdArgs* rifd_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleRifdArgs>();

        std::auto_ptr<ccReg_EPP_i> myccReg_EPP_i ( new ccReg_EPP_i(
                    db_args_ptr->get_conn_info()
                    , &mm, CorbaContainer::get_instance()->getNS()
                    , registry_args_ptr->restricted_handles
                    , registry_args_ptr->disable_epp_notifier
                    , registry_args_ptr->lock_epp_commands
                    , registry_args_ptr->nsset_level
                    , registry_args_ptr->docgen_path
                    , registry_args_ptr->docgen_template_path
                    , registry_args_ptr->fileclient_path
                    , registry_args_ptr->disable_epp_notifier_cltrid_prefix
                    , rifd_args_ptr->rifd_session_max
                    , rifd_args_ptr->rifd_session_timeout
                    , rifd_args_ptr->rifd_session_registrar_max
                    , rifd_args_ptr->rifd_epp_update_domain_keyset_clear
            ));

            // create session use values from config
            LOGGER(PACKAGE).info(boost::format(
                    "sessions max: %1%; timeout: %2%")
                    % rifd_args_ptr->rifd_session_max
                    % rifd_args_ptr->rifd_session_timeout);

            ccReg::timestamp_var ts;
            char *version = myccReg_EPP_i->version(ts);
            LOGGER(PACKAGE).info(boost::format("RIFD server version: %1% (%2%)")
                                                  % version
                                                  % ts);
            CORBA::string_free(version);

            // load all country code from table enum_country
            if (myccReg_EPP_i->LoadCountryCode() <= 0) {
              LOGGER(PACKAGE).alert("database error: load country code");
              exit(-5);
            }

            // load error messages to memory
            if (myccReg_EPP_i->LoadErrorMessages() <= 0) {
              LOGGER(PACKAGE).alert("database error: load error messages");
              exit(-6);
            }

            // load reason messages to memory
            if (myccReg_EPP_i->LoadReasonMessages() <= 0) {
              LOGGER(PACKAGE).alert("database error: load reason messages" );
              exit(-7);
            }

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()
            ->register_server(myccReg_EPP_i.release(), "EPP");

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
