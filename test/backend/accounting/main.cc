/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#define BOOST_TEST_MODULE TestAccounting

#include "config.h"

#include "test/setup/fixtures.hh"

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_tests_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_registry_args.hh"

#include <boost/assign/list_of.hpp>
#include <boost/test/framework.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

class HandleCommandLineArgs
{
public:
    HandleCommandLineArgs()
        : config_handlers(
                  static_cast<const HandlerPtrVector&>(
                          boost::assign::list_of
                                (HandleArgsPtr(new HandleTestsArgs(CONFIG_FILE)))
                                (HandleArgsPtr(new HandleServerArgs))
                                (HandleArgsPtr(new HandleLoggingArgs))
                                (HandleArgsPtr(new HandleDatabaseArgs))
                                (HandleArgsPtr(new HandleCorbaNameServiceArgs))
                                (HandleArgsPtr(new HandleThreadGroupArgs))
                                (HandleArgsPtr(new HandleRegistryArgs))
                                (HandleArgsPtr(new HandleAdminDatabaseArgs))
                                .convert_to_container<HandlerPtrVector>()))
    {
        CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(
                boost::unit_test::framework::master_test_suite().argc,
                boost::unit_test::framework::master_test_suite().argv);
        setup_logging(CfgArgs::instance());
    }
private:
    static void setup_logging(CfgArgs *cfg_instance_ptr);
    const HandlerPtrVector config_handlers;
};

void HandleCommandLineArgs::setup_logging(CfgArgs* cfg_instance_ptr)
{
    HandleLoggingArgs* const handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>();

    const auto log_type = static_cast<unsigned>(handler_ptr->log_type);
    Logging::Log::Severity min_severity = Logging::Log::Severity::trace;
    switch (handler_ptr->log_level)
    {
        case 0:
            min_severity = Logging::Log::Severity::emerg;
            break;
        case 1:
            min_severity = Logging::Log::Severity::alert;
            break;
        case 2:
            min_severity = Logging::Log::Severity::crit;
            break;
        case 3:
            min_severity = Logging::Log::Severity::err;
            break;
        case 4:
            min_severity = Logging::Log::Severity::warning;
            break;
        case 5:
            min_severity = Logging::Log::Severity::notice;
            break;
        case 6:
            min_severity = Logging::Log::Severity::info;
            break;
        case 7:
            min_severity = Logging::Log::Severity::debug;
            break;
        case 8:
            min_severity = Logging::Log::Severity::trace;
            break;
    }

    switch (log_type)
    {
        case 0:
            LOGGER.add_handler_of<Logging::Log::Device::console>(min_severity);
            break;
        case 1:
            LOGGER.add_handler_of<Logging::Log::Device::file>(handler_ptr->log_file, min_severity);
            break;
        case 2:
            LOGGER.add_handler_of<Logging::Log::Device::syslog>(handler_ptr->log_syslog_facility, min_severity);
            break;
    }
}

} // namespace Test

class GlobalFixture
{
    Test::HandleCommandLineArgs handle_cmd_line_args;
    Test::create_db_template create_db_template;
};

BOOST_GLOBAL_FIXTURE(GlobalFixture);
