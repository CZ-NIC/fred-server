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
#include "libfred/registrar/group/membership/end_registrar_group_membership.hh"
#include "libfred/registrar/group/membership/exceptions.hh"

#include "libfred/opcontext.hh"
#include "libfred/db_settings.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/unit_test.hpp>

using namespace boost::gregorian;

struct test_end_membership_fixture : virtual public Test::instantiate_db_template
{
    unsigned long long group_id;
    unsigned long long past_group_id;
    unsigned long long mem_id;
    unsigned long long past_mem_id;
    LibFred::InfoRegistrarData reg;
    date today;

    test_end_membership_fixture()
    : today(day_clock::universal_day())
    {
        LibFred::OperationContextCreator ctx;
        reg = Test::registrar::make(ctx);
        group_id = LibFred::Registrar::CreateRegistrarGroup("test_reg_grp").exec(ctx);
        mem_id = LibFred::Registrar::CreateRegistrarGroupMembership(reg.id, group_id, today).exec(ctx);

        past_group_id = LibFred::Registrar::CreateRegistrarGroup("past_test_reg_grp").exec(ctx);
        past_mem_id = LibFred::Registrar::CreateRegistrarGroupMembership(
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
    LibFred::OperationContextCreator ctx;
    LibFred::Registrar::EndRegistrarGroupMembership(reg.id, group_id).exec(ctx);
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

BOOST_AUTO_TEST_CASE(membership_not_found)
{
    LibFred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
        LibFred::Registrar::EndRegistrarGroupMembership(reg.id, 123456).exec(ctx),
        MembershipNotFound);
    BOOST_CHECK_THROW(
        LibFred::Registrar::EndRegistrarGroupMembership(123456, group_id).exec(ctx),
        MembershipNotFound);
    BOOST_CHECK_THROW(
        LibFred::Registrar::EndRegistrarGroupMembership(reg.id, past_group_id).exec(ctx),
        MembershipNotFound);
}

BOOST_AUTO_TEST_SUITE_END();
