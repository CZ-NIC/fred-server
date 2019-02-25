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

#include <boost/date_time/posix_time/posix_time.hpp>
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"
#include "libfred/registrar/info_registrar_impl.hh"
#include "libfred/registrar/registrar_zone_access.hh"
#include "libfred/zone/zone.hh"
#include "libfred/opexception.hh"
#include "libfred/opcontext.hh"
#include "util/util.hh"

#include "util/random_data_generator.hh"

/**
 *  @file
 *  test registrar access to zone
 */

#include <boost/test/unit_test.hpp>

#include "test/setup/fixtures.hh"


struct test_registrar_zone_access_fixture : virtual public Test::instantiate_db_template
{
    std::string xmark;
    ::LibFred::InfoRegistrarData test_registrar_data_1;
    ::LibFred::InfoRegistrarData test_registrar_data_2;

    test_registrar_zone_access_fixture()
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

        ctx.get_conn().exec("TRUNCATE zone CASCADE");

        unsigned long long zone_id = static_cast<unsigned long long>(ctx.get_conn().exec(
                "INSERT INTO zone (fqdn, ex_period_min, ex_period_max, val_period, dots_max, enum_zone, warning_letter) "
                "VALUES ('cz', 12, 120, 0, 1, FALSE, TRUE) RETURNING id"
                )[0][0]);

        ctx.get_conn().exec_params("INSERT INTO zone_soa (zone, ttl, hostmaster, serial, refresh, update_retr, expiry, minimum, ns_fqdn) "
                        "VALUES ($1::bigint, 18000, 'hostmaster@nic.cz', NULL,900,300, 604800, 900, 'a.ns.nic.cz')",
                Database::query_param_list(zone_id));

        ctx.get_conn().exec_params(
            "INSERT INTO registrarinvoice (registrarid,zone,fromdate,todate) "
            "VALUES ($1::bigint, $2::bigint, $3::date, $4::date) RETURNING id",
                Database::query_param_list(test_registrar_data_2.id)
                    (zone_id)
                    (boost::posix_time::microsec_clock::local_time().date())
                    (Database::QPNull)
                );

        ctx.commit_transaction();//commit fixture
    }
    ~test_registrar_zone_access_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestRegistrarZoneAccess, test_registrar_zone_access_fixture)

/**
 * test call registrar zone access
*/
BOOST_AUTO_TEST_CASE(registrar_zone_access)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::Zone::Data zone_cz = ::LibFred::Zone::get_zone(ctx, "cz");
    BOOST_CHECK(!LibFred::is_zone_accessible_by_registrar(test_registrar_data_1.id, zone_cz.id, boost::posix_time::microsec_clock::local_time().date(), ctx));
    BOOST_CHECK(::LibFred::is_zone_accessible_by_registrar(test_registrar_data_2.id, zone_cz.id, boost::posix_time::microsec_clock::local_time().date(), ctx));
}

BOOST_AUTO_TEST_SUITE_END();
