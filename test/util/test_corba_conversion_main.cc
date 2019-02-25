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
#define BOOST_TEST_MODULE TestCorbaConversionUtils

// dynamic library version
#include <boost/test/unit_test.hpp>

#include "config.h"

#include "test/setup/fixtures.hh"

#include "src/util/cfg/config_handler.hh"

#include "src/util/cfg/handle_tests_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"

#include "util/log/logger.hh"

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
                    (HandleArgsPtr(new HandleAdminDatabaseArgs)).convert_to_container<HandlerPtrVector>();

            namespace boost_args_ns = boost::unit_test::framework;

            CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(
                boost_args_ns::master_test_suite().argc,
                boost_args_ns::master_test_suite().argv
            ).copy_onlynospaces_args();
        }
    };

    void setup_logging(CfgArgs* cfg_instance_ptr)
    {
        HandleLoggingArgs* const handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>();

        const auto log_type = static_cast<unsigned>(handler_ptr->log_type);
        Logging::Log::EventImportance min_importance = Logging::Log::EventImportance::trace;
        if ((log_type == 0) || (log_type == 1))
        {
            switch (handler_ptr->log_level)
            {
                case 0:
                    min_importance = Logging::Log::EventImportance::emerg;
                    break;
                case 1:
                    min_importance = Logging::Log::EventImportance::alert;
                    break;
                case 2:
                    min_importance = Logging::Log::EventImportance::crit;
                    break;
                case 3:
                    min_importance = Logging::Log::EventImportance::err;
                    break;
                case 4:
                    min_importance = Logging::Log::EventImportance::warning;
                    break;
                case 5:
                    min_importance = Logging::Log::EventImportance::notice;
                    break;
                case 6:
                    min_importance = Logging::Log::EventImportance::info;
                    break;
                case 7:
                    min_importance = Logging::Log::EventImportance::debug;
                    break;
                case 8:
                    min_importance = Logging::Log::EventImportance::trace;
                    break;
            }
        }

        switch (log_type)
        {
            case 0:
                LOGGER.add_handler_of<Logging::Log::Device::console>(min_importance);
                break;
            case 1:
                LOGGER.add_handler_of<Logging::Log::Device::file>(handler_ptr->log_file, min_importance);
                break;
            case 2:
                LOGGER.add_handler_of<Logging::Log::Device::syslog>(handler_ptr->log_syslog_facility);
                break;
        }
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
