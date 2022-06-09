/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ParallelTests

#include "test/poc/parallel-tests/setup/cfg.hh"
#include "test/poc/parallel-tests/fixtures/has_fresh_database.hh"
#include "test/poc/parallel-tests/fixtures/operation_context.hh"
#include "test/poc/parallel-tests/fixtures/zone.hh"

#include "src/util/cfg/handle_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_domainbrowser_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"

#include "util/log/logger.hh"

#include <boost/test/unit_test.hpp>


namespace {

void setup_database()
{
    using namespace Test;
    struct Setup
    {
        Setup()
            : ctx{ignore_commit_done},
              init_domain_name_checkers{ctx},
              cz_zone{ctx},
              cz_enum_zone{ctx}
        { }
        OperationContext ctx;
        InitDomainNameCheckers init_domain_name_checkers;
        CzZone cz_zone;
        CzEnumZone cz_enum_zone;
        static void ignore_commit_done() noexcept { }
    };
    Setup setup;
    std::move(setup.ctx).commit_transaction();
}

class GlobalFixture
{
public:
    GlobalFixture()
        : database_administrator_{
                []()
                {
                    const HandlerPtrVector config_handlers = {
                            HandleArgsPtr{new HandleLoggingArgs},
                            HandleArgsPtr{new HandleDatabaseArgs},
                            HandleArgsPtr{new HandleDomainBrowserArgs},
                            HandleArgsPtr{new HandleMojeIdArgs}};
                    return Test::Cfg::handle_command_line_args(config_handlers, setup_database);
                }()}
    {
        Test::HasFreshDatabase::set_restore_test_database_procedure(database_administrator_.get_restore_test_database_procedure());
        ::LOGGER.info("tests start");
    }
    ~GlobalFixture()
    {
        try
        {
            ::LOGGER.info("tests done");
            Test::HasFreshDatabase::clear_restore_test_database_procedure();
        }
        catch (...) { }
    }
private:
    Test::Cfg::DatabaseAdministrator database_administrator_;
};

}//namespace {anonymous}

BOOST_GLOBAL_FIXTURE(GlobalFixture);
