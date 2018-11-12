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
#include "src/libfred/opcontext.hh"
#include "src/libfred/zone/exceptions.hh"
#include "test/backend/admin/zone/fixtures.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Zone)
BOOST_AUTO_TEST_SUITE(TestAdminAddZone)

std::size_t exists_zone(const std::string& _fqdn)
{
    ::LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT 1 FROM zone AS z "
            "INNER JOIN zone_soa AS zs ON (zs.zone = z.id) "
            "WHERE z.fqdn = $1::text ",
            // clang-format on
            Database::query_param_list(_fqdn));
    return db_result.size();
}

BOOST_FIXTURE_TEST_CASE(set_already_existing_zone, SupplyFixtureCtx<HasExistingZone>)
{
    BOOST_CHECK_THROW(
            ::Admin::Zone::add_zone(
                    zone.fqdn,
                    expiration_period_min,
                    expiration_period_max,
                    hostmaster,
                    ns_fqdn,
                    ttl,
                    refresh,
                    update_retr,
                    expiry,
                    minimum),
            ::LibFred::Zone::DuplicateZone);
}

BOOST_FIXTURE_TEST_CASE(set_enum_zone, SupplyFixtureCtx<HasEnumZone>)
{
    ::Admin::Zone::add_zone(
            zone.fqdn,
            expiration_period_min,
            expiration_period_max,
            hostmaster,
            ns_fqdn,
            ttl,
            refresh,
            update_retr,
            expiry,
            minimum);
    BOOST_CHECK_EQUAL(exists_zone(zone.fqdn), 1);
}

BOOST_FIXTURE_TEST_CASE(set_non_enum_zone, SupplyFixtureCtx<HasNonEnumZone>)
{
    ::Admin::Zone::add_zone(
            zone.fqdn,
            expiration_period_min,
            expiration_period_max,
            hostmaster,
            ns_fqdn,
            ttl,
            refresh,
            update_retr,
            expiry,
            minimum);
    BOOST_CHECK_EQUAL(exists_zone(zone.fqdn), 1);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
