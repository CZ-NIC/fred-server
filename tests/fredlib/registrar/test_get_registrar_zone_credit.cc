/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/get_registrar_zone_credit.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"
#include "util/random_data_generator.h"

#include "tests/setup/fixtures.h"

#include <string>

#include <boost/test/unit_test.hpp>

namespace {

struct test_get_registrar_zone_credit_fixture : Test::instantiate_db_template
{
    test_get_registrar_zone_credit_fixture()
        : xmark(RandomDataGenerator().xnumstring(6)),
          registrar_1_handle("TEST-REGISTRAR1-HANDLE" + xmark),
          registrar_2_handle("TEST-REGISTRAR2-HANDLE" + xmark)
    {
        Fred::OperationContextCreator ctx;

        Fred::CreateRegistrar(registrar_1_handle)
            .set_name("TEST-REGISTRAR NAME1" + xmark)
            .set_organization("TEST-REGISTRAR ORG1" + xmark)
            .set_street1("STR11" + xmark)
            .set_street2("STR21" + xmark)
            .set_street3("STR31" + xmark)
            .set_city("Praha 1")
            .set_stateorprovince("State1")
            .set_postalcode("11150")
            .set_country("CZ")
            .set_telephone("+420.728123456")
            .set_fax("+420.728123457")
            .set_email("test1@nic.cz")
            .set_url("www.test1.com")
            .set_system(false)
            .set_ico("1023456789")
            .set_dic("1123456789")
            .set_variable_symbol("123456789")
            .set_payment_memo_regex("test-registrar1*")
            .set_vat_payer(true)
            .exec(ctx);

        const unsigned long long registrar_1_id = static_cast<unsigned long long>(
            ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle=$1::TEXT",
                    Database::query_param_list(registrar_1_handle))[0][0]);

        Fred::CreateRegistrar(registrar_2_handle)
            .set_name("TEST-REGISTRAR NAME2" + xmark)
            .set_organization("TEST-REGISTRAR ORG2" + xmark)
            .set_street1("STR12" + xmark)
            .set_street2("STR22" + xmark)
            .set_street3("STR32" + xmark)
            .set_city("Praha 2")
            .set_stateorprovince("State2")
            .set_postalcode("21150")
            .set_country("SK")
            .set_telephone("+420.728123458")
            .set_fax("+420.728123459")
            .set_email("test2@nic.cz")
            .set_url("www.test2.com")
            .set_system(true)
            .set_ico("2023456789")
            .set_dic("2123456789")
            .set_variable_symbol("223456789")
            .set_payment_memo_regex("test-registrar2*")
            .set_vat_payer(false)
            .exec(ctx);

        const unsigned long long registrar_2_id = static_cast<unsigned long long>(
            ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle=$1::TEXT",
                    Database::query_param_list(registrar_2_handle))[0][0]);

        const Database::Result db_res = ctx.get_conn().exec("SELECT id,LOWER(fqdn) FROM zone LIMIT 2");
        BOOST_REQUIRE(db_res.size() == 2);
        const unsigned long long zone_1_id = static_cast<unsigned long long>(db_res[0][0]);
        const std::string zone_1_fqdn = static_cast<std::string>(db_res[0][1]);
        const unsigned long long zone_2_id = static_cast<unsigned long long>(db_res[1][0]);
        const std::string zone_2_fqdn = static_cast<std::string>(db_res[1][1]);

        ctx.get_conn().exec_params(
                "INSERT INTO registrarinvoice (registrarid,zone,fromdate,todate) "
                "VALUES($1::BIGINT,$3::BIGINT,NOW()::DATE,NULL),"
                      "($2::BIGINT,$3::BIGINT,NOW()::DATE,NULL),"
                      "($2::BIGINT,$4::BIGINT,NOW()::DATE,NULL)",
                Database::query_param_list(registrar_1_id)
                                          (registrar_2_id)
                                          (zone_1_id)
                                          (zone_2_id));
        registrar_1_zones.insert(Fred::ZoneCredit(zone_1_fqdn, Decimal("1000.00")));
        registrar_1_zones.insert(Fred::ZoneCredit(zone_2_fqdn));
        registrar_2_zones.insert(Fred::ZoneCredit(zone_1_fqdn, Decimal("2000.01")));
        registrar_2_zones.insert(Fred::ZoneCredit(zone_2_fqdn, Decimal("3000.10")));

        ctx.get_conn().exec_params(
                "DELETE FROM registrar_credit WHERE registrar_id IN ($1::BIGINT,$2::BIGINT)",
                Database::query_param_list(registrar_1_id)
                                          (registrar_2_id));

        ctx.get_conn().exec_params(
                "INSERT INTO registrar_credit (registrar_id,zone_id,credit) "
                "VALUES($1::BIGINT,$2::BIGINT,$3::NUMERIC(30,2)),"
                      "($4::BIGINT,$5::BIGINT,$6::NUMERIC(30,2)),"
                      "($7::BIGINT,$8::BIGINT,$9::NUMERIC(30,2))",
                Database::query_param_list(registrar_1_id)(zone_1_id)("1000.00")
                                          (registrar_2_id)(zone_1_id)("2000.01")
                                          (registrar_2_id)(zone_2_id)("3000.10"));

        ctx.commit_transaction();//commit fixture
    }
    const std::string xmark;
    const std::string registrar_1_handle;
    const std::string registrar_2_handle;
    Fred::RegistrarZoneCredit registrar_1_zones;
    Fred::RegistrarZoneCredit registrar_2_zones;
};

bool operator==(const Fred::RegistrarZoneCredit& a, const Fred::RegistrarZoneCredit& b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (Fred::RegistrarZoneCredit::const_iterator a_ptr = a.begin(); a_ptr != a.end(); ++a_ptr)
    {
        const Fred::RegistrarZoneCredit::const_iterator b_ptr = b.find(*a_ptr);
        const bool a_found_in_b = b_ptr != b.end();
        if (!a_found_in_b)
        {
            return false;
        }
        if (a_ptr->has_credit() != b_ptr->has_credit())
        {
            return false;
        }
        if (a_ptr->has_credit() &&
            (a_ptr->get_credit() != b_ptr->get_credit()))
        {
            return false;
        }
    }
    return true;
}

}//namespace {anonymous}

BOOST_FIXTURE_TEST_SUITE(TestGetRegistrarZoneCredit, test_get_registrar_zone_credit_fixture)

BOOST_AUTO_TEST_CASE(get_registrar_zone_credit_success)
{
    Fred::OperationContextCreator ctx;
    const Fred::RegistrarZoneCredit registrar_1_result = Fred::GetRegistrarZoneCredit().exec(ctx, registrar_1_handle);
    BOOST_CHECK_EQUAL(registrar_1_result.size(), 2);
    BOOST_CHECK(registrar_1_result == registrar_1_zones);
    const Fred::RegistrarZoneCredit registrar_2_result = Fred::GetRegistrarZoneCredit().exec(ctx, registrar_2_handle);
    BOOST_CHECK_EQUAL(registrar_2_result.size(), 2);
    BOOST_CHECK(registrar_2_result == registrar_2_zones);
}

BOOST_AUTO_TEST_SUITE_END()//TestGetRegistrarZoneCredit
