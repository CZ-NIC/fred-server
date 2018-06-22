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
#include "src/libfred/zone_ns/create_zone_ns.hh"
#include "src/libfred/zone_ns/info_zone_ns.hh"
#include "src/libfred/zone_ns/update_zone_ns.hh"
#include "src/libfred/zone_ns/util.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/zone/util.hh"
#include "test/libfred/zone_ns/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/ip/address.hpp>
#include <string>
#include <vector>

namespace Test {

struct UpdateZoneNsFixture
{
    std::string zone;
    ::LibFred::ZoneNs::InfoZoneNsData info_zone_ns;

    UpdateZoneNsFixture(::LibFred::OperationContext& _ctx)
        : zone(RandomDataGenerator().xstring(3))
    {
        info_zone_ns.zone_id = ::LibFred::Zone::CreateZone(zone, 6, 12).exec(_ctx);

        info_zone_ns.nameserver_fqdn = "localhost";

        std::vector<boost::asio::ip::address> ns_ip_addrs;
        info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

        info_zone_ns.id = ::LibFred::ZoneNs::CreateZoneNs(zone)
                .exec(_ctx);
    }

    ~UpdateZoneNsFixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateZoneNs, SupplyFixtureCtx<UpdateZoneNsFixture>)


BOOST_AUTO_TEST_CASE(set_zone_ns_no_data)
{
    BOOST_CHECK_THROW(::LibFred::ZoneNs::UpdateZoneNs(info_zone_ns.zone_id+RandomDataGenerator().xuint())
                .exec(ctx),
           ::LibFred::ZoneNs::NoZoneNsData);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone_ns)
{
    BOOST_CHECK_THROW(::LibFred::ZoneNs::UpdateZoneNs(info_zone_ns.zone_id+RandomDataGenerator().xuint())
                .set_nameserver_fqdn(info_zone_ns.nameserver_fqdn)
                .exec(ctx),
           ::LibFred::ZoneNs::NonExistentZoneNs);
}

BOOST_AUTO_TEST_CASE(set_zone_ns_update_min)
{
   std::transform(zone.begin(), zone.end(), zone.begin(), ::tolower);
   info_zone_ns.nameserver_fqdn = "testovaci.domena." + zone;

   const unsigned long long id = ::LibFred::ZoneNs::UpdateZoneNs(info_zone_ns.id)
                .set_nameserver_fqdn(info_zone_ns.nameserver_fqdn)
                .exec(ctx);

   BOOST_CHECK(info_zone_ns == ::LibFred::ZoneNs::InfoZoneNs(id).exec(ctx));
}

BOOST_AUTO_TEST_CASE(set_zone_ns_update_ip_address)
{
   std::vector<boost::asio::ip::address> ns_ip_addrs;
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("9.9.9.9"));
   info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

   const unsigned long long id = ::LibFred::ZoneNs::UpdateZoneNs(info_zone_ns.id)
                .set_nameserver_ip_addresses(info_zone_ns.nameserver_ip_addresses)
                .exec(ctx);

   BOOST_CHECK(info_zone_ns == ::LibFred::ZoneNs::InfoZoneNs(id).exec(ctx));
}

BOOST_AUTO_TEST_CASE(set_zone_ns_update_all)
{
   zone = RandomDataGenerator().xstring(3);
   info_zone_ns.zone_id = ::LibFred::Zone::CreateZone(zone, 6, 12).exec(ctx);

   std::transform(zone.begin(), zone.end(), zone.begin(), ::tolower);
   info_zone_ns.nameserver_fqdn = "a.b.c.test." + zone;

   std::vector<boost::asio::ip::address> ns_ip_addrs;
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("1.2.3.4"));
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("5.6.7.8"));
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("9.9.9.9"));
   info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

   const unsigned long long id = ::LibFred::ZoneNs::UpdateZoneNs(info_zone_ns.id)
                .set_zone_fqdn(zone)
                .set_nameserver_fqdn(info_zone_ns.nameserver_fqdn)
                .set_nameserver_ip_addresses(info_zone_ns.nameserver_ip_addresses)
                .exec(ctx);

   BOOST_CHECK(info_zone_ns == ::LibFred::ZoneNs::InfoZoneNs(id).exec(ctx));
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
