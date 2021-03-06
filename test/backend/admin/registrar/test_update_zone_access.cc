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
#include "src/backend/admin/registrar/update_zone_access.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/zone_access/get_zone_access_history.hh"
#include "test/backend/admin/registrar/fixtures.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <vector>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Registrar)
BOOST_AUTO_TEST_SUITE(TestUpdateZoneAccess)

std::vector<::LibFred::Registrar::ZoneAccess::ZoneAccess> getNewZoneAccess(const std::string& _handle)
{
    ::LibFred::OperationContextCreator ctx;
    const ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccessHistory zone_access =
            ::LibFred::Registrar::ZoneAccess::GetZoneAccessHistory(_handle).exec(ctx);
    std::vector<::LibFred::Registrar::ZoneAccess::ZoneAccess> new_accesses;
    for (const auto& zone_invoices : zone_access.invoices_by_zone)
    {
        for (const auto& date_invoice_id : zone_invoices.second)
        {
            ::LibFred::Registrar::ZoneAccess::ZoneAccess access;
            access.zone_fqdn = zone_invoices.first;
            access.id = get_raw_value_from(date_invoice_id.second);
            access.from_date = date_invoice_id.first.from_date;
            if (date_invoice_id.first.to_date != boost::none)
            {
                access.to_date = *date_invoice_id.first.to_date;
            }
            new_accesses.push_back(access);
        }
    }
    return new_accesses;
}

BOOST_FIXTURE_TEST_CASE(set_noexistent_registrar, SupplyFixtureCtx<HasZoneAccessWithNonexistentRegistrar>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_zone_access(accesses),
            ::Admin::Registrar::ZoneAccessNonexistentRegistrar);
}

BOOST_FIXTURE_TEST_CASE(set_nonexistent_zone, SupplyFixtureCtx<HasZoneAccessWithNonexistentZone>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_zone_access(accesses),
            ::Admin::Registrar::NonexistentZone);
}

BOOST_FIXTURE_TEST_CASE(set_nonexistent_zone_access, SupplyFixtureCtx<HasNonexistentZoneAccess>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_zone_access(accesses),
            ::Admin::Registrar::NonexistentZoneAccess);
}

BOOST_FIXTURE_TEST_CASE(set_no_update_data, SupplyFixtureCtx<HasNoUpdateData>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_zone_access(accesses),
            ::Admin::Registrar::ZoneAccessNoUpdateData);
}

BOOST_FIXTURE_TEST_CASE(set_missing_params, SupplyFixtureCtx<HasMissingParams>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_zone_access(accesses),
            ::Admin::Registrar::ZoneAccessMissingParameters);
}

BOOST_FIXTURE_TEST_CASE(set_overlapping_range, SupplyFixtureCtx<HasAddZoneAccessMax>)
{
    ::Admin::Registrar::update_zone_access(accesses);
    BOOST_CHECK_THROW(::Admin::Registrar::update_zone_access(accesses),
            ::Admin::Registrar::UpdateZoneAccessException);
}

BOOST_FIXTURE_TEST_CASE(set_zone_access_empty, SupplyFixtureCtx<HasZoneAccessEmpty>)
{
    ::Admin::Registrar::update_zone_access(accesses);
    const std::vector<::LibFred::Registrar::ZoneAccess::ZoneAccess>& new_accesses =
            getNewZoneAccess(accesses.registrar_handle);
    BOOST_CHECK(accesses.zone_accesses.size() == new_accesses.size());
    for (unsigned i = 0; i < new_accesses.size(); ++i)
    {
        BOOST_CHECK(accesses.zone_accesses[i] == new_accesses[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(set_add_zone_access_max, SupplyFixtureCtx<HasAddZoneAccessMax>)
{
    ::Admin::Registrar::update_zone_access(accesses);
    const std::vector<::LibFred::Registrar::ZoneAccess::ZoneAccess>& new_accesses =
            getNewZoneAccess(accesses.registrar_handle);
    BOOST_CHECK(accesses.zone_accesses.size() == new_accesses.size());
    for (unsigned i = 0; i < new_accesses.size(); ++i)
    {
        BOOST_CHECK(accesses.zone_accesses[i] == new_accesses[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(set_update_zone_access_max, SupplyFixtureCtx<HasUpdateZoneAccessMax>)
{
    ::Admin::Registrar::update_zone_access(accesses);
    const std::vector<::LibFred::Registrar::ZoneAccess::ZoneAccess>& new_accesses =
            getNewZoneAccess(accesses.registrar_handle);
    BOOST_CHECK(accesses.zone_accesses.size() == new_accesses.size());
    for (unsigned i = 0; i < new_accesses.size(); ++i)
    {
        BOOST_CHECK(accesses.zone_accesses[i] == new_accesses[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(set_more_zone_accesses, SupplyFixtureCtx<HasMoreZoneAccesses>)
{
    ::Admin::Registrar::update_zone_access(accesses);
    const std::vector<::LibFred::Registrar::ZoneAccess::ZoneAccess>& new_accesses =
            getNewZoneAccess(accesses.registrar_handle);
    BOOST_CHECK(accesses.zone_accesses.size() == new_accesses.size());
    for (unsigned i = 0; i < new_accesses.size(); ++i)
    {
        BOOST_CHECK(accesses.zone_accesses[i] == new_accesses[i]);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
