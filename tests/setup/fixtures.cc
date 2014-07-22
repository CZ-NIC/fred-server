#include "tests/setup/fixtures.h"
#include "util/cfg/config_handler_decl.h"

/** well, these includes are ugly
 * but there is no other way to get to the name of current test_case
 * needed for ~instantiate_db_template() to store post-test db copy
 */
#include <boost/test/framework.hpp>
#include <boost/test/unit_test_suite_impl.hpp>

namespace Test {
namespace Fixture {

    namespace po = boost::program_options;


    static void terminate_persistent_connections(const std::auto_ptr<Database::StandaloneConnection>& conn) {
        conn->exec("SELECT pg_terminate_backend(procpid) FROM pg_stat_activity WHERE usename='fred';");
    }

    static void force_copy_db(
        const std::string& src_name,
        const std::string& dst_name,
        const std::auto_ptr<Database::StandaloneConnection>& conn
    ) {
        terminate_persistent_connections(conn);
        conn->exec("DROP DATABASE IF EXISTS "+ dst_name +";");
        conn->exec("CREATE DATABASE "+ dst_name +" TEMPLATE "+ src_name +";");
    }

    static void force_drop_db(
        const std::string& db_name,
        const std::auto_ptr<Database::StandaloneConnection>& conn
    ) {
        terminate_persistent_connections(conn);
        conn->exec("DROP DATABASE IF EXISTS "+ db_name +";");
    }



    create_db_template::create_db_template() {
        force_copy_db(
            get_original_db_name(),
            get_db_template_name(),
            CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection()
        );
    }

    create_db_template::~create_db_template() {
        force_drop_db(
            get_db_template_name(),
            CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection()
        );
    }



    instantiate_db_template::instantiate_db_template() {
        force_copy_db(
            create_db_template::get_db_template_name(),
            get_original_db_name(),
            CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection()
        );
    }

    instantiate_db_template::~instantiate_db_template() {
        std::string log_db_name(get_original_db_name() + "_");
        log_db_name += boost::unit_test::framework::current_test_case().p_name;

        const std::auto_ptr<Database::StandaloneConnection> conn =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection();

        force_drop_db(
            log_db_name,
            conn
        );

        conn->exec("ALTER DATABASE "+ get_original_db_name() +" RENAME TO " + log_db_name );
    }




    boost::shared_ptr<po::options_description> HandleAdminDatabaseArgs::get_options_description() {

         boost::shared_ptr<po::options_description> db_opts(
             new po::options_description(
                 std::string("Admin database connection configuration"))
         );

         db_opts->add_options()
             (   "admin_database.name",
                 po::value<std::string>(),
                 "admin database name"
             )
             (   "admin_database.user",
                 po::value<std::string>(),
                 "admin database user name"
             )
             (   "admin_database.password",
                 po::value<std::string>(),
                 "admin database password"
             )
             (   "admin_database.host",
                 po::value<std::string>(),
                 "admin database hostname"
             )
             (   "admin_database.port",
                 po::value<unsigned int>(),
                 "admin database port number"
             )
             (   "admin_database.timeout",
                 po::value<unsigned int>(),
                 "admin database timeout"
             );

         return db_opts;
    }

    void HandleAdminDatabaseArgs::handle( int argc, char* argv[],  FakedArgs &fa) {
        po::variables_map vm;

        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        /* construct connection string */
        host = vm.count("admin_database.host") == 0
                ? ""
                : vm["admin_database.host"].as<std::string>();
        pass = vm.count("admin_database.password") == 0
                ? ""
                : vm["admin_database.password"].as<std::string>();
        dbname = vm.count("admin_database.name") == 0
                ? ""
                : vm["admin_database.name"].as<std::string>();
        user = vm.count("admin_database.user") == 0
                ? ""
                : vm["admin_database.user"].as<std::string>();
        port = vm.count("admin_database.port") == 0
                ? ""
                : boost::lexical_cast<std::string>(vm["admin_database.port"].as<unsigned>());
        timeout = vm.count("admin_database.timeout") == 0
                ? ""
                : boost::lexical_cast<std::string>(vm["admin_database.timeout"].as<unsigned>());

    }

    std::auto_ptr<Database::StandaloneConnection> HandleAdminDatabaseArgs::get_admin_connection() {
        return
            std::auto_ptr<Database::StandaloneConnection>(
                Database::StandaloneManager(
                    new Database::StandaloneConnectionFactory(
                        "host="+host
                        +" port="+port
                        +" dbname="+dbname
                        +" user="+user
                        +" password="+pass
                        +" connect_timeout="+timeout
                    )
                ).acquire()
            );
    }


}
}
