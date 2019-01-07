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
#include "src/libfred/registrar/create_registrar.hh"
#include "src/libfred/registrar/exceptions.hh"
#include "src/libfred/registrar/info_registrar_data.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/libfred/registrar/update_registrar.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/registrar/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct update_registrar_fixture
{
    ::LibFred::InfoRegistrarData registrar;

    update_registrar_fixture(::LibFred::OperationContext& _ctx)
    {
        registrar.handle = RandomDataGenerator().xstring(12);
        registrar.name = Nullable<std::string>("Testname");
        registrar.street1 = Nullable<std::string>("STR1");
        registrar.street2 = Nullable<std::string>("str2");
        registrar.street3 = Nullable<std::string>("str3");
        registrar.stateorprovince = Nullable<std::string>("");
        registrar.city = Nullable<std::string>("Praha");
        registrar.postalcode = Nullable<std::string>("11150");
        registrar.country = Nullable<std::string>("CZ");
        registrar.dic = Nullable<std::string>("5555551234");
        registrar.email = Nullable<std::string>("test@nic.cz");
        registrar.fax = Nullable<std::string>("132456789");
        registrar.ico = Nullable<std::string>("1234567890");
        registrar.organization = Nullable<std::string>("org");
        registrar.payment_memo_regex = Nullable<std::string>("");
        registrar.system = Nullable<bool>(false);
        registrar.telephone = Nullable<std::string>("123456789");
        registrar.url = Nullable<std::string>("http://test.nic.cz");
        registrar.variable_symbol = Nullable<std::string>("1234567890");
        registrar.vat_payer = true;

        ::LibFred::CreateRegistrar(registrar.handle).exec(_ctx);
    }

    ~update_registrar_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateRegistrar, SupplyFixtureCtx<update_registrar_fixture>)

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar)
{
    const std::string& nonexistentRegistrar = RandomDataGenerator().xstring(13);
    BOOST_CHECK_THROW(::LibFred::Registrar::UpdateRegistrar(nonexistentRegistrar)
            .set_name(registrar.name.get_value())
            .exec(ctx),
           ::LibFred::Registrar::NonExistentRegistrar);
}

BOOST_AUTO_TEST_CASE(set_no_update_registrar_data)
{
    BOOST_CHECK_THROW(::LibFred::Registrar::UpdateRegistrar(registrar.handle)
                .exec(ctx),
           ::LibFred::Registrar::NoUpdateData);
}

BOOST_AUTO_TEST_CASE(set_registrar_update_all)
{
    registrar.id = ::LibFred::Registrar::UpdateRegistrar(registrar.handle)
            .set_name(registrar.name.get_value())
            .set_street1(registrar.street1.get_value())
            .set_street2(registrar.street2.get_value())
            .set_street3(registrar.street3.get_value())
            .set_state_or_province(registrar.stateorprovince.get_value())
            .set_city(registrar.city.get_value())
            .set_postal_code(registrar.postalcode.get_value())
            .set_country(registrar.country.get_value())
            .set_dic(registrar.dic.get_value())
            .set_email(registrar.email.get_value())
            .set_fax(registrar.fax.get_value())
            .set_ico(registrar.ico.get_value())
            .set_organization(registrar.organization.get_value())
            .set_payment_memo_regex(registrar.payment_memo_regex.get_value())
            .set_system(registrar.system.get_value())
            .set_telephone(registrar.telephone.get_value())
            .set_url(registrar.url.get_value())
            .set_variable_symbol(registrar.variable_symbol.get_value())
            .set_vat_payer(registrar.vat_payer)
            .exec(ctx);

    ::LibFred::InfoRegistrarOutput registrar_info = ::LibFred::InfoRegistrarByHandle(registrar.handle).exec(ctx);

    BOOST_CHECK(registrar == registrar_info.info_registrar_data);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
