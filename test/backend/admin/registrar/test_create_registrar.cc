/*
 * Copyright (C) 2018-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/admin/registrar/create_registrar.hh"

#include "libfred/opcontext.hh"

#include "test/backend/admin/registrar/fixtures.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Registrar)
BOOST_AUTO_TEST_SUITE(TestAdminCreateRegistrar)

::LibFred::InfoRegistrarData getInfoRegistrar(const std::string& _handle)
{
    ::LibFred::OperationContextCreator ctx;
    const ::LibFred::InfoRegistrarOutput registrar_output = ::LibFred::InfoRegistrarByHandle(_handle).exec(ctx);
    return registrar_output.info_registrar_data;
}

BOOST_FIXTURE_TEST_CASE(set_already_existing_registrar, SupplyFixtureCtx<HasExistingRegistrarEmpty>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::create_registrar(registrar.handle,
                              contact.name,
                              merchant.organization,
                              address.street1,
                              address.street2,
                              address.street3,
                              address.city,
                              address.state_or_province,
                              address.postal_code,
                              address.country,
                              contact.telephone,
                              contact.fax,
                              contact.email,
                              contact.url,
                              system,
                              merchant.ico,
                              merchant.dic,
                              merchant.variable_symbol,
                              merchant.payment_memo_regex,
                              merchant.vat_payer,
                              false),
            ::Admin::Registrar::RegistrarAlreadyExists);
}

BOOST_FIXTURE_TEST_CASE(set_create_registrar_min, SupplyFixtureCtx<HasNonexistentRegistrarMin>)
{
    registrar.id = ::Admin::Registrar::create_registrar(registrar.handle,
            contact.name,
            merchant.organization,
            address.street1,
            address.street2,
            address.street3,
            address.city,
            address.state_or_province,
            address.postal_code,
            address.country,
            contact.telephone,
            contact.fax,
            contact.email,
            contact.url,
            system,
            merchant.ico,
            merchant.dic,
            merchant.variable_symbol,
            merchant.payment_memo_regex,
            merchant.vat_payer,
            false);

    BOOST_CHECK(registrar == getInfoRegistrar(registrar.handle));
}

BOOST_FIXTURE_TEST_CASE(set_create_registrar_max, SupplyFixtureCtx<HasNonexistentRegistrarMax>)
{
    registrar.id = ::Admin::Registrar::create_registrar(registrar.handle,
            contact.name,
            merchant.organization,
            address.street1,
            address.street2,
            address.street3,
            address.city,
            address.state_or_province,
            address.postal_code,
            address.country,
            contact.telephone,
            contact.fax,
            contact.email,
            contact.url,
            system,
            merchant.ico,
            merchant.dic,
            merchant.variable_symbol,
            merchant.payment_memo_regex,
            merchant.vat_payer,
            false);

    BOOST_CHECK(registrar == getInfoRegistrar(registrar.handle));
}

BOOST_FIXTURE_TEST_CASE(set_create_system_registrar, SupplyFixtureCtx<HasSystemRegistrar>)
{
    registrar.id = ::Admin::Registrar::create_registrar(registrar.handle,
            contact.name,
            merchant.organization,
            address.street1,
            address.street2,
            address.street3,
            address.city,
            address.state_or_province,
            address.postal_code,
            address.country,
            contact.telephone,
            contact.fax,
            contact.email,
            contact.url,
            system,
            merchant.ico,
            merchant.dic,
            merchant.variable_symbol,
            merchant.payment_memo_regex,
            merchant.vat_payer,
            internal);

    BOOST_CHECK(registrar == getInfoRegistrar(registrar.handle));
}

BOOST_FIXTURE_TEST_CASE(set_create_internal_registrar, SupplyFixtureCtx<HasInternalRegistrar>)
{
    BOOST_REQUIRE(!system);
    BOOST_REQUIRE(internal);
    registrar.id = ::Admin::Registrar::create_registrar(registrar.handle,
            contact.name,
            merchant.organization,
            address.street1,
            address.street2,
            address.street3,
            address.city,
            address.state_or_province,
            address.postal_code,
            address.country,
            contact.telephone,
            contact.fax,
            contact.email,
            contact.url,
            system,
            merchant.ico,
            merchant.dic,
            merchant.variable_symbol,
            merchant.payment_memo_regex,
            merchant.vat_payer,
            internal);

    BOOST_CHECK(registrar == getInfoRegistrar(registrar.handle));
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Admin/Registrar/TestAdminCreateRegistrar
BOOST_AUTO_TEST_SUITE_END()//Backend/Admin/Registrar
BOOST_AUTO_TEST_SUITE_END()//Backend/Admin
BOOST_AUTO_TEST_SUITE_END()//Backend

} // namespace Test
