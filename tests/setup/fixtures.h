/**
 *  @file
 *  test fixtures
 */

#ifndef TESTS_SETUP_FIXTURES_41215653023
#define TESTS_SETUP_FIXTURES_41215653023

#include <string>
#include "util/cfg/handle_args.h"
#include "src/fredlib/db_settings.h"

/**
 * @file fixtures for data isolation in tests
 * for more info see documentation with examples:
 * https://admin.nic.cz/wiki/developers/fred/tests
 */

namespace Test {

    // database created by fred-manager init_cz
    static std::string get_original_db_name() { return "fred"; }



    struct create_db_template {
        static std::string get_db_template_name() {
            return get_original_db_name() + "_test_template";
        }

        create_db_template();
        virtual ~create_db_template();
    };



    struct instantiate_db_template {
        const std::string db_name_suffix_;/**< suffix of the name of database instance left in database cluster after fixture teardown, useful in case of more database instances per testcase */
        instantiate_db_template(const std::string& db_name_suffix = "");
        virtual ~instantiate_db_template();
    private:
        std::string testcase_db_name();
    };


    /***
     * config handlers for admin connection to db used by fixtures related to db data
     */
    class HandleAdminDatabaseArgs : public HandleArgs {

        public:
            std::string host;
            std::string port;
            std::string user;
            std::string pass;
            std::string dbname;
            std::string timeout;

            boost::shared_ptr<boost::program_options::options_description> get_options_description();

            void handle( int argc, char* argv[],  FakedArgs &fa);

            std::auto_ptr<Database::StandaloneConnection> get_admin_connection();
    };


} // namespace Test

#endif // #include guard end
