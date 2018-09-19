/*
 * Copyright (C) 2018  CZ.NIC, z.s.P.o.
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
 * along with FRED.  If not, see <http://www.gnu.or/licenses/>.
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

void HandleCommandLineArgs::setup_logging(CfgArgs *cfg_instance_ptr)
{
    const HandleLoggingArgs* const handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>();

    const Logging::Log::Type log_type = static_cast<Logging::Log::Type>(handler_ptr->log_type);
    const Logging::Log::Level log_level = static_cast<Logging::Log::Level>(handler_ptr->log_level);

    boost::any param;
    switch (log_type)
    {
        case Logging::Log::LT_FILE:
            param = handler_ptr->log_file;
            break;
        case Logging::Log::LT_SYSLOG:
            param = handler_ptr->log_syslog_facility;
            break;
        case Logging::Log::LT_CONSOLE:
            break;
    }

    Logging::Manager::instance_ref().get(PACKAGE).addHandler(log_type, param);
    Logging::Manager::instance_ref().get(PACKAGE).setLevel(log_level);
}

} // namespace Test

class GlobalFixture
{
    Test::HandleCommandLineArgs handle_cmd_line_args;
    Test::create_db_template create_db_template;
};

BOOST_GLOBAL_FIXTURE(GlobalFixture);
