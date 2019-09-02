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
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/zone_access/add_registrar_zone_access.hh"
#include "libfred/registrar/zone_access/exceptions.hh"
#include "libfred/zone/create_zone.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/libfred/registrar/util.hh"
#include "test/libfred/registrar/zone_access/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct AddRegistrarZoneAccessFixture
{
    std::string registrar_handle;
    std::string zone_fqdn;
    int ex_period_min;
    int ex_period_max;
    boost::gregorian::date from_date;
    boost::gregorian::date to_date;

    AddRegistrarZoneAccessFixture(::LibFred::OperationContext& _ctx)
        : registrar_handle(Random::Generator().get_seq(Random::CharSet::letters(), 10)),
          zone_fqdn(Random::Generator().get_seq(Random::CharSet::letters(), 3)),
          ex_period_min(Random::Generator().get(1, 5)),
          ex_period_max(Random::Generator().get(6, 10)),
          from_date(Random::Generator().xdate(
                  boost::gregorian::from_simple_string("1990-01-01");
                  boost::gregorian::from_simple_string("2038-01-19"))),
          to_date(not_a_date_time)
    {
        ::LibFred::CreateRegistrar(registrar_handle).exec(_ctx);
        ::LibFred::Zone::CreateZone(zone_fqdn, ex_period_min, ex_period_max).exec(_ctx);
    }
};

BOOST_FIXTURE_TEST_SUITE(TestAddRegistrarZoneAccess, SupplyFixtureCtx<AddRegistrarZoneAccessFixture>)

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar)
{
    const std::string nonexistent_registrar = "noreg" + Random::Generator().get_seq(Random::CharSet::letters(), 3);
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
                    nonexistent_registrar,
                    zone_fqdn,
                    from_date)
            .exec(ctx),
            ::LibFred::Registrar::ZoneAccess::NonexistentRegistrar);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    const std::string nonexistent_zone = "nozone" + Random::Generator().get_seq(Random::CharSet::letters(), 3);
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
                    registrar_handle,
                    nonexistent_zone,
                    from_date)
            .exec(ctx),
            ::LibFred::Registrar::ZoneAccess::NonexistentZone);
}

BOOST_AUTO_TEST_CASE(set_overlapping_range)
{
    ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
            registrar_handle,
            zone_fqdn,
            from_date)
    .exec(ctx),
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
                    registrar_handle,
                    zone_fqdn,
                    from_date)
            .exec(ctx),
            ::LibFred::Registrar::ZoneAccess::OverlappingZoneAccessRange);
}

BOOST_AUTO_TEST_CASE(set_invalid_from_date)
{
    boost::gregorian::date invalid_from_date;
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
                    registrar_handle,
                    zone_fqdn,
                    invalid_from_date)
            .exec(ctx),
            ::LibFred::Registrar::ZoneAccess::InvalidDateFrom);
}

BOOST_AUTO_TEST_CASE(set_min_add_registrar_zone)
{
    const unsigned long long id =
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, zone_fqdn, from_date)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_zone_access_id(ctx, registrar_handle, zone_fqdn, from_date, to_date), id);
}

BOOST_AUTO_TEST_CASE(set_max_add_registrar_zone)
{
    to_date = Random::Generator().xdate(
                  boost::gregorian::from_simple_string("1990-01-01");
                  boost::gregorian::from_simple_string("2038-01-19"));
    const unsigned long long id =
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, zone_fqdn, from_date)
                    .set_to_date(to_date)
                    .exec(ctx);
    BOOST_CHECK_EQUAL(get_zone_access_id(ctx, registrar_handle, zone_fqdn, from_date, to_date), id);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
