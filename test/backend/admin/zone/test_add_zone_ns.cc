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

#include "src/backend/admin/zone/zone.hh"
#include "src/libfred/zone/exceptions.hh"
#include "src/libfred/zone/zone_ns/util.hh"
#include "test/backend/admin/zone/fixtures.hh"
#include "test/setup/fixtures.hh"

#include <boost/asio.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Zone)
BOOST_AUTO_TEST_SUITE(TestAdminAddZoneNs)

std::size_t exists_zone_ns(
        const unsigned long long _zone_id,
        const std::string& _fqdn,
        const std::vector<boost::asio::ip::address>& _addrs)
{
    ::LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT 1 FROM zone_ns AS zn "
            "WHERE zn.zone = $1::bigint "
            "AND zn.fqdn = $2::text "
            "AND zn.addrs = $3::inet[] ",
            // clang-format on
            Database::query_param_list(_zone_id)(_fqdn)(::LibFred::Zone::ip_addresses_to_string(_addrs)));
    return db_result.size();
}

BOOST_FIXTURE_TEST_CASE(set_nonexistent_zone, SupplyFixtureCtx<HasNonExistentZone>)
{
    BOOST_CHECK_THROW(
            ::Admin::Zone::add_zone_ns(zone.fqdn, nameserver_fqdn, nameserver_ip_addresses),
            ::LibFred::Zone::NonExistentZone);
}

BOOST_FIXTURE_TEST_CASE(set_more_zone_ns_records, SupplyFixtureCtx<HasMoreRecords>)
{
    ::Admin::Zone::add_zone_ns(zone.fqdn, nameserver_fqdn_0, nameserver_ip_addresses);
    ::Admin::Zone::add_zone_ns(zone.fqdn, nameserver_fqdn_1, nameserver_ip_addresses);
    ::Admin::Zone::add_zone_ns(zone.fqdn, nameserver_fqdn_2, nameserver_ip_addresses);
    BOOST_CHECK_EQUAL(exists_zone_ns(zone_id, nameserver_fqdn_0, nameserver_ip_addresses), 1);
    BOOST_CHECK_EQUAL(exists_zone_ns(zone_id, nameserver_fqdn_1, nameserver_ip_addresses), 1);
    BOOST_CHECK_EQUAL(exists_zone_ns(zone_id, nameserver_fqdn_2, nameserver_ip_addresses), 1);
}

BOOST_FIXTURE_TEST_CASE(set_empty_ip_address, SupplyFixtureCtx<HasEmptyIpAddress>)
{
    ::Admin::Zone::add_zone_ns(zone.fqdn, nameserver_fqdn, nameserver_ip_addresses);
    BOOST_CHECK_EQUAL(exists_zone_ns(zone_id, nameserver_fqdn, nameserver_ip_addresses), 1);
}
BOOST_FIXTURE_TEST_CASE(set_one_ip_address, SupplyFixtureCtx<HasOneIpAddress>)
{
    ::Admin::Zone::add_zone_ns(zone.fqdn, nameserver_fqdn, nameserver_ip_addresses);
    BOOST_CHECK_EQUAL(exists_zone_ns(zone_id, nameserver_fqdn, nameserver_ip_addresses), 1);
}

BOOST_FIXTURE_TEST_CASE(set_more_ip_addresses, SupplyFixtureCtx<HasMoreIpAddresses>)
{
    ::Admin::Zone::add_zone_ns(zone.fqdn, nameserver_fqdn, nameserver_ip_addresses);
    BOOST_CHECK_EQUAL(exists_zone_ns(zone_id, nameserver_fqdn, nameserver_ip_addresses), 1);
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
