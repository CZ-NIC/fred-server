/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFredlib

// dynamic library version
#include <boost/test/unit_test.hpp>

#include "config.h"

#include "test/setup/fixtures.hh"

#include "src/util/cfg/config_handler_decl.hh"
// XXX just perverse. why the **** is there this second header with global object???
#include "src/util/cfg/config_handler.hh"

#include "src/util/cfg/handle_tests_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"

#include <boost/assign/list_of.hpp>
#include <utility>

namespace Test {

    struct handle_command_line_args {
        HandlerPtrVector config_handlers;

        handle_command_line_args() {

            config_handlers =
                boost::assign::list_of
                    (HandleArgsPtr(new HandleTestsArgs(CONFIG_FILE)))
                    (HandleArgsPtr(new HandleServerArgs))
                    (HandleArgsPtr(new HandleLoggingArgs))
                    (HandleArgsPtr(new HandleDatabaseArgs))
                    (HandleArgsPtr(new HandleThreadGroupArgs))
                    (HandleArgsPtr(new HandleRegistryArgs))
                    (HandleArgsPtr(new HandleRifdArgs))
                    (HandleArgsPtr(new HandleContactVerificationArgs))
                    (HandleArgsPtr(new HandleMojeIdArgs))
                    (HandleArgsPtr(new HandleAdminDatabaseArgs)).convert_to_container<HandlerPtrVector>();

            namespace boost_args_ns = boost::unit_test::framework;

            CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(
                boost_args_ns::master_test_suite().argc,
                boost_args_ns::master_test_suite().argv
            ).copy_onlynospaces_args();
        }
    };

    void setup_logging(CfgArgs * cfg_instance_ptr) {

        HandleLoggingArgs* handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>();

        Logging::Log::Type log_type =
            static_cast<Logging::Log::Type>(handler_ptr->log_type);

        boost::any param;
        if (log_type == Logging::Log::LT_FILE) {
            param = handler_ptr->log_file;
        }
        if (log_type == Logging::Log::LT_SYSLOG) {
            param = handler_ptr->log_syslog_facility;
        }

        Logging::Manager::instance_ref().addHandler(log_type, param);

        Logging::Manager::instance_ref().setLevel(
            static_cast<Logging::Log::Level>(handler_ptr->log_level)
        );
    }
}

struct global_fixture {
    Test::handle_command_line_args handle_admin_db_cmd_line_args;
    Test::create_db_template crete_db_template;

    global_fixture() {
        Test::setup_logging(CfgArgs::instance());
    }
};

BOOST_GLOBAL_FIXTURE( global_fixture );
