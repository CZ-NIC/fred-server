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

#include "src/libfred/opcontext.hh"
#include "src/libfred/opexception.hh"
#include "src/libfred/zone/create_zone.hh"
#include "src/libfred/zone/exceptions.hh"
#include "src/libfred/zone/zone_soa/create_zone_soa.hh"
#include "src/libfred/zone/zone_soa/default_values.hh"
#include "src/libfred/zone/zone_soa/info_zone_soa.hh"
#include "src/libfred/zone/zone_soa/update_zone_soa.hh"
#include "src/libfred/zone/zone_soa/exceptions.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/zone/zone_soa/util.hh"
#include "test/libfred/zone/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct UpdateZoneSoaFixture
{
    std::string fqdn;
    std::string hostmaster;
    std::string ns_fqdn;
    ::LibFred::Zone::InfoZoneSoaData zone_soa;

    UpdateZoneSoaFixture(::LibFred::OperationContext& _ctx)
        : fqdn(RandomDataGenerator().xstring(3)),
          hostmaster("hostmaster@nic.cz"),
          ns_fqdn("t.ns.nic." + fqdn)
    {
        ::LibFred::Zone::CreateZone(fqdn, 5, 6).exec(_ctx);

        zone_soa.ttl = ::LibFred::Zone::default_ttl_in_seconds;
        zone_soa.hostmaster = hostmaster;
        zone_soa.refresh = ::LibFred::Zone::default_refresh_in_seconds;
        zone_soa.update_retr = ::LibFred::Zone::default_update_retr_in_seconds;
        zone_soa.expiry = ::LibFred::Zone::default_expiry_in_seconds;
        zone_soa.minimum = ::LibFred::Zone::default_minimum_in_seconds;
        zone_soa.ns_fqdn = ns_fqdn;
    }

    ~UpdateZoneSoaFixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateZoneSoa, SupplyFixtureCtx<UpdateZoneSoaFixture>)

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZoneSoa(RandomDataGenerator().xstring(3))
                .set_ttl(zone_soa.ttl)
                .exec(ctx),
           ::LibFred::Zone::NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone_soa)
{
    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZoneSoa(fqdn)
                .set_ttl(zone_soa.ttl)
                .exec(ctx),
           ::LibFred::Zone::NonExistentZoneSoa);
}

BOOST_AUTO_TEST_CASE(set_no_update_zone_soa)
{
    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZoneSoa(fqdn)
                .exec(ctx),
           ::LibFred::Zone::NoZoneSoaData);
}

BOOST_AUTO_TEST_CASE(set_min_update_zone_soa)
{
    zone_soa.zone = ::LibFred::Zone::CreateZoneSoa(fqdn, hostmaster, ns_fqdn).exec(ctx);
    zone_soa.minimum = new_minimum_in_seconds;
    ::LibFred::Zone::UpdateZoneSoa(fqdn)
           .set_minimum(zone_soa.minimum)
           .exec(ctx);
    BOOST_CHECK(zone_soa == ::LibFred::Zone::InfoZoneSoa(fqdn).exec(ctx));
}

BOOST_AUTO_TEST_CASE(set_max_update_zone_soa)
{
    ::LibFred::Zone::InfoZoneSoaData zone_soa;

    zone_soa.zone = ::LibFred::Zone::CreateZoneSoa(fqdn, zone_soa.hostmaster, zone_soa.ns_fqdn).exec(ctx);
    zone_soa.ttl = new_ttl_in_seconds;
    zone_soa.hostmaster = new_hostmaster;
    zone_soa.refresh = new_refresh_in_seconds;
    zone_soa.update_retr = new_update_retr_in_seconds;
    zone_soa.expiry = new_expiry_in_seconds;
    zone_soa.minimum = new_minimum_in_seconds;
    zone_soa.ns_fqdn = new_ns_fqdn;

    ::LibFred::Zone::UpdateZoneSoa(fqdn)
            .set_ttl(zone_soa.ttl)
            .set_hostmaster(zone_soa.hostmaster)
            .set_refresh(zone_soa.refresh)
            .set_update_retr(zone_soa.update_retr)
            .set_expiry(zone_soa.expiry)
            .set_minimum(zone_soa.minimum)
            .set_ns_fqdn(zone_soa.ns_fqdn)
            .exec(ctx);

    BOOST_CHECK(zone_soa == ::LibFred::Zone::InfoZoneSoa(fqdn).exec(ctx));
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test

