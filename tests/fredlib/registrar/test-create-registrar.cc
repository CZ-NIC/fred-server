/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#include <string>
#include <utility>

#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar_diff.h"
#include "src/fredlib/opexception.h"
#include "util/util.h"

#include "random_data_generator.h"

/**
 *  @file
 *  test create registrar
 */

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestCreateRegistrar)

const std::string server_name = "test-create-registrar";


struct test_registrar_fixture
{
    std::string xmark;
    std::string test_registrar_handle;
    Fred::InfoRegistrarData test_info;

    test_registrar_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
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
        test_info.vat_payer = Nullable<bool>(true);
    }
    ~test_registrar_fixture()
    {}
};

/**
 * test CreateRegistrar
*/
BOOST_FIXTURE_TEST_CASE(create_registrar, test_registrar_fixture )
{
    Fred::OperationContext ctx;

    Fred::CreateRegistrar(test_info.handle)
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
        .set_vat_payer(test_info.vat_payer.get_value())
        .exec(ctx);

    Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);

    test_info.id = registrar_info.info_registrar_data.id;

    if(test_info != registrar_info.info_registrar_data)
    {
        BOOST_MESSAGE(Fred::diff_registrar_data(test_info,registrar_info.info_registrar_data).to_string());
    }

    BOOST_CHECK(test_info == registrar_info.info_registrar_data);
}

/**
 * test CreateRegistrar invalid handle
 */
BOOST_FIXTURE_TEST_CASE(create_registrar_invalid_handle, test_registrar_fixture )
{

    Fred::OperationContext ctx;
    Fred::CreateRegistrar(test_registrar_handle).exec(ctx);
    ctx.commit_transaction();

    try
    {
        Fred::OperationContext ctx;
        Fred::CreateRegistrar(test_registrar_handle).set_email("test1@nic.cz").exec(ctx);
        BOOST_ERROR("unreported invalid_registrar_handle");
    }
    catch(const Fred::CreateRegistrar::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_registrar_handle());
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_invalid_registrar_handle().compare(test_registrar_handle) == 0);
    }

    Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);
    BOOST_CHECK(test_info.handle == registrar_info.info_registrar_data.handle);

}

/**
 * test CreateRegistrar unknown country
 */
BOOST_FIXTURE_TEST_CASE(create_registrar_unknown_country, test_registrar_fixture )
{
    Fred::OperationContext ctx;

    try
    {
        Fred::CreateRegistrar(test_registrar_handle).set_country("XY").exec(ctx);
        BOOST_ERROR("unreported unknown country");
    }
    catch(const Fred::CreateRegistrar::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_country());
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_country().compare("XY") == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestCreateRegistrar
