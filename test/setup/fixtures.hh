/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  test fixtures
 */

#ifndef FIXTURES_HH_327B2D873D064624803561323E8E3BF3
#define FIXTURES_HH_327B2D873D064624803561323E8E3BF3

#include <string>
#include "src/util/cfg/handle_args.hh"
#include "libfred/db_settings.hh"

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

            std::shared_ptr<boost::program_options::options_description> get_options_description();

            void handle( int argc, char* argv[],  FakedArgs &fa);

            std::unique_ptr<Database::StandaloneConnection> get_admin_connection();
    };


} // namespace Test

#endif
