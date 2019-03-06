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
#include "libfred/zone/create_zone.hh"
#include "libfred/zone/zone_ns/create_zone_ns.hh"
#include "libfred/zone/zone_ns/info_zone_ns_data.hh"
#include "libfred/zone/zone_ns/info_zone_ns.hh"
#include "libfred/zone/zone_ns/exceptions.hh"
#include "util/random_data_generator.hh"
#include "test/libfred/zone/zone_ns/util.hh"
#include "test/libfred/zone/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct InfoZoneNsFixture
{
    std::string zone;
    ::LibFred::Zone::InfoZoneNsData info_zone_ns;

    InfoZoneNsFixture(::LibFred::OperationContext& _ctx)
        : zone(RandomDataGenerator().xstring(3))
    {
        info_zone_ns.zone_id = ::LibFred::Zone::CreateZone(zone, 6, 12).exec(_ctx);

        std::transform(zone.begin(), zone.end(), zone.begin(), ::tolower);

        info_zone_ns.nameserver_fqdn = "a.ns.nic." + zone;

        std::vector<boost::asio::ip::address> ns_ip_addrs;
        info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

    }

    ~InfoZoneNsFixture()
    {
    }
};


BOOST_FIXTURE_TEST_SUITE(TestInfoZoneNs, SupplyFixtureCtx<InfoZoneNsFixture>)

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    BOOST_CHECK_THROW(::LibFred::Zone::InfoZoneNs(info_zone_ns.id).exec(ctx),
           ::LibFred::Zone::NonExistentZoneNs);
}

BOOST_AUTO_TEST_CASE(set_min_info_zone_ns)
{
    info_zone_ns.id = ::LibFred::Zone::CreateZoneNs(zone, info_zone_ns.nameserver_fqdn).exec(ctx);

    ::LibFred::Zone::InfoZoneNsData zone_ns_info = ::LibFred::Zone::InfoZoneNs(info_zone_ns.id).exec(ctx);
    BOOST_CHECK(info_zone_ns == zone_ns_info);
}

BOOST_AUTO_TEST_CASE(set_all_info_zone_ns)
{
    std::vector<boost::asio::ip::address> ns_ip_addrs;
    ns_ip_addrs.push_back(boost::asio::ip::address::from_string("1.2.3.4"));
    info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

    info_zone_ns.id = ::LibFred::Zone::CreateZoneNs(zone, info_zone_ns.nameserver_fqdn)
            .set_nameserver_ip_addresses(info_zone_ns.nameserver_ip_addresses)
            .exec(ctx);

    ::LibFred::Zone::InfoZoneNsData zone_ns_info = ::LibFred::Zone::InfoZoneNs(info_zone_ns.id).exec(ctx);
    BOOST_CHECK(info_zone_ns == zone_ns_info);
}

BOOST_AUTO_TEST_CASE(set_more_addresses_info_zone_ns)
{
    std::vector<boost::asio::ip::address> ns_ip_addrs;
    ns_ip_addrs.push_back(boost::asio::ip::address::from_string("1.2.3.4"));
    ns_ip_addrs.push_back(boost::asio::ip::address::from_string("5.6.7.8"));
    ns_ip_addrs.push_back(boost::asio::ip::address::from_string("9.9.9.9"));
    info_zone_ns.nameserver_ip_addresses = ns_ip_addrs;

    info_zone_ns.id = ::LibFred::Zone::CreateZoneNs(zone, info_zone_ns.nameserver_fqdn)
            .set_nameserver_ip_addresses(info_zone_ns.nameserver_ip_addresses)
            .exec(ctx);

    ::LibFred::Zone::InfoZoneNsData zone_ns_info = ::LibFred::Zone::InfoZoneNs(info_zone_ns.id).exec(ctx);
    BOOST_CHECK(info_zone_ns == zone_ns_info);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
