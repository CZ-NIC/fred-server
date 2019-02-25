/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/whois/fixture_common.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_registrar_by_handle)

struct registrar_fixture
: whois_impl_instance_fixture
{
    ::LibFred::InfoRegistrarData registrar;

    registrar_fixture()
    {
        ::LibFred::OperationContextCreator ctx;
        registrar = Test::exec(
                Test::generate_test_data(
                    Test::CreateX_factory<::LibFred::CreateRegistrar>().make()),
                ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_fine_registrar, registrar_fixture)
{
    Fred::Backend::Whois::Registrar reg = impl.get_registrar_by_handle(registrar.handle);

    BOOST_CHECK(reg.organization            == registrar.organization.get_value_or_default());
    BOOST_CHECK(reg.fax                     == registrar.fax.get_value_or_default());
    BOOST_CHECK(reg.name                    == registrar.name.get_value_or_default());
    BOOST_CHECK(reg.handle                  == registrar.handle);
    BOOST_CHECK(reg.id                      == registrar.id);
    BOOST_CHECK(reg.phone                   == registrar.telephone.get_value_or_default());
    BOOST_CHECK(reg.url                     == registrar.url.get_value_or_default());
    BOOST_CHECK(reg.address.city            == registrar.city.get_value_or_default());
    BOOST_CHECK(reg.address.country_code    == registrar.country.get_value_or_default());
    BOOST_CHECK(reg.address.postal_code     == registrar.postalcode.get_value_or_default());
    BOOST_CHECK(reg.address.stateorprovince == registrar.stateorprovince.get_value_or_default());
    BOOST_CHECK(reg.address.street1         == registrar.street1.get_value_or_default());
    BOOST_CHECK(reg.address.street2         == registrar.street2.get_value_or_default());
    BOOST_CHECK(reg.address.street3         == registrar.street3.get_value_or_default());
}

BOOST_FIXTURE_TEST_CASE(get_no_registar, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_registrar_by_handle("REG-ABSENT"), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_wrong_registrar, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_registrar_by_handle("REG@#$"), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_registrar_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
