#include "test/setup/fixtures.hh"
#include "src/util/cfg/config_handler_decl.hh"

#include <boost/lexical_cast.hpp>

/** well, these includes are ugly
 * but there is no other way to get to the name of current test_case
 * needed for ~instantiate_db_template() to store post-test db copy
 */
#include <boost/test/framework.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <utility>

namespace Test {

namespace po = boost::program_options;

namespace {

const unsigned max_postgresql_database_name_length = 63;

void disable_connections_add_terminate_persistent_connections(
        const std::unique_ptr<Database::StandaloneConnection>& conn,
        const std::string& database_name_disable_connection)
{

    conn->exec_params("UPDATE pg_database SET datallowconn = false WHERE datname = $1::text"
        , Database::query_param_list (database_name_disable_connection));

    std::string pid_column_name;
    {
        Database::Result dbres = conn->exec("SELECT column_name "
                                            "FROM information_schema.columns "
                                            "WHERE table_name = 'pg_stat_activity' AND "
                                                  "column_name IN ('procpid', 'pid')");
        if (dbres.size() <= 0) {
            std::runtime_error("table 'pg_stat_activity' contains neither procpid nor pid column");
        }
        pid_column_name = static_cast< std::string >(dbres[0][0]);
    }
    const std::string query = "SELECT " + pid_column_name + ", pg_terminate_backend(" + pid_column_name + ") "
                              "FROM pg_stat_activity "
                              "WHERE usename = 'fred' OR (datname = $1::text AND "
                                                          + pid_column_name + " != pg_backend_pid())";
    while (conn->exec_params(query, Database::query_param_list(database_name_disable_connection)).size() != 0) {}
}

void enable_connections(const std::unique_ptr<Database::StandaloneConnection>& conn, const std::string& database_name)
{
    conn->exec_params("UPDATE pg_database SET datallowconn = true WHERE datname = $1::text"
        , Database::query_param_list (database_name));
}

void check_dbname_length(const std::string& db_name)
{
    if(db_name.length() > max_postgresql_database_name_length)
    {
        throw std::runtime_error("db_name.length(): " + boost::lexical_cast<std::string>(db_name.length()) + " > "
                "max_postgresql_database_name_length: " + boost::lexical_cast<std::string>(max_postgresql_database_name_length) + " "
                "db_name: " + db_name);
    }
}

void force_drop_db(
        const std::string& db_name,
        const std::unique_ptr<Database::StandaloneConnection>& conn)
{
    check_dbname_length(db_name);
    disable_connections_add_terminate_persistent_connections(conn, db_name);
    conn->exec("DROP DATABASE IF EXISTS \"" + db_name + "\"");
}

void force_copy_db(
        const std::string& src_name,
        const std::string& dst_name,
        const std::unique_ptr<Database::StandaloneConnection>& conn)
{
    check_dbname_length(src_name);
    check_dbname_length(dst_name);
    force_drop_db(dst_name, conn);
    conn->exec("CREATE DATABASE \""+ dst_name +"\" TEMPLATE \""+ src_name +"\"");
}

}//namespace Test::{anonymous}

create_db_template::create_db_template()
{
    check_dbname_length(get_original_db_name());
    check_dbname_length(get_db_template_name());
    force_copy_db(
        get_original_db_name(),
        get_db_template_name(),
        CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection());
}

create_db_template::~create_db_template()
{
    // restore original db for the last time
    force_copy_db(
        get_db_template_name(),
        get_original_db_name(),
        CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection());
    force_drop_db(
        get_db_template_name(),
        CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection());
}

namespace {

std::string get_unit_test_path(const boost::unit_test::test_unit &tu,
                               const std::string &delimiter = "/")
{
    using namespace boost::unit_test;
    const bool has_no_parent = (tu.p_parent_id < MIN_TEST_SUITE_ID) || (tu.p_parent_id == INV_TEST_UNIT_ID);
    if (has_no_parent)
    {
        return static_cast< std::string >(tu.p_name);
    }
    return get_unit_test_path(framework::get<test_suite>(tu.p_parent_id), delimiter) +
           delimiter + static_cast<std::string>(tu.p_name);
}

}//namespace Test::{anonymous}

std::string instantiate_db_template::testcase_db_name()
{
    const std::string db_name =
        get_original_db_name() + "_" +
        get_unit_test_path(boost::unit_test::framework::current_test_case(), "_") +
        db_name_suffix_;
    if (db_name.length() <= max_postgresql_database_name_length)
    {
        return db_name;
    }
    return db_name.substr(db_name.length() - max_postgresql_database_name_length,
                          max_postgresql_database_name_length);
}

instantiate_db_template::instantiate_db_template(const std::string& db_name_suffix)
    : db_name_suffix_(db_name_suffix)
{
    BOOST_REQUIRE(testcase_db_name().length() <= max_postgresql_database_name_length);

    force_copy_db(
            create_db_template::get_db_template_name(),
            get_original_db_name(),
            CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection());
}

instantiate_db_template::~instantiate_db_template()
{
    const std::string log_db_name = testcase_db_name();

    const std::unique_ptr<Database::StandaloneConnection> conn =
        CfgArgs::instance()->get_handler_ptr_by_type<HandleAdminDatabaseArgs>()->get_admin_connection();

    disable_connections_add_terminate_persistent_connections(conn, get_original_db_name());

    force_drop_db(log_db_name, conn);
    conn->exec("ALTER DATABASE \""+ get_original_db_name() +"\" RENAME TO \"" + log_db_name + "\"");

    enable_connections(conn, log_db_name);
}

std::shared_ptr<po::options_description> HandleAdminDatabaseArgs::get_options_description()
{
     const auto db_opts = std::make_shared<po::options_description>(
             std::string("Admin database connection configuration"));

     db_opts->add_options()
         ("admin_database.name",
          po::value<std::string>()->default_value("fred"),
          "admin database name")
         ("admin_database.user",
          po::value<std::string>()->default_value("fred"),
          "admin database user name")
         ("admin_database.password",
          po::value<std::string>()->default_value("password"),
          "admin database password")
         ("admin_database.host",
          po::value<std::string>()->default_value("localhost"),
          "admin database hostname")
         ("admin_database.port",
          po::value<unsigned int>()->default_value(5432),
          "admin database port number")
         ("admin_database.timeout",
          po::value<unsigned int>()->default_value(10),
          "admin database timeout");

     return db_opts;
}

void HandleAdminDatabaseArgs::handle(int argc, char* argv[], FakedArgs& fa)
{
    po::variables_map vm;

    handler_parse_args()(get_options_description(), vm, argc, argv, fa);

    /* construct connection string */
    host = vm["admin_database.host"].as<std::string>();
    pass = vm["admin_database.password"].as<std::string>();
    dbname = vm["admin_database.name"].as<std::string>();
    user = vm["admin_database.user"].as<std::string>();
    port = boost::lexical_cast<std::string>(vm["admin_database.port"].as<unsigned>());
    timeout = boost::lexical_cast<std::string>(vm["admin_database.timeout"].as<unsigned>());
}

std::unique_ptr<Database::StandaloneConnection> HandleAdminDatabaseArgs::get_admin_connection()
{
    return Database::StandaloneManager(
            "host=" + host + " "
            "port=" + port + " "
            "dbname=" + dbname + " "
            "user=" + user + " "
            "password=" + pass + " "
            "connect_timeout=" + timeout).acquire();
}

}//namespace Test
