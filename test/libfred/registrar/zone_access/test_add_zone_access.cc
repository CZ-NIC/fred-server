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

#include "src/libfred/registrar/create_registrar.hh"
#include "src/libfred/registrar/zone_access/add_registrar_zone_access.hh"
#include "src/libfred/registrar/zone_access/exceptions.hh"
#include "src/libfred/zone/create_zone.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/registrar/util.hh"
#include "test/libfred/registrar/zone_access/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
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
        : registrar_handle(RandomDataGenerator().xstring(10)),
          zone_fqdn(RandomDataGenerator().xstring(3)),
          ex_period_min(RandomDataGenerator().xnum1_5()),
          ex_period_max(RandomDataGenerator().xnum1_5() + 5),
          from_date(RandomDataGenerator().xdate()),
          to_date(not_a_date_time)
    {
        ::LibFred::CreateRegistrar(registrar_handle).exec(_ctx);
        ::LibFred::Zone::CreateZone(zone_fqdn, ex_period_min, ex_period_max).exec(_ctx);
    }
};

BOOST_FIXTURE_TEST_SUITE(TestAddRegistrarZoneAccess, SupplyFixtureCtx<AddRegistrarZoneAccessFixture>)

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar)
{
    const std::string& nonexistent_registrar = "noreg" + RandomDataGenerator().xstring(3);
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
    const std::string& nonexistent_zone = "nozone" + RandomDataGenerator().xstring(3);
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
                    registrar_handle,
                    nonexistent_zone,
                    from_date)
            .exec(ctx),
            ::LibFred::Registrar::ZoneAccess::NonexistentZone);
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
    to_date = RandomDataGenerator().xdate();
    const unsigned long long id =
            ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, zone_fqdn, from_date)
                    .set_to_date(to_date)
                    .exec(ctx);
    BOOST_CHECK_EQUAL(get_zone_access_id(ctx, registrar_handle, zone_fqdn, from_date, to_date), id);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
