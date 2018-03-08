/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/libfred/registrar/group/create_registrar_group.hh"
#include "src/libfred/registrar/group/exceptions.hh"

#include "src/libfred/db_settings.hh"
#include "src/libfred/opcontext.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_AUTO_TEST_SUITE(TestCreateRegistrarGroup)

struct test_create_group_fixture : virtual public Test::instantiate_db_template
{
    std::string name;
    unsigned long long id;

    test_create_group_fixture() : name("test_reg_grp")
    {
        LibFred::OperationContextCreator ctx;
        id = LibFred::Registrar::CreateRegistrarGroup(name).exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(create_registrar_group, test_create_group_fixture)
{
    LibFred::OperationContextCreator ctx;
    Database::Result result = ctx.get_conn().exec_params(
            "SELECT id, short_name, cancelled FROM registrar_group "
            "WHERE id = $1::bigint",
            Database::query_param_list(id));
    BOOST_CHECK(name == static_cast<std::string>(result[0][1]));
    BOOST_CHECK(static_cast<std::string>(result[0][2]).empty());
    ctx.commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(group_exists, test_create_group_fixture)
{
    LibFred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
            LibFred::Registrar::CreateRegistrarGroup(name).exec(ctx),
            GroupExists);
}

BOOST_FIXTURE_TEST_CASE(empty_group_name, test_create_group_fixture)
{
    LibFred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarGroup("").exec(ctx),
        EmptyGroupName);
}

BOOST_AUTO_TEST_SUITE_END(); //TestCreateRegistrarGroup
