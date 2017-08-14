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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAutomaticKeysetManagement

#include "config.h"

#include "tests/setup/fixtures.h"

#include "util/corba_wrapper.h"

#include "util/cfg/config_handler_decl.h"
#include "util/cfg/config_handler.h"
#include "util/cfg/handle_tests_args.h"
#include "util/cfg/handle_server_args.h"
#include "util/cfg/handle_logging_args.h"
#include "util/cfg/handle_database_args.h"
#include "util/cfg/handle_corbanameservice_args.h"
#include "util/cfg/handle_threadgroup_args.h"
#include "util/cfg/handle_registry_args.h"
#include "util/cfg/handle_adifd_args.h"
#include "util/cfg/handle_akmd_args.h"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

class handle_command_line_args
{
public:
    handle_command_line_args()
    :   config_handlers(
            static_cast< const HandlerPtrVector& >(
                boost::assign::list_of(HandleArgsPtr(new HandleTestsArgs(CONFIG_FILE)))
                                      (HandleArgsPtr(new HandleServerArgs))
                                      (HandleArgsPtr(new HandleLoggingArgs))
                                      (HandleArgsPtr(new HandleDatabaseArgs))
                                      (HandleArgsPtr(new HandleCorbaNameServiceArgs))
                                      (HandleArgsPtr(new HandleThreadGroupArgs))
                                      (HandleArgsPtr(new HandleRegistryArgs))
                                      (HandleArgsPtr(new HandleAkmdArgs))
                                      .convert_to_container<HandlerPtrVector>()))
    {
        CfgArgs::init< HandleTestsArgs >(config_handlers)->handle(
            boost::unit_test::framework::master_test_suite().argc,
            boost::unit_test::framework::master_test_suite().argv);
        setup_logging(CfgArgs::instance());
    }
private:
    static void setup_logging(CfgArgs *cfg_instance_ptr);
    const HandlerPtrVector config_handlers;
};

void handle_command_line_args::setup_logging(CfgArgs *cfg_instance_ptr)
{
    const HandleLoggingArgs* const handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type< HandleLoggingArgs >();

    const Logging::Log::Type log_type = static_cast< Logging::Log::Type >(handler_ptr->log_type);
    const Logging::Log::Level log_level = static_cast< Logging::Log::Level >(handler_ptr->log_level);

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

class global_fixture
{
    Test::handle_command_line_args handle_admin_db_cmd_line_args;
    Test::Fixture::create_db_template create_db_template;
};

BOOST_GLOBAL_FIXTURE(global_fixture);
