#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAutoMergeContact

// dynamic library version
#include <boost/test/unit_test.hpp>

#include "config.h"

#include "tests/setup/fixtures.h"

#include "util/cfg/config_handler_decl.h"
#include "util/cfg/config_handler.h"
#include "util/corba_wrapper.h"
#include "setup_server.h"

#include "util/cfg/handle_tests_args.h"
#include "util/cfg/handle_server_args.h"
#include "util/cfg/handle_logging_args.h"
#include "util/cfg/handle_database_args.h"
#include "util/cfg/handle_corbanameservice_args.h"
#include "util/cfg/handle_threadgroup_args.h"
#include "util/cfg/handle_registry_args.h"
#include "util/cfg/handle_rifd_args.h"
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
                    (HandleArgsPtr(new HandleMojeIDArgs))
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

        Logging::Manager::instance_ref().get(PACKAGE).addHandler(log_type, param);

        Logging::Manager::instance_ref().get(PACKAGE).setLevel(
            static_cast<Logging::Log::Level>(handler_ptr->log_level)
        );
    }

} // namespace Test

struct global_fixture {
    Test::handle_command_line_args handle_admin_db_cmd_line_args;
    Test::create_db_template crete_db_template;

    global_fixture() {
        Test::setup_logging(CfgArgs::instance());
    }
};

BOOST_GLOBAL_FIXTURE( global_fixture );
