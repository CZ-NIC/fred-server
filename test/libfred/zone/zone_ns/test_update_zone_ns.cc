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
#include "libfred/zone/zone_ns/create_zone_ns.hh"
#include "libfred/zone/zone_ns/info_zone_ns.hh"
#include "libfred/zone/zone_ns/update_zone_ns.hh"
#include "libfred/zone/zone_ns/exceptions.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/libfred/zone/util.hh"
#include "test/libfred/zone/zone_ns/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/ip/address.hpp>
#include <limits>
#include <string>
#include <vector>

namespace Test {

struct UpdateZoneNsFixture
{
    std::string zone;
    ::LibFred::Zone::InfoZoneNsData info_zone_ns;

    UpdateZoneNsFixture(::LibFred::OperationContext& _ctx)
        : zone(Random::Generator().get_seq(Random::CharSet::letters(), 3))
    {
        info_zone_ns.zone_id = ::LibFred::Zone::CreateZone(zone, 6, 12).exec(_ctx);

        info_zone_ns.nameserver_fqdn = "a.ns.nic.cz";

        std::vector<boost::asio::ip::address> ns_ip_addrs;
        info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

        info_zone_ns.id = ::LibFred::Zone::CreateZoneNs(zone, info_zone_ns.nameserver_fqdn)
                .exec(_ctx);
    }

    ~UpdateZoneNsFixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateZoneNs, SupplyFixtureCtx<UpdateZoneNsFixture>)


BOOST_AUTO_TEST_CASE(set_zone_ns_no_data)
{
    const unsigned int rnd_uint = Random::Generator().get(
            std::numeric_limits<unsigned>::min(), 
            std::numeric_limits<unsigned>::max());
    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZoneNs(info_zone_ns.zone_id + rnd_uint)
                .exec(ctx),
           ::LibFred::Zone::NoZoneNsData);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone_ns)
{
    const unsigned int rnd_uint = Random::Generator().get(
            std::numeric_limits<unsigned>::min(), 
            std::numeric_limits<unsigned>::max());
    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZoneNs(info_zone_ns.zone_id + rnd_uint)
                .set_nameserver_fqdn(info_zone_ns.nameserver_fqdn)
                .exec(ctx),
           ::LibFred::Zone::NonExistentZoneNs);
}

BOOST_AUTO_TEST_CASE(set_zone_ns_update_min)
{
   std::transform(zone.begin(), zone.end(), zone.begin(), ::tolower);
   info_zone_ns.nameserver_fqdn = "testovaci.domena." + zone;

   const unsigned long long id = ::LibFred::Zone::UpdateZoneNs(info_zone_ns.id)
                .set_nameserver_fqdn(info_zone_ns.nameserver_fqdn)
                .exec(ctx);

   BOOST_CHECK(info_zone_ns == ::LibFred::Zone::InfoZoneNs(id).exec(ctx));
}

BOOST_AUTO_TEST_CASE(set_zone_ns_update_ip_address)
{
   std::vector<boost::asio::ip::address> ns_ip_addrs;
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("9.9.9.9"));
   info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

   const unsigned long long id = ::LibFred::Zone::UpdateZoneNs(info_zone_ns.id)
                .set_nameserver_ip_addresses(info_zone_ns.nameserver_ip_addresses)
                .exec(ctx);

   BOOST_CHECK(info_zone_ns == ::LibFred::Zone::InfoZoneNs(id).exec(ctx));
}

BOOST_AUTO_TEST_CASE(set_zone_ns_update_all)
{
   zone = Random::Generator().get_seq(Random::CharSet::letters(), 3);
   info_zone_ns.zone_id = ::LibFred::Zone::CreateZone(zone, 6, 12).exec(ctx);

   std::transform(zone.begin(), zone.end(), zone.begin(), ::tolower);
   info_zone_ns.nameserver_fqdn = "a.b.c.test." + zone;

   std::vector<boost::asio::ip::address> ns_ip_addrs;
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("1.2.3.4"));
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("5.6.7.8"));
   ns_ip_addrs.push_back(boost::asio::ip::address::from_string("9.9.9.9"));
   info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

   const unsigned long long id = ::LibFred::Zone::UpdateZoneNs(info_zone_ns.id)
                .set_zone_fqdn(zone)
                .set_nameserver_fqdn(info_zone_ns.nameserver_fqdn)
                .set_nameserver_ip_addresses(info_zone_ns.nameserver_ip_addresses)
                .exec(ctx);

   BOOST_CHECK(info_zone_ns == ::LibFred::Zone::InfoZoneNs(id).exec(ctx));
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
