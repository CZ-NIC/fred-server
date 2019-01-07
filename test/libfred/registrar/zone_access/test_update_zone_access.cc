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
#include "src/libfred/registrar/zone_access/update_registrar_zone_access.hh"
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

struct UpdateRegistrarZoneAccessFixture
{
    std::string registrar_handle;
    unsigned long long zone_access_id;
    std::string zone_fqdn;
    int ex_period_min;
    int ex_period_max;
    boost::gregorian::date from_date;
    boost::gregorian::date to_date;

    UpdateRegistrarZoneAccessFixture(::LibFred::OperationContext& _ctx)
        : registrar_handle(RandomDataGenerator().xstring(10)),
          zone_fqdn(RandomDataGenerator().xstring(3)),
          ex_period_min(RandomDataGenerator().xnum1_5()),
          ex_period_max(RandomDataGenerator().xnum1_5() + 5),
          from_date(RandomDataGenerator().xdate()),
          to_date(RandomDataGenerator().xdate())
    {
        ::LibFred::CreateRegistrar(registrar_handle).exec(_ctx);
        ::LibFred::Zone::CreateZone(zone_fqdn, ex_period_min, ex_period_max).exec(_ctx);
        zone_access_id =
                ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, zone_fqdn, from_date).exec(_ctx);
    }
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateRegistrarZoneAccess, SupplyFixtureCtx<UpdateRegistrarZoneAccessFixture>)

BOOST_AUTO_TEST_CASE(set_no_update_data)
{
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::ZoneAccess::UpdateRegistrarZoneAccess(zone_access_id).exec(ctx),
            ::LibFred::Registrar::ZoneAccess::NoUpdateData);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone_access)
{
    const unsigned long long nonexistent_id = RandomDataGenerator().xuint();
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::ZoneAccess::UpdateRegistrarZoneAccess(nonexistent_id)
                    .set_to_date(to_date)
                    .exec(ctx),
            ::LibFred::Registrar::ZoneAccess::NonexistentZoneAccess);
}

BOOST_AUTO_TEST_CASE(set_zone_access_to_date)
{
    ::LibFred::Registrar::ZoneAccess::UpdateRegistrarZoneAccess(zone_access_id)
            .set_to_date(to_date)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_zone_access_id(ctx, registrar_handle, zone_fqdn, from_date, to_date), zone_access_id);
}

BOOST_AUTO_TEST_CASE(set_zone_access_from_and_to_date)
{
    from_date = RandomDataGenerator().xdate();
    ::LibFred::Registrar::ZoneAccess::UpdateRegistrarZoneAccess(zone_access_id)
            .set_from_date(from_date)
            .set_to_date(to_date)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_zone_access_id(ctx, registrar_handle, zone_fqdn, from_date, to_date), zone_access_id);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
