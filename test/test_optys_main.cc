#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOptysUtils

#include "config.h"

#include "test/setup/fixtures.hh"

#include "util/log/logger.hh"

#include "src/util/cfg/config_handler.hh"

#include "src/util/cfg/handle_tests_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"

#include <boost/assign/list_of.hpp>
// dynamic library version
#include <boost/test/unit_test.hpp>

#include <utility>

namespace Test {

struct handle_command_line_args
{
    HandlerPtrVector config_handlers;

    handle_command_line_args()
    {
        config_handlers =
            boost::assign::list_of
                (HandleArgsPtr(new HandleTestsArgs(CONFIG_FILE)))
                (HandleArgsPtr(new HandleServerArgs))
                (HandleArgsPtr(new HandleLoggingArgs))
                (HandleArgsPtr(new HandleDatabaseArgs))
                (HandleArgsPtr(new HandleAdminDatabaseArgs)).convert_to_container<HandlerPtrVector>();

        CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(
                boost::unit_test::framework::master_test_suite().argc,
                boost::unit_test::framework::master_test_suite().argv).copy_onlynospaces_args();
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

}//namespace Test

struct global_fixture
{
    Test::handle_command_line_args handle_admin_db_cmd_line_args;
    Test::create_db_template crete_db_template;

    global_fixture()
    {
        Test::setup_logging(CfgArgs::instance());
    }
};

BOOST_GLOBAL_FIXTURE(global_fixture);
