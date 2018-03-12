/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "src/fredlib/registrar/group/create_registrar_group.h"
#include "src/fredlib/registrar/group/membership/create_registrar_group_membership.h"
#include "src/fredlib/registrar/group/membership/exceptions.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#include <boost/test/unit_test.hpp>

#include <string>

using namespace boost::gregorian;

struct test_create_membership_fixture : virtual public Test::Fixture::instantiate_db_template
{
    unsigned long long group_id;
    Fred::InfoRegistrarData reg;
    date today;

    test_create_membership_fixture()
    : today(day_clock::universal_day())
    {
        Fred::OperationContextCreator ctx;
        reg = Test::registrar::make(ctx);
        group_id = Fred::CreateRegistrarGroup("test-reg_grp").exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestCreateRegistrarGroupMembership, test_create_membership_fixture)

BOOST_AUTO_TEST_CASE(create_group_membership)
{
    Fred::OperationContextCreator ctx;
    unsigned long long mem_id =
        Fred::CreateRegistrarGroupMembership(reg.id, group_id, today, today).exec(ctx);
    Database::Result result = ctx.get_conn().exec_params(
            "SELECT registrar_id, registrar_group_id, member_from, member_until "
            "FROM registrar_group_map "
            "WHERE id = $1::bigint",
            Database::query_param_list(mem_id));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(result[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(result[0][1]));
    BOOST_CHECK(today == from_string(result[0][2]));
    BOOST_CHECK(today == from_string(result[0][3]));
}

BOOST_AUTO_TEST_CASE(inifite_membership_cut)
{
    Fred::OperationContextCreator ctx;
    unsigned long long mem_id1 = Fred::CreateRegistrarGroupMembership(
            reg.id,
            group_id,
            today)
        .exec(ctx);
    const char* query =
            "SELECT registrar_id, registrar_group_id, member_from, member_until "
            "FROM registrar_group_map "
            "WHERE id = $1::bigint";
    Database::Result result1 = ctx.get_conn().exec_params(
            query,
            Database::query_param_list(mem_id1));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(result1[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(result1[0][1]));
    BOOST_CHECK(today ==
            from_string(result1[0][2]));
    BOOST_CHECK(static_cast<std::string>(result1[0][3]).empty());

    date tomorrow = today + date_duration(1);
    unsigned long long mem_id2 =
        Fred::CreateRegistrarGroupMembership(reg.id, group_id, tomorrow).exec(ctx);

    result1 = ctx.get_conn().exec_params(query, Database::query_param_list(mem_id1));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(result1[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(result1[0][1]));
    BOOST_CHECK(today == from_string(result1[0][2]));
    BOOST_CHECK(today == from_string(result1[0][2]));

    Database::Result result2 =
        ctx.get_conn().exec_params(query, Database::query_param_list(mem_id2));
    BOOST_CHECK(reg.id == static_cast<unsigned long long>(result2[0][0]));
    BOOST_CHECK(group_id == static_cast<unsigned long long>(result2[0][1]));
    BOOST_CHECK(tomorrow == from_string(result2[0][2]));
    BOOST_CHECK(static_cast<std::string>(result2[0][3]).empty());
}

BOOST_AUTO_TEST_CASE(wrong_interval_order)
{
    Fred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
        Fred::CreateRegistrarGroupMembership(reg.id, group_id, today, today - date_duration(1)).exec(ctx),
        WrongIntervalOrder);
}

BOOST_AUTO_TEST_CASE(interval_intersection)
{
    Fred::OperationContextCreator ctx;
    Fred::CreateRegistrarGroupMembership(reg.id, group_id, today, today).exec(ctx);

    BOOST_CHECK_THROW(
        Fred::CreateRegistrarGroupMembership(reg.id, group_id, today - date_duration(1), today).exec(ctx),
        IntervalIntersection);
}

BOOST_AUTO_TEST_SUITE_END(); //TestCreateRegistrarGroupMembership
