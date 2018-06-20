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

#include "src/libfred/zone_ns/create_zone_ns.hh"
#include "src/libfred/zone/create_zone.hh"
#include "src/libfred/zone/util.hh"
#include "test/libfred/zone/util.hh"
#include "test/setup/fixtures.hh"
#include "src/util/random_data_generator.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/ip/address.hpp>
#include <vector>
#include <string>

namespace Test {

struct CreateZoneNsFixture
{
    std::string zone;

    CreateZoneNsFixture(::LibFred::OperationContext& _ctx)
        : zone(RandomDataGenerator().xstring(3))
    {
    }

    ~CreateZoneNsFixture()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(TestCreateZoneNs, SupplyFixtureCtx<CreateZoneNsFixture>)

size_t exists_new_zone_ns(const std::string& _zone, ::LibFred::OperationContext& _ctx)
{
    const Database::Result db_result = _ctx.get_conn().exec_params(
            "SELECT 1 FROM zone_ns AS zn "
            "JOIN zone AS z ON z.id=zn.zone "
            "WHERE z.fqdn = LOWER($1::text)",
            Database::query_param_list(_zone));
    return db_result.size();
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    BOOST_CHECK_THROW(::LibFred::ZoneNs::CreateZoneNs(zone)
            .exec(ctx),
            ::LibFred::Zone::NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_min_create_zone_ns)
{
    ::LibFred::Zone::CreateZone(zone, 6, 12).exec(ctx);
    ::LibFred::ZoneNs::CreateZoneNs(zone).exec(ctx);
    BOOST_CHECK_EQUAL(exists_new_zone_ns(zone, ctx), 1);
}

BOOST_AUTO_TEST_CASE(set_max_create_zone_ns)
{
    std::vector<boost::asio::ip::address> ns_ip_addrs;
    ::LibFred::Zone::CreateZone(zone, 6, 12).exec(ctx);
    ::LibFred::ZoneNs::CreateZoneNs(zone)
            .set_nameserver_fqdn("a.ns.nic." + zone)
            .exec(ctx);
    ns_ip_addrs.push_back(boost::asio::ip::address::from_string("1.2.3.4"));
    ::LibFred::ZoneNs::CreateZoneNs(zone)
            .set_nameserver_fqdn("b.ns.nic." + zone)
            .set_nameserver_ip_addresses(ns_ip_addrs)
            .exec(ctx);
    ns_ip_addrs.push_back(boost::asio::ip::address::from_string("5.6.7.8"));
    ns_ip_addrs.push_back(boost::asio::ip::address::from_string("9.9.9.9"));
    ::LibFred::ZoneNs::CreateZoneNs(zone)
            .set_nameserver_fqdn("c.ns.nic." + zone)
            .set_nameserver_ip_addresses(ns_ip_addrs)
            .exec(ctx);
    BOOST_CHECK_EQUAL(exists_new_zone_ns(zone, ctx), 3);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
