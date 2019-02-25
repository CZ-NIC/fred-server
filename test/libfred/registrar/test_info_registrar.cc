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

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"
#include "libfred/registrar/info_registrar_impl.hh"
#include "libfred/opexception.hh"
#include "libfred/opcontext.hh"
#include "util/util.hh"

#include "util/random_data_generator.hh"

/**
 *  @file
 *  test registrar info
 */

#include <boost/test/unit_test.hpp>

#include "test/setup/fixtures.hh"


struct test_info_registrar_fixture : virtual public Test::instantiate_db_template
{
    std::string xmark;
    ::LibFred::InfoRegistrarData test_registrar_data_1;
    ::LibFred::InfoRegistrarData test_registrar_data_2;

    test_info_registrar_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    {
        test_registrar_data_1.handle = std::string("TEST-REGISTRAR1-HANDLE")+xmark;
        test_registrar_data_1.name = std::string("TEST-REGISTRAR NAME1")+xmark;
        test_registrar_data_1.organization = std::string("TEST-REGISTRAR ORG1")+xmark;
        test_registrar_data_1.street1 = std::string("STR11")+xmark;
        test_registrar_data_1.street2 = std::string("STR21")+xmark;
        test_registrar_data_1.street3 = std::string("STR31")+xmark;
        test_registrar_data_1.city = "Praha 1";
        test_registrar_data_1.stateorprovince = "State1";
        test_registrar_data_1.postalcode = "11150";
        test_registrar_data_1.country = "CZ";
        test_registrar_data_1.telephone = "+420.728123456";
        test_registrar_data_1.fax = "+420.728123457";
        test_registrar_data_1.email = "test1@nic.cz";
        test_registrar_data_1.url = "www.test1.com";
        test_registrar_data_1.system = false;
        test_registrar_data_1.ico = "1023456789";
        test_registrar_data_1.dic = "1123456789";
        test_registrar_data_1.variable_symbol = "123456789";
        test_registrar_data_1.payment_memo_regex = "test-registrar1*";
        test_registrar_data_1.vat_payer = true;

        test_registrar_data_2.handle = std::string("TEST-REGISTRAR2-HANDLE")+xmark;
        test_registrar_data_2.name = std::string("TEST-REGISTRAR NAME2")+xmark;
        test_registrar_data_2.organization = std::string("TEST-REGISTRAR ORG2")+xmark;
        test_registrar_data_2.street1 = std::string("STR12")+xmark;
        test_registrar_data_2.street2 = std::string("STR22")+xmark;
        test_registrar_data_2.street3 = std::string("STR32")+xmark;
        test_registrar_data_2.city = "Praha 2";
        test_registrar_data_2.stateorprovince = "State2";
        test_registrar_data_2.postalcode = "21150";
        test_registrar_data_2.country = "SK";
        test_registrar_data_2.telephone = "+420.728123458";
        test_registrar_data_2.fax = "+420.728123459";
        test_registrar_data_2.email = "test2@nic.cz";
        test_registrar_data_2.url = "www.test2.com";
        test_registrar_data_2.system = true;
        test_registrar_data_2.ico = "2023456789";
        test_registrar_data_2.dic = "2123456789";
        test_registrar_data_2.variable_symbol = "223456789";
        test_registrar_data_2.payment_memo_regex = "test-registrar2*";
        test_registrar_data_2.vat_payer = false;


        ::LibFred::OperationContextCreator ctx;

        ctx.get_conn().exec("TRUNCATE TABLE registrar CASCADE;");

        ::LibFred::CreateRegistrar(test_registrar_data_1.handle)
            .set_name(test_registrar_data_1.name.get_value())
            .set_organization(test_registrar_data_1.organization.get_value())
            .set_street1(test_registrar_data_1.street1.get_value())
            .set_street2(test_registrar_data_1.street2.get_value())
            .set_street3(test_registrar_data_1.street3.get_value())
            .set_city(test_registrar_data_1.city.get_value())
            .set_stateorprovince(test_registrar_data_1.stateorprovince.get_value())
            .set_postalcode(test_registrar_data_1.postalcode.get_value())
            .set_country(test_registrar_data_1.country.get_value())
            .set_telephone(test_registrar_data_1.telephone.get_value())
            .set_fax(test_registrar_data_1.fax.get_value())
            .set_email(test_registrar_data_1.email.get_value())
            .set_url(test_registrar_data_1.url.get_value())
            .set_system(test_registrar_data_1.system.get_value())
            .set_ico(test_registrar_data_1.ico.get_value())
            .set_dic(test_registrar_data_1.dic.get_value())
            .set_variable_symbol(test_registrar_data_1.variable_symbol.get_value())
            .set_payment_memo_regex(test_registrar_data_1.payment_memo_regex.get_value())
            .set_vat_payer(test_registrar_data_1.vat_payer)
            .exec(ctx);

        test_registrar_data_1.id = static_cast<unsigned long long>(
            ctx.get_conn().exec_params(Database::ParamQuery(
                "SELECT id FROM registrar WHERE handle = ")
                    .param_text(test_registrar_data_1.handle))[0]["id"]);

        ::LibFred::CreateRegistrar(test_registrar_data_2.handle)
            .set_name(test_registrar_data_2.name.get_value())
            .set_organization(test_registrar_data_2.organization.get_value())
            .set_street1(test_registrar_data_2.street1.get_value())
            .set_street2(test_registrar_data_2.street2.get_value())
            .set_street3(test_registrar_data_2.street3.get_value())
            .set_city(test_registrar_data_2.city.get_value())
            .set_stateorprovince(test_registrar_data_2.stateorprovince.get_value())
            .set_postalcode(test_registrar_data_2.postalcode.get_value())
            .set_country(test_registrar_data_2.country.get_value())
            .set_telephone(test_registrar_data_2.telephone.get_value())
            .set_fax(test_registrar_data_2.fax.get_value())
            .set_email(test_registrar_data_2.email.get_value())
            .set_url(test_registrar_data_2.url.get_value())
            .set_system(test_registrar_data_2.system.get_value())
            .set_ico(test_registrar_data_2.ico.get_value())
            .set_dic(test_registrar_data_2.dic.get_value())
            .set_variable_symbol(test_registrar_data_2.variable_symbol.get_value())
            .set_payment_memo_regex(test_registrar_data_2.payment_memo_regex.get_value())
            .set_vat_payer(test_registrar_data_2.vat_payer)
            .exec(ctx);

        test_registrar_data_2.id = static_cast<unsigned long long>(
            ctx.get_conn().exec_params(Database::ParamQuery(
                "SELECT id FROM registrar WHERE handle = ")
                    .param_text(test_registrar_data_2.handle))[0]["id"]);


        ctx.commit_transaction();//commit fixture
    }
    ~test_info_registrar_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestInfoRegistrar, test_info_registrar_fixture)

/**
 * test call InfoRegistrar
*/
BOOST_AUTO_TEST_CASE(info_registrar)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoRegistrarOutput registrar_info1 = ::LibFred::InfoRegistrarByHandle(test_registrar_data_1.handle).exec(ctx);
    BOOST_CHECK(registrar_info1.info_registrar_data == test_registrar_data_1);

    ::LibFred::InfoRegistrarOutput registrar_info2 = ::LibFred::InfoRegistrarByHandle(test_registrar_data_1.handle).set_lock().exec(ctx);
    BOOST_CHECK(registrar_info2.info_registrar_data == test_registrar_data_1);

    BOOST_TEST_MESSAGE(std::string("test registrar id: ") + boost::lexical_cast<std::string>(test_registrar_data_1.id));

    ::LibFred::InfoRegistrarOutput registrar_info3 = ::LibFred::InfoRegistrarById(test_registrar_data_1.id).exec(ctx);
    BOOST_CHECK(registrar_info3.info_registrar_data == test_registrar_data_1);

    std::vector<::LibFred::InfoRegistrarOutput> nosystem_registrar_infos = ::LibFred::InfoRegistrarAllExceptSystem().exec(ctx);

    BOOST_CHECK(nosystem_registrar_infos.size() == 1 );

    BOOST_CHECK(nosystem_registrar_infos.at(0).info_registrar_data == test_registrar_data_1);
}

BOOST_AUTO_TEST_CASE(test_info_registrar_output_timestamp)
{
    const std::string timezone = "Europe/Prague";
    ::LibFred::OperationContextCreator ctx;
    const ::LibFred::InfoRegistrarOutput registrar_output_by_handle = ::LibFred::InfoRegistrarByHandle(test_registrar_data_1.handle).exec(ctx, timezone);
    const ::LibFred::InfoRegistrarOutput registrar_output_by_id     = ::LibFred::InfoRegistrarById(registrar_output_by_handle.info_registrar_data.id).exec(ctx, timezone);

    /* all are equal amongst themselves ... */
    BOOST_CHECK(
            registrar_output_by_handle.utc_timestamp    == registrar_output_by_id.utc_timestamp
    );

    /* ... and one of them is equal to correct constant value */
    BOOST_CHECK_EQUAL(
        registrar_output_by_handle.utc_timestamp,
        boost::posix_time::time_from_string( static_cast<std::string>( ctx.get_conn().exec("SELECT now()::timestamp")[0][0] ) )
    );
}

/**
 * test wrong handle with InfoRegistrarByHandle
 */
BOOST_AUTO_TEST_CASE(info_registrar_wrong_handle)
{
    ::LibFred::OperationContextCreator ctx;
    std::string bad_registrar_handle = "BAD_REGISTRAR_HANDLE";

    try
    {
        ::LibFred::InfoRegistrarByHandle(bad_registrar_handle).exec(ctx);
        BOOST_ERROR("unreported bad_registrar_handle");
    }
    catch(const ::LibFred::InfoRegistrarByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_TEST_MESSAGE(bad_registrar_handle);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }
}

/**
 * test wrong id with InfoRegistrarById
 */
BOOST_AUTO_TEST_CASE(info_registrar_wrong_id)
{
    ::LibFred::OperationContextCreator ctx;
    unsigned long long bad_registrar_id = 0;

    try
    {
        ::LibFred::InfoRegistrarById(bad_registrar_id).exec(ctx);
        BOOST_ERROR("unreported bad_registrar_id");
    }
    catch(const ::LibFred::InfoRegistrarById::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_id());
        BOOST_TEST_MESSAGE(bad_registrar_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_registrar_id() == bad_registrar_id);
    }
}


/**
 * test call InfoRegistrarDiff
*/
BOOST_AUTO_TEST_CASE(info_registrar_diff)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoRegistrarOutput registrar_info1 = ::LibFred::InfoRegistrarByHandle(test_registrar_data_1.handle).exec(ctx);
    ::LibFred::InfoRegistrarOutput registrar_info2 = ::LibFred::InfoRegistrarByHandle(test_registrar_data_2.handle).set_lock().exec(ctx);

    ::LibFred::InfoRegistrarDiff test_diff, test_empty_diff;

    //differing data
    test_diff.id = std::make_pair(test_registrar_data_1.id,test_registrar_data_2.id);
    test_diff.handle = std::make_pair(test_registrar_data_1.handle,test_registrar_data_2.handle);
    test_diff.name = std::make_pair(test_registrar_data_1.name,test_registrar_data_2.name);
    test_diff.organization = std::make_pair(test_registrar_data_1.organization,test_registrar_data_2.organization);
    test_diff.street1 = std::make_pair(test_registrar_data_1.street1,test_registrar_data_2.street1);
    test_diff.street2 = std::make_pair(test_registrar_data_1.street2,test_registrar_data_2.street2);
    test_diff.street3 = std::make_pair(test_registrar_data_1.street3,test_registrar_data_2.street3);
    test_diff.city = std::make_pair(test_registrar_data_1.city,test_registrar_data_2.city);
    test_diff.stateorprovince = std::make_pair(test_registrar_data_1.stateorprovince,test_registrar_data_2.stateorprovince);
    test_diff.postalcode = std::make_pair(test_registrar_data_1.postalcode,test_registrar_data_2.postalcode);
    test_diff.country = std::make_pair(test_registrar_data_1.country,test_registrar_data_2.country);
    test_diff.telephone = std::make_pair(test_registrar_data_1.telephone,test_registrar_data_2.telephone);
    test_diff.fax = std::make_pair(test_registrar_data_1.fax,test_registrar_data_2.fax);
    test_diff.email = std::make_pair(test_registrar_data_1.email,test_registrar_data_2.email);
    test_diff.url = std::make_pair(test_registrar_data_1.url,test_registrar_data_2.url);
    test_diff.system= std::make_pair(test_registrar_data_1.system,test_registrar_data_2.system);
    test_diff.ico = std::make_pair(test_registrar_data_1.ico,test_registrar_data_2.ico);
    test_diff.dic = std::make_pair(test_registrar_data_1.dic,test_registrar_data_2.dic);
    test_diff.variable_symbol = std::make_pair(test_registrar_data_1.variable_symbol,test_registrar_data_2.variable_symbol);
    test_diff.payment_memo_regex = std::make_pair(test_registrar_data_1.payment_memo_regex,test_registrar_data_2.payment_memo_regex);
    test_diff.vat_payer= std::make_pair(test_registrar_data_1.vat_payer,test_registrar_data_2.vat_payer);

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_TEST_MESSAGE(::LibFred::diff_registrar_data(registrar_info1.info_registrar_data,registrar_info2.info_registrar_data).to_string());
    BOOST_TEST_MESSAGE(test_diff.to_string());

    BOOST_CHECK(::LibFred::diff_registrar_data(registrar_info1.info_registrar_data,registrar_info2.info_registrar_data).to_string() == test_diff.to_string());
}

BOOST_AUTO_TEST_SUITE_END();//TestInfoRegistrar
