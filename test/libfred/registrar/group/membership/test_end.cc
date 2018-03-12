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
#include "src/fredlib/registrar/group/membership/end_registrar_group_membership.h"
#include "src/fredlib/registrar/group/membership/exceptions.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#include <boost/test/unit_test.hpp>

using namespace boost::gregorian;

struct test_end_membership_fixture : virtual public Test::Fixture::instantiate_db_template
{
    unsigned long long group_id;
    unsigned long long past_group_id;
    unsigned long long mem_id;
    unsigned long long past_mem_id;
    Fred::InfoRegistrarData reg;
    date today;

    test_end_membership_fixture()
    : today(day_clock::universal_day())
    {
        Fred::OperationContextCreator ctx;
        reg = Test::registrar::make(ctx);
        group_id = Fred::CreateRegistrarGroup("test_reg_grp").exec(ctx);
        mem_id = Fred::CreateRegistrarGroupMembership(reg.id, group_id, today).exec(ctx);

        past_group_id = Fred::CreateRegistrarGroup("past_test_reg_grp").exec(ctx);
        past_mem_id = Fred::CreateRegistrarGroupMembership(
                reg.id,
                past_group_id,
                today - date_duration(1),
                today - date_duration(1))
            .exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestEndRegistrarGroupMembership, test_end_membership_fixture)

BOOST_AUTO_TEST_CASE(end_group_membership)
{
    Fred::OperationContextCreator ctx;
    Fred::EndRegistrarGroupMembership(reg.id, group_id).exec(ctx);
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

BOOST_AUTO_TEST_CASE(membership_not_found)
{
    Fred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
        Fred::EndRegistrarGroupMembership(reg.id, 123456).exec(ctx),
        MembershipNotFound);
    BOOST_CHECK_THROW(
        Fred::EndRegistrarGroupMembership(123456, group_id).exec(ctx),
        MembershipNotFound);
    BOOST_CHECK_THROW(
        Fred::EndRegistrarGroupMembership(reg.id, past_group_id).exec(ctx),
        MembershipNotFound);
}

BOOST_AUTO_TEST_SUITE_END(); //TestEndRegistrarGroupMembership
