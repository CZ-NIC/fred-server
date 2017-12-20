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
#define BOOST_TEST_MODULE TestPasswordStorageUtils

#include "config.h"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_tests_args.hh"
#include "src/util/cfg/handle_logging_args.hh"

#include "src/util/log/logger.hh"

// dynamic library version
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

namespace Test {

struct handle_command_line_args
{
    handle_command_line_args()
        : config_handlers(boost::assign::list_of(HandleArgsPtr(new HandleTestsArgs(CONFIG_FILE)))
                                                (HandleArgsPtr(new HandleLoggingArgs))
            .convert_to_container<HandlerPtrVector>())
    {
        CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(
                boost::unit_test::framework::master_test_suite().argc,
                boost::unit_test::framework::master_test_suite().argv).copy_onlynospaces_args();
    }
    HandlerPtrVector config_handlers;
};

void setup_logging(CfgArgs* cfg_instance_ptr)
{
    HandleLoggingArgs* const handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>();

    const Logging::Log::Type log_type =
        static_cast<Logging::Log::Type>(handler_ptr->log_type);

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

    Logging::Manager::instance_ref().get(PACKAGE).setLevel(
            static_cast<Logging::Log::Level>(handler_ptr->log_level));
}

} // namespace Test

struct global_fixture
{
    global_fixture()
    {
        Test::setup_logging(CfgArgs::instance());
    }
    Test::handle_command_line_args handle_cmd_line_args;
};

BOOST_GLOBAL_FIXTURE(global_fixture);
