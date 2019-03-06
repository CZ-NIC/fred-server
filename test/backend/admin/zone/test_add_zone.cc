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
#include "src/backend/admin/zone/zone.hh"

#include "libfred/opcontext.hh"
#include "libfred/zone/exceptions.hh"

#include "test/backend/admin/zone/fixtures.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Zone)
BOOST_AUTO_TEST_SUITE(TestAdminAddZone)

std::size_t get_number_of_zones(const std::string& _fqdn)
{
    ::LibFred::OperationContextCreator ctx;
    return ctx.get_conn().exec_params(
            // clang-format off
            "SELECT 1 "
            "FROM zone AS z "
            "JOIN zone_soa AS zs ON zs.zone=z.id "
            "WHERE z.fqdn=$1::TEXT",
            // clang-format on
            Database::query_param_list(_fqdn)).size();
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
    BOOST_CHECK_EQUAL(get_number_of_zones(zone.fqdn), 1);
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
    BOOST_CHECK_EQUAL(get_number_of_zones(zone.fqdn), 1);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Admin/Zone/TestAdminAddZone
BOOST_AUTO_TEST_SUITE_END()//Backend/Admin/Zone
BOOST_AUTO_TEST_SUITE_END()//Backend/Admin
BOOST_AUTO_TEST_SUITE_END()//Backend

} // namespace Test
