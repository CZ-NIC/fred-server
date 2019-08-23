/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#include <string>
#include <utility>
#include <iostream>

#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"
#include "libfred/opexception.hh"
#include "util/util.hh"

#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"

/**
 *  @file
 *  test create registrar
 */

#include <boost/test/unit_test.hpp>

#include "test/setup/fixtures.hh"

const std::string server_name = "test-create-registrar";

struct test_registrar_fixture : virtual public Test::instantiate_db_template
{
    std::string xmark;
    std::string test_registrar_handle;
    ::LibFred::InfoRegistrarData test_info;

    test_registrar_fixture()
    :xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , test_registrar_handle(std::string("TEST-REGISTRAR-HANDLE")+xmark)
    {
        test_info.handle = test_registrar_handle;
        test_info.name = Nullable<std::string>(std::string("TEST-REGISTRAR NAME")+xmark);
        test_info.street1 = Nullable<std::string>(std::string("STR1")+xmark);
        test_info.street2 = Nullable<std::string>("str2");
        test_info.street3 = Nullable<std::string>("str3");
        test_info.stateorprovince = Nullable<std::string>("");
        test_info.city = Nullable<std::string>("Praha");
        test_info.postalcode = Nullable<std::string>("11150");
        test_info.country = Nullable<std::string>("CZ");
        test_info.dic = Nullable<std::string>("5555551234");
        test_info.email = Nullable<std::string>("test@nic.cz");
        test_info.fax = Nullable<std::string>("132456789");
        test_info.ico = Nullable<std::string>("1234567890");
        test_info.organization = Nullable<std::string>("org");
        test_info.payment_memo_regex = Nullable<std::string>("");
        test_info.system = Nullable<bool>(false);
        test_info.telephone = Nullable<std::string>("123456789");
        test_info.url = Nullable<std::string>("http://test.nic.cz");
        test_info.variable_symbol = Nullable<std::string>("1234567890");
        test_info.vat_payer = true;
    }
    ~test_registrar_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateRegistrar, test_registrar_fixture)

/**
 * test CreateRegistrar
*/
BOOST_AUTO_TEST_CASE(create_registrar)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::CreateRegistrar(test_info.handle)
        .set_name(test_info.name.get_value())
        .set_street1(test_info.street1.get_value())
        .set_street2(test_info.street2.get_value())
        .set_street3(test_info.street3.get_value())
        .set_stateorprovince(test_info.stateorprovince.get_value())
        .set_city(test_info.city.get_value()).set_postalcode(test_info.postalcode.get_value())
        .set_country(test_info.country.get_value())
        .set_dic(test_info.dic.get_value())
        .set_email(test_info.email.get_value())
        .set_fax(test_info.fax.get_value())
        .set_ico(test_info.ico.get_value())
        .set_organization(test_info.organization.get_value())
        .set_payment_memo_regex(test_info.payment_memo_regex.get_value())
        .set_system(test_info.system.get_value())
        .set_telephone(test_info.telephone.get_value())
        .set_url(test_info.url.get_value())
        .set_variable_symbol(test_info.variable_symbol.get_value())
        .set_vat_payer(test_info.vat_payer)
        .exec(ctx);

    ::LibFred::InfoRegistrarOutput registrar_info = ::LibFred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);

    test_info.id = registrar_info.info_registrar_data.id;

    if(test_info != registrar_info.info_registrar_data)
    {
        BOOST_TEST_MESSAGE(::LibFred::diff_registrar_data(test_info,registrar_info.info_registrar_data).to_string());
    }

    BOOST_CHECK(test_info == registrar_info.info_registrar_data);
}

/**
 * test CreateRegistrar invalid handle
 */
BOOST_AUTO_TEST_CASE(create_registrar_invalid_handle)
{

    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateRegistrar(test_registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateRegistrar(test_registrar_handle).set_email("test1@nic.cz").exec(ctx);
        BOOST_ERROR("unreported invalid_registrar_handle");
    }
    catch(const ::LibFred::CreateRegistrar::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_registrar_handle());
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_invalid_registrar_handle().compare(test_registrar_handle) == 0);
    }

    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoRegistrarOutput registrar_info = ::LibFred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);
        BOOST_CHECK(test_info.handle == registrar_info.info_registrar_data.handle);
    }

}

/**
 * test CreateRegistrar unknown country
 */
BOOST_AUTO_TEST_CASE(create_registrar_unknown_country)
{
    ::LibFred::OperationContextCreator ctx;

    try
    {
        ::LibFred::CreateRegistrar(test_registrar_handle).set_country("XY").exec(ctx);
        BOOST_ERROR("unreported unknown country");
    }
    catch(const ::LibFred::CreateRegistrar::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_country());
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_country().compare("XY") == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestCreateRegistrar
