/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/registrar/group/create_registrar_group.hh"
#include "libfred/registrar/group/membership/create_registrar_group_membership.hh"
#include "libfred/registrar/group/membership/info_group_membership_by_group.cc"
#include "libfred/registrar/group/membership/exceptions.hh"

#include "libfred/opcontext.hh"
#include "libfred/db_settings.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/unit_test.hpp>
#include <map>

using namespace boost::gregorian;

struct test_membership_by_group_fixture : virtual public Test::instantiate_db_template
{
    unsigned long long group_id;
    std::map<unsigned long long, LibFred::InfoRegistrarData> mem_map;
    date today;

    test_membership_by_group_fixture()
    : today(day_clock::universal_day())
    {
        LibFred::OperationContextCreator ctx;
        group_id = LibFred::Registrar::CreateRegistrarGroup("test_reg_grp").exec(ctx);
        for (int i = 0; i < 5; ++i)
        {
            const LibFred::InfoRegistrarData reg = Test::registrar::make(ctx);
            mem_map[LibFred::Registrar::CreateRegistrarGroupMembership(reg.id, group_id, today).exec(ctx)] = reg;
        }
        LibFred::Registrar::CreateRegistrarGroupMembership(
                Test::registrar::make(ctx).id,
                LibFred::Registrar::CreateRegistrarGroup("other_test_reg_grp").exec(ctx),
                today)
            .exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestGroupMembershipByGroup, test_membership_by_group_fixture)

BOOST_AUTO_TEST_CASE(info_by_group_membership)
{
    LibFred::OperationContextCreator ctx;
    std::vector<LibFred::Registrar::GroupMembershipByGroup> info_list =
        LibFred::Registrar::InfoGroupMembershipByGroup(group_id).exec(ctx);
    BOOST_CHECK_EQUAL(info_list.size(), mem_map.size());
    for (const auto& i: info_list)
    {
        const auto map_it = mem_map.find(i.membership_id);
        BOOST_CHECK(map_it != mem_map.end());
        BOOST_CHECK(i.registrar_id == map_it->second.id);
        BOOST_CHECK(i.member_from == today);
        BOOST_CHECK(i.member_until.is_pos_infinity());
    }
}

BOOST_AUTO_TEST_SUITE_END();
