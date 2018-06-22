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
#include "src/libfred/zone/create_zone.hh"
#include "src/libfred/zone_ns/create_zone_ns.hh"
#include "src/libfred/zone_ns/info_zone_ns_data.hh"
#include "src/libfred/zone_ns/info_zone_ns.hh"
#include "src/libfred/zone_ns/util.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/zone_ns/util.hh"
#include "test/libfred/zone/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct InfoZoneNsFixture
{
    std::string zone;
    ::LibFred::ZoneNs::InfoZoneNsData info_zone_ns;

    InfoZoneNsFixture(::LibFred::OperationContext& _ctx)
        : zone(RandomDataGenerator().xstring(3))
    {
        info_zone_ns.zone_id = ::LibFred::Zone::CreateZone(zone, 6, 12).exec(_ctx);

        std::transform(zone.begin(), zone.end(), zone.begin(), ::tolower);
        info_zone_ns.nameserver_fqdn = "a.b.c.test." + zone;

        std::vector<boost::asio::ip::address> ns_ip_addrs;
        ns_ip_addrs.push_back(boost::asio::ip::address::from_string("1.2.3.4"));
        ns_ip_addrs.push_back(boost::asio::ip::address::from_string("5.6.7.8"));
        ns_ip_addrs.push_back(boost::asio::ip::address::from_string("9.9.9.9"));
        info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

        info_zone_ns.id = ::LibFred::ZoneNs::CreateZoneNs(zone)
                .set_nameserver_fqdn(info_zone_ns.nameserver_fqdn)
                .set_nameserver_ip_addresses(info_zone_ns.nameserver_ip_addresses)
                .exec(_ctx);
    }

    ~InfoZoneNsFixture()
    {
    }
};


BOOST_FIXTURE_TEST_SUITE(TestInfoZoneNs, SupplyFixtureCtx<InfoZoneNsFixture>)

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    BOOST_CHECK_THROW(::LibFred::ZoneNs::InfoZoneNs(info_zone_ns.id + RandomDataGenerator().xuint())
                .exec(ctx),
           ::LibFred::ZoneNs::NonExistentZoneNs);
}

BOOST_AUTO_TEST_CASE(set_info_zone_ns)
{
    ::LibFred::ZoneNs::InfoZoneNsData zone_ns_info = ::LibFred::ZoneNs::InfoZoneNs(info_zone_ns.id).exec(ctx);
    BOOST_CHECK(info_zone_ns == zone_ns_info);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
