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

#include "libfred/registrar/group/create_registrar_group.hh"
#include "libfred/registrar/group/membership/create_registrar_group_membership.hh"
#include "libfred/registrar/group/membership/exceptions.hh"

#include "libfred/opcontext.hh"
#include "libfred/db_settings.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/unit_test.hpp>

#include <string>

using namespace boost::gregorian;

struct test_create_membership_fixture : virtual public Test::instantiate_db_template
{
    unsigned long long group_id;
    LibFred::InfoRegistrarData reg;
    date today;

    test_create_membership_fixture()
    : today(day_clock::universal_day())
    {
        LibFred::OperationContextCreator ctx;
        reg = Test::registrar::make(ctx);
        group_id = LibFred::Registrar::CreateRegistrarGroup("test-reg_grp").exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestCreateRegistrarGroupMembership, test_create_membership_fixture)

BOOST_AUTO_TEST_CASE(create_group_membership)
{
    LibFred::OperationContextCreator ctx;
    const unsigned long long mem_id =
        LibFred::Registrar::CreateRegistrarGroupMembership(reg.id, group_id, today, today).exec(ctx);
    const Database::Result result = ctx.get_conn().exec_params(
            "SELECT registrar_id, registrar_group_id, member_from, member_until "
            "FROM registrar_group_map "
            "WHERE id = $1::bigint",
            Database::query_param_list(mem_id));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(result[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(result[0][1]));
    BOOST_CHECK(today == from_string(result[0][2]));
    BOOST_CHECK(today == from_string(result[0][3]));
}

BOOST_AUTO_TEST_CASE(create_already_existing_group_membership)
{
    LibFred::OperationContextCreator ctx;
    const unsigned long long created_id = LibFred::Registrar::CreateRegistrarGroupMembership(
            reg.id,
            group_id,
            today)
        .exec(ctx);
    const std::string query =
            "SELECT registrar_id, registrar_group_id, member_from, member_until "
            "FROM registrar_group_map "
            "WHERE id = $1::bigint";
    const Database::Result created_map = ctx.get_conn().exec_params(
            query,
            Database::query_param_list(created_id));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(created_map[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(created_map[0][1]));
    BOOST_CHECK(today == from_string(created_map[0][2]));
    BOOST_CHECK(static_cast<std::string>(created_map[0][3]).empty());

    const date tomorrow = today + date_duration(1);
    const unsigned long long updated_id =
        LibFred::Registrar::CreateRegistrarGroupMembership(reg.id, group_id, tomorrow).exec(ctx);

    const Database::Result history_map = ctx.get_conn().exec_params(
            query,
            Database::query_param_list(created_id));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(history_map[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(history_map[0][1]));
    BOOST_CHECK(today == from_string(history_map[0][2]));
    BOOST_CHECK(tomorrow == from_string(history_map[0][3]));

    const Database::Result current_map =
        ctx.get_conn().exec_params(query, Database::query_param_list(updated_id));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(current_map[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(current_map[0][1]));
    BOOST_CHECK(tomorrow == from_string(current_map[0][2]));
    BOOST_CHECK(static_cast<std::string>(current_map[0][3]).empty());
}

BOOST_AUTO_TEST_CASE(wrong_interval_order)
{
    LibFred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarGroupMembership(reg.id, group_id, today, today - date_duration(1))
                .exec(ctx),
        WrongIntervalOrder);
}

BOOST_AUTO_TEST_CASE(interval_intersection)
{
    LibFred::OperationContextCreator ctx;
    LibFred::Registrar::CreateRegistrarGroupMembership(reg.id, group_id, today, today).exec(ctx);

    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarGroupMembership(reg.id, group_id, today - date_duration(1), today)
                .exec(ctx),
        IntervalIntersection);
}

BOOST_AUTO_TEST_SUITE_END();
