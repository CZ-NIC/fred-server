/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/get_registrar_zone_credit.hh"
#include "libfred/zone/zone.hh"
#include "libfred/opexception.hh"
#include "libfred/opcontext.hh"
#include "util/util.hh"
#include "util/random_data_generator.hh"

#include "test/setup/fixtures.hh"

#include <string>

#include <boost/test/unit_test.hpp>

namespace {

struct test_get_registrar_zone_credit_fixture:Test::instantiate_db_template
{
    test_get_registrar_zone_credit_fixture()
        : xmark(RandomDataGenerator().xnumstring(6)),
          registrar_1_handle("TEST-REGISTRAR1-HANDLE" + xmark),
          registrar_2_handle("TEST-REGISTRAR2-HANDLE" + xmark)
    {
        ::LibFred::CreateRegistrar(registrar_1_handle)
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

        ::LibFred::CreateRegistrar(registrar_2_handle)
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

        ctx.get_conn().exec_params(
                "DELETE FROM registrarinvoice WHERE registrarid IN ($1::BIGINT,$2::BIGINT)",
                Database::query_param_list(registrar_1_id)
                                          (registrar_2_id));

        ctx.get_conn().exec_params(
                "DELETE FROM registrar_credit WHERE registrar_id IN ($1::BIGINT,$2::BIGINT)",
                Database::query_param_list(registrar_1_id)
                                          (registrar_2_id));

        const Database::Result db_reg_1_zones = ctx.get_conn().exec_params(
                "INSERT INTO registrarinvoice (registrarid,zone,fromdate,todate) "
                "SELECT $1::BIGINT,id,NOW()::DATE,NULL FROM zone LIMIT (SELECT COUNT(*)-1 FROM zone) "
                "RETURNING (SELECT LOWER(fqdn) FROM zone WHERE id=zone)",
                Database::query_param_list(registrar_1_id));
        BOOST_REQUIRE(0 < db_reg_1_zones.size());
        Decimal credit = "1000.00";
        for (unsigned idx = 0; idx < db_reg_1_zones.size(); ++idx)
        {
            const std::string zone_fqdn = static_cast<std::string>(db_reg_1_zones[idx][0]);
            ctx.get_conn().exec_params(
                    "INSERT INTO registrar_credit (registrar_id,zone_id,credit) "
                    "SELECT $1::BIGINT,id,$3::NUMERIC(30,2) FROM zone WHERE LOWER(fqdn)=$2::TEXT",
                    Database::query_param_list(registrar_1_id)
                                              (zone_fqdn)
                                              (credit.get_string()));
            registrar_1_zones.insert(::LibFred::ZoneCredit(zone_fqdn, credit));
            credit += Decimal("1000.00");
        }

        const Database::Result db_reg_2_zones = ctx.get_conn().exec_params(
                "INSERT INTO registrarinvoice (registrarid,zone,fromdate,todate) "
                "SELECT $1::BIGINT,id,NOW()::DATE,NULL FROM zone "
                "RETURNING (SELECT LOWER(fqdn) FROM zone WHERE id=zone)",
                Database::query_param_list(registrar_2_id));
        BOOST_REQUIRE((db_reg_1_zones.size() + 1) == db_reg_2_zones.size());
        credit = "1500.01";
        for (unsigned idx = 0; idx < db_reg_2_zones.size(); ++idx)
        {
            const std::string zone_fqdn = static_cast<std::string>(db_reg_2_zones[idx][0]);
            ctx.get_conn().exec_params(
                    "INSERT INTO registrar_credit (registrar_id,zone_id,credit) "
                    "SELECT $1::BIGINT,id,$3::NUMERIC(30,2) FROM zone WHERE LOWER(fqdn)=$2::TEXT",
                    Database::query_param_list(registrar_2_id)
                                              (zone_fqdn)
                                              (credit.get_string()));
            registrar_2_zones.insert(::LibFred::ZoneCredit(zone_fqdn, credit));
            credit += Decimal("1000.10");
            const ::LibFred::ZoneCredit missing_zone(zone_fqdn);
            if (registrar_1_zones.find(missing_zone) == registrar_1_zones.end())
            {
                registrar_1_zones.insert(missing_zone);
            }
        }
        BOOST_REQUIRE(registrar_1_zones.size() == registrar_2_zones.size());
    }
    const std::string xmark;
    const std::string registrar_1_handle;
    const std::string registrar_2_handle;
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::RegistrarZoneCredit registrar_1_zones;
    ::LibFred::RegistrarZoneCredit registrar_2_zones;
};

bool operator==(const ::LibFred::RegistrarZoneCredit& a, const ::LibFred::RegistrarZoneCredit& b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (::LibFred::RegistrarZoneCredit::const_iterator a_ptr = a.begin(); a_ptr != a.end(); ++a_ptr)
    {
        const ::LibFred::RegistrarZoneCredit::const_iterator b_ptr = b.find(*a_ptr);
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

} // namespace {anonymous}

BOOST_FIXTURE_TEST_SUITE(TestGetRegistrarZoneCredit, test_get_registrar_zone_credit_fixture)

BOOST_AUTO_TEST_CASE(get_registrar_zone_credit_success)
{
    const ::LibFred::RegistrarZoneCredit registrar_1_result = ::LibFred::GetRegistrarZoneCredit().exec(ctx, registrar_1_handle);
    BOOST_CHECK(2 <= registrar_1_result.size());
    BOOST_CHECK(registrar_1_zones == registrar_1_result);
    const ::LibFred::RegistrarZoneCredit registrar_2_result = ::LibFred::GetRegistrarZoneCredit().exec(ctx, registrar_2_handle);
    BOOST_CHECK(2 <= registrar_2_result.size());
    BOOST_CHECK_EQUAL(registrar_1_result.size(), registrar_2_result.size());
    BOOST_CHECK(registrar_2_result == registrar_2_zones);
}

BOOST_AUTO_TEST_SUITE_END()//TestGetRegistrarZoneCredit
