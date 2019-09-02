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
#include "libfred/opcontext.hh"
#include "libfred/opexception.hh"
#include "libfred/zone/create_zone.hh"
#include "libfred/zone/exceptions.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/libfred/zone/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct create_zone_fixture
{
    std::string fqdn;
    int ex_period_min;
    int ex_period_max;

    create_zone_fixture(::LibFred::OperationContext& _ctx)
        : fqdn(Random::Generator().get_seq(Random::CharSet::letters(), 3)),
          ex_period_min(6),
          ex_period_max(12)
    {}
    ~create_zone_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateZone, SupplyFixtureCtx<create_zone_fixture>)

unsigned long long get_zone_id(::LibFred::OperationContext& _ctx,
        const std::string& _fqdn,
        const int _ex_min,
        const int _ex_max,
        const bool _warn_letter = true,
        const int _val_period = 0)
{
    const Database::Result db_result = _ctx.get_conn().exec_params(
            "SELECT id FROM zone AS z "
            "WHERE z.fqdn = LOWER($1::text) "
            "AND z.ex_period_min = $2::integer "
            "AND z.ex_period_max = $3::integer ",
            Database::query_param_list(_fqdn)(_ex_min)(_ex_max));
    const auto id = static_cast<unsigned long long>(db_result[0][0]);
    return id;
}


BOOST_AUTO_TEST_CASE(set_enum_val_period)
{
    BOOST_CHECK_THROW(::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max)
                .set_enum_validation_period_in_months(5)
                .exec(ctx),
           ::LibFred::Zone::NotEnumZone);
}

BOOST_AUTO_TEST_CASE(set_create_duplicate_zone)
{
    ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max).exec(ctx);
    BOOST_CHECK_THROW(::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max)
                .exec(ctx),
           ::LibFred::Zone::DuplicateZone);
}

BOOST_AUTO_TEST_CASE(set_min_create_zone)
{
   const unsigned long long id = ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max).exec(ctx);
   BOOST_CHECK_EQUAL(get_zone_id(ctx, fqdn, ex_period_min, ex_period_max), id);
}

BOOST_AUTO_TEST_CASE(set_max_create_zone)
{
   const unsigned long long id = ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max)
           .set_sending_warning_letter(true)
           .exec(ctx);
   BOOST_CHECK_EQUAL(get_zone_id(ctx, fqdn, ex_period_min, ex_period_max), id);
}

BOOST_AUTO_TEST_CASE(set_min_create_enum_zone)
{
    fqdn = "0.3.4.e164.arpa";
    const unsigned long long id = ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max).exec(ctx);
    BOOST_CHECK_EQUAL(get_zone_id(ctx, fqdn, ex_period_min, ex_period_max, true, 6), id);
}

BOOST_AUTO_TEST_CASE(set_max_create_enum_zone)
{
    fqdn = "1.2.E164.ArpA";
    const unsigned long long id = ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max)
            .set_enum_validation_period_in_months(5)
            .set_sending_warning_letter(false)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_zone_id(ctx, fqdn, ex_period_min, ex_period_max, false, 5), id);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
