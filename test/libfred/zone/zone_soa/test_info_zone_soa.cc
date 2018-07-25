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
#include "src/libfred/zone/zone_soa/info_zone_soa_data.hh"
#include "src/libfred/zone/zone_soa/info_zone_soa.hh"
#include "src/libfred/zone/zone_soa/exceptions.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/zone/util.hh"
#include "test/libfred/zone/zone_soa/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct InfoZoneSoaFixture
{
    ::LibFred::Zone::InfoZoneSoaData zone_soa;
    std::string fqdn;

    InfoZoneSoaFixture(::LibFred::OperationContext& _ctx)
            :fqdn(RandomDataGenerator().xstring(3))
    {
        zone_soa.ttl = new_ttl_in_seconds;
        zone_soa.hostmaster = new_hostmaster;
        zone_soa.refresh = new_refresh_in_seconds;
        zone_soa.update_retr = new_update_retr_in_seconds;
        zone_soa.expiry = new_expiry_in_seconds;
        zone_soa.minimum = new_minimum_in_seconds;
        zone_soa.ns_fqdn = new_ns_fqdn;

        zone_soa.zone = ::LibFred::Zone::CreateZone(fqdn, 6, 12).exec(_ctx);

        ::LibFred::Zone::CreateZoneSoa(fqdn)
                .set_ttl(zone_soa.ttl)
                .set_hostmaster(zone_soa.hostmaster)
                .set_refresh(zone_soa.refresh)
                .set_update_retr(zone_soa.update_retr)
                .set_expiry(zone_soa.expiry)
                .set_minimum(zone_soa.minimum)
                .set_ns_fqdn(zone_soa.ns_fqdn)
                .exec(_ctx);
    }
    ~InfoZoneSoaFixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestInfoZoneSoa, SupplyFixtureCtx<InfoZoneSoaFixture>)

BOOST_AUTO_TEST_CASE(set_nonexistent_zone_soa)
{
    BOOST_CHECK_THROW(::LibFred::Zone::InfoZoneSoa(RandomDataGenerator().xstring(5))
                .exec(ctx),
           ::LibFred::Zone::NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_info_zone)
{
    ::LibFred::Zone::InfoZoneSoaData zone_soa_info = ::LibFred::Zone::InfoZoneSoa(fqdn).exec(ctx);
    BOOST_CHECK(zone_soa == zone_soa_info);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
