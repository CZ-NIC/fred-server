#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestPR

#include "config.h"

#include "test/setup/fixtures.hh"

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/config_handler.hh"
#include "src/util/corba_wrapper.hh"
#include "src/util/setup_server.hh"

#include "src/util/cfg/handle_tests_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>
#include <utility>

namespace Test
{

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
                (HandleArgsPtr(new HandleCorbaNameServiceArgs))
                (HandleArgsPtr(new HandleThreadGroupArgs))
                (HandleArgsPtr(new HandleRegistryArgs))
                (HandleArgsPtr(new HandleRifdArgs))
                (HandleArgsPtr(new HandleMojeIdArgs))
                (HandleArgsPtr(new HandleAdminDatabaseArgs)).convert_to_container<HandlerPtrVector>();

        namespace boost_args_ns = boost::unit_test::framework;

        CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(
            boost_args_ns::master_test_suite().argc,
            boost_args_ns::master_test_suite().argv
        ).copy_onlynospaces_args();
    }
};

} // namespace Test

struct global_fixture
{
    Test::handle_command_line_args handle_admin_db_cmd_line_args;
    Test::create_db_template create_db_template;
};

BOOST_GLOBAL_FIXTURE( global_fixture );
