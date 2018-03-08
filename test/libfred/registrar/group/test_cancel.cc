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

#include "src/libfred/registrar/group/create_registrar_group.hh"
#include "src/libfred/registrar/group/cancel_registrar_group.hh"
#include "src/libfred/registrar/group/exceptions.hh"

#include "src/libfred/db_settings.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/db/value.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>

struct test_cancel_group_fixture : virtual public Test::instantiate_db_template
{
    std::string name;
    unsigned long long id;

    test_cancel_group_fixture() : name("test_reg_grp")
    {
        LibFred::OperationContextCreator ctx;
        id = LibFred::Registrar::CreateRegistrarGroup(name).exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestCancelRegistrarGroup, test_cancel_group_fixture)

BOOST_AUTO_TEST_CASE(cancel_registrar_group)
{
    LibFred::OperationContextCreator ctx;
    LibFred::Registrar::CancelRegistrarGroup(id).exec(ctx);
    Database::Result result = ctx.get_conn().exec_params(
            "SELECT id, short_name, cancelled FROM registrar_group "
            "WHERE id = $1::bigint",
            Database::query_param_list(id));
    BOOST_CHECK(name == static_cast<std::string>(result[0][1]));
    BOOST_CHECK(boost::gregorian::day_clock::universal_day() ==
            (result[0][2].operator ptime()).date());
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_CASE(already_cancelled)
{
    LibFred::OperationContextCreator ctx;
    LibFred::Registrar::CancelRegistrarGroup(id).exec(ctx);
    BOOST_CHECK_THROW(
        LibFred::Registrar::CancelRegistrarGroup(id).exec(ctx),
        AlreadyCancelled);
}

BOOST_AUTO_TEST_CASE(nonempty_group_delete)
{
    LibFred::OperationContextCreator ctx;
    LibFred::InfoRegistrarData reg = Test::registrar::make(ctx);
    ctx.get_conn().exec_params(
            "INSERT INTO registrar_group_map (registrar_id, registrar_group_id, member_from, member_until) "
            "VALUES ($1::bigint, $2::bigint, $3::date, $4::date) ",
            Database::query_param_list(reg.id)
                (id)
                (boost::gregorian::day_clock::universal_day())
                (boost::gregorian::day_clock::universal_day() + boost::gregorian::date_duration(1)));

    BOOST_CHECK_THROW(
        LibFred::Registrar::CancelRegistrarGroup(id).exec(ctx),
        NonemptyGroupDelete);
}

BOOST_AUTO_TEST_SUITE_END(); //TestCancelRegistrarGroup
