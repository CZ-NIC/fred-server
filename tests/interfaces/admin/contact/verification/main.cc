#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAdminContactVerification

// dynamic library version
#include <boost/test/unit_test.hpp>

#include "config.h"

#include "tests/setup/fixtures.h"

#include "util/cfg/config_handler_decl.h"
#include "util/cfg/config_handler.h"

#include "util/cfg/handle_tests_args.h"
#include "util/cfg/handle_server_args.h"
#include "util/cfg/handle_logging_args.h"
#include "util/cfg/handle_database_args.h"
#include "util/cfg/handle_corbanameservice_args.h"
#include "util/cfg/handle_threadgroup_args.h"
#include "util/cfg/handle_registry_args.h"
#include "util/cfg/handle_rifd_args.h"
#include "util/cfg/handle_contactverification_args.h"
#include "util/cfg/handle_mojeid_args.h"

#include <boost/assign/list_of.hpp>

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
                    (HandleArgsPtr(new HandleCorbaNameServiceArgs))
                    (HandleArgsPtr(new HandleThreadGroupArgs))
                    (HandleArgsPtr(new HandleRegistryArgs))
                    (HandleArgsPtr(new HandleRifdArgs))
                    (HandleArgsPtr(new HandleContactVerificationArgs))
                    (HandleArgsPtr(new HandleMojeIDArgs))
                    (HandleArgsPtr(new Fixture::HandleAdminDatabaseArgs)).convert_to_container<HandlerPtrVector>();

            namespace boost_args_ns = boost::unit_test::framework;

            CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(
                boost_args_ns::master_test_suite().argc,
                boost_args_ns::master_test_suite().argv
            ).copy_onlynospaces_args();
        }
    };
}

struct global_fixture {
    Test::handle_command_line_args handle_admin_db_cmd_line_args;
    Test::Fixture::create_db_template crete_db_template;
};

BOOST_GLOBAL_FIXTURE( global_fixture );
