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

#include "src/libfred/registrar/group/cancel_registrar_group.hh"
#include "src/libfred/registrar/group/create_registrar_group.hh"
#include "src/libfred/registrar/group/get_registrar_groups.hh"
#include "src/libfred/registrar/group/registrar_group_type.hh"

#include "src/libfred/db_settings.hh"
#include "src/libfred/opcontext.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

BOOST_AUTO_TEST_SUITE(TestGetRegistrarGroups)

struct test_get_groups_fixture : virtual public Test::instantiate_db_template
{
    std::vector<unsigned long long> ids;
    boost::posix_time::ptime time;
    std::string name;

    test_get_groups_fixture()
    : name("test_reg_grp")
    {
        LibFred::OperationContextCreator ctx;
        for (int i = 0; i < 10; ++i)
        {
            ids.push_back(LibFred::Registrar::CreateRegistrarGroup(name + boost::lexical_cast<std::string>(i)).exec(ctx));
        }
        for (int i = 0; i < 4; ++i)
        {
            LibFred::Registrar::CancelRegistrarGroup(ids[i]).exec(ctx);
        }
        time = time_from_string(static_cast<std::string>(ctx.get_conn().exec("SELECT now() ")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_registrar_groups, test_get_groups_fixture)
{
    LibFred::OperationContextCreator ctx;
    std::vector<LibFred::Registrar::RegistrarGroup> result = LibFred::Registrar::GetRegistrarGroups().exec(ctx);
    std::map<int, LibFred::Registrar::RegistrarGroup> group_map;
    for (std::vector<LibFred::Registrar::RegistrarGroup>::iterator it = result.begin(); it < result.end(); ++it)
    {
        group_map[it->id] = *it;
    }
    for (int i = 0; i < 4; ++i)
    {
        BOOST_CHECK(group_map[ids[i]].name == name + boost::lexical_cast<std::string>(i));
        BOOST_CHECK(group_map[ids[i]].cancelled == time);
    }
    for (int i = 4; i < 10; ++i)
    {
        BOOST_CHECK(group_map[ids[i]].name == name + boost::lexical_cast<std::string>(i));
        BOOST_CHECK(group_map[ids[i]].cancelled.is_not_a_date_time());
    }
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END(); //TestGetRegistrarGroups
