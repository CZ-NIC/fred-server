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
#include "src/fredlib/registrar/group/membership/info_group_membership_by_registrar.cc"
#include "src/fredlib/registrar/group/membership/exceptions.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#include <boost/test/unit_test.hpp>
#include <map>

using namespace boost::gregorian;

struct test_membership_by_registrar_fixture : virtual public Test::Fixture::instantiate_db_template
{
    Fred::InfoRegistrarData reg;
    std::map<unsigned long long, unsigned long long> mem_map;
    date today;

    test_membership_by_registrar_fixture()
    : today(day_clock::universal_day())
    {
        Fred::OperationContextCreator ctx;
        reg = Test::registrar::make(ctx);
        RandomDataGenerator rdg;
        for (int i = 0; i < 5; ++i)
        {
            unsigned long long group_id =
                Fred::CreateRegistrarGroup(std::string("test_reg_grp_") + rdg.xnumstring(6)).exec(ctx);
            mem_map[Fred::CreateRegistrarGroupMembership(reg.id, group_id, today).exec(ctx)] = group_id;
        }
        Fred::CreateRegistrarGroupMembership(
                Test::registrar::make(ctx).id,
                Fred::CreateRegistrarGroup("other_test_reg_grp").exec(ctx),
                today)
            .exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestGroupMembershipByRegistrar, test_membership_by_registrar_fixture)

BOOST_AUTO_TEST_CASE(info_by_registrar_membership)
{
    Fred::OperationContextCreator ctx;
    std::vector<Fred::GroupMembershipByRegistrar> info_list =
        Fred::InfoGroupMembershipByRegistrar(reg.id).exec(ctx);
    BOOST_CHECK_EQUAL(info_list.size(), mem_map.size());
    BOOST_FOREACH(Fred::GroupMembershipByRegistrar it, info_list)
    {
        std::map<unsigned long long, unsigned long long>::iterator map_it =
            mem_map.find(it.membership_id);
        BOOST_CHECK(map_it != mem_map.end());
        BOOST_CHECK(it.group_id == map_it->second);
        BOOST_CHECK(it.member_from == today);
        BOOST_CHECK(it.member_until.is_pos_infinity());
    }
}

BOOST_AUTO_TEST_SUITE_END(); //TestEndRegistrarGroupMembership
