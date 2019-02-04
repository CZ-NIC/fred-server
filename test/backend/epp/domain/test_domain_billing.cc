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

#include "src/backend/epp/domain/impl/domain_billing.hh"
#include "test/backend/epp/domain/fixture.hh"
#include "util/decimal/decimal.hh"

#include <vector>
#include <string>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(DomainBilling)

BOOST_FIXTURE_TEST_CASE(create_domain_no_pricelist, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    //no pricelist
    ctx.get_conn().exec("UPDATE enum_operation SET operation='xCreateDomain' WHERE operation = 'CreateDomain';");

    BOOST_CHECK_THROW(
        ::Epp::Domain::create_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(create_domain_no_pricelist_quantity, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    //no pricelist quantity
    ctx.get_conn().exec(
        "UPDATE price_list SET quantity = 0 where id = (SELECT id FROM price_list"
        " WHERE operation_id = (SELECT id FROM enum_operation WHERE operation='CreateDomain')"
            " and zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
            " ORDER BY valid_from DESC LIMIT 1);");

    BOOST_CHECK_THROW(
        ::Epp::Domain::create_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
        ctx),
        std::runtime_error);
}


BOOST_FIXTURE_TEST_CASE(create_domain_no_credit_record, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    BOOST_CHECK_THROW(
        ::Epp::Domain::create_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(create_domain_no_money, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    //no money
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "(0, $1::bigint, (SELECT id FROM zone WHERE fqdn = 'cz')) ",
            Database::query_param_list(registrar.data.id));

    BOOST_CHECK_NO_THROW(
        ::Epp::Domain::create_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
        ctx));
}

BOOST_FIXTURE_TEST_CASE(create_domain, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());
    const std::string registrar_initial_credit = "10000";

    //insert credit
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "($1::bigint, $2::bigint, (SELECT id FROM zone WHERE fqdn = 'cz')) ",
            Database::query_param_list(registrar_initial_credit)(registrar.data.id));

    //charge create, zero price
    ::Epp::Domain::create_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
        ctx);

    //no credit change check
    BOOST_CHECK(Decimal(static_cast<std::string>(ctx.get_conn().exec_params(
        "SELECT credit FROM registrar_credit"
        " WHERE registrar_id = $1::bigint and zone_id = ("
        "SELECT id FROM zone WHERE fqdn = 'cz')",
        Database::query_param_list(registrar.data.id))[0][0])) == Decimal(registrar_initial_credit));

    //get created invoice item
    Database::Result invoice_item = ctx.get_conn().exec_params(
        "SELECT id, date_from, date_to FROM invoice_operation "
        " WHERE registrar_id = $1::bigint "
        " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz') "
        " AND object_id = $2::bigint "
        " AND operation_id = (SELECT id FROM enum_operation WHERE operation='CreateDomain') "
        " AND quantity = 1 "
        " AND registrar_credit_transaction_id = ("
            " SELECT id FROM registrar_credit_transaction"
            " WHERE balance_change = 0"
            " AND registrar_credit_id = (SELECT id FROM registrar_credit"
                " WHERE registrar_id = $1::bigint and zone_id = ("
                    "SELECT id FROM zone WHERE fqdn = 'cz')) ORDER BY id DESC LIMIT 1)",
        Database::query_param_list(registrar.data.id)
        (domain.data.id));

    BOOST_REQUIRE(invoice_item.size() == 1);

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item[0]["date_from"])) == current_local_date);
    BOOST_CHECK(invoice_item[0]["date_to"].isnull());
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_pricelist, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();
    const int length_of_domain_registration_in_months = 12;

    //no pricelist
    ctx.get_conn().exec("UPDATE enum_operation SET operation='xRenewDomain' WHERE operation = 'RenewDomain';");

    BOOST_CHECK_THROW(
        ::Epp::Domain::renew_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
            length_of_domain_registration_in_months,
            current_local_date,
            domain.data.expiration_date,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_pricelist_quantity, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();
    const int length_of_domain_registration_in_months = 12;

    //no pricelist quantity
    ctx.get_conn().exec(
        "UPDATE price_list SET quantity = 0 where id = (SELECT id FROM price_list"
        " WHERE operation_id = (SELECT id FROM enum_operation WHERE operation='RenewDomain')"
            " and zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
            " ORDER BY valid_from DESC LIMIT 1);");

    BOOST_CHECK_THROW(
        ::Epp::Domain::renew_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
            length_of_domain_registration_in_months,
            current_local_date,
            domain.data.expiration_date,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_credit_record, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();
    const int length_of_domain_registration_in_months = 12;

    BOOST_CHECK_THROW(
        ::Epp::Domain::renew_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
            length_of_domain_registration_in_months,
            current_local_date,
            domain.data.expiration_date,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_money, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();
    const std::string registrar_initial_credit = "0";
    const int length_of_domain_registration_in_months = 12;

    //no money
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "($1::bigint, $2::bigint, (SELECT id FROM zone WHERE fqdn = 'cz')) ",
            Database::query_param_list(registrar_initial_credit)(registrar.data.id));

    BOOST_CHECK_THROW(
        ::Epp::Domain::renew_domain_bill_item(domain.data.fqdn,
            domain.data.creation_time,
            registrar.data.id,
            domain.data.id,
            length_of_domain_registration_in_months,
            current_local_date,
            domain.data.expiration_date,
        ctx),
        ::Epp::BillingFailure);
}

BOOST_FIXTURE_TEST_CASE(renew_domain, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_TEST_MESSAGE(domain.data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();
    const std::string registrar_initial_credit = "10000";
    const int length_of_domain_registration_12_months = 12;
    const int length_of_domain_registration_24_months = 24;

    //insert credit
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "($1::bigint, $2::bigint, (SELECT id FROM zone WHERE fqdn = 'cz')) ",
            Database::query_param_list(registrar_initial_credit)(registrar.data.id));

    //1. renew
    ::Epp::Domain::renew_domain_bill_item(
        domain.data.fqdn,
        domain.data.creation_time,
        registrar.data.id,
        domain.data.id,
        length_of_domain_registration_12_months,
        current_local_date,
        domain.data.expiration_date,
        ctx);

    //check credit change for 1 year renew
    BOOST_CHECK_EQUAL(
            Decimal(static_cast<std::string>(ctx.get_conn().exec_params(
                    "SELECT credit FROM registrar_credit"
                    " WHERE registrar_id = $1::bigint and zone_id = ("
                    "SELECT id FROM zone WHERE fqdn = 'cz')",
                    Database::query_param_list(registrar.data.id))[0][0])),
            Decimal(registrar_initial_credit) - Decimal(static_cast<std::string>(ctx.get_conn().exec(
                                                        "SELECT price FROM price_list"
                                                        " WHERE operation_id = ("
                                                        "SELECT id FROM enum_operation WHERE operation='RenewDomain')"
                                                        " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
                                                        " ORDER BY valid_from DESC LIMIT 1")[0][0])));

    //get created invoice item for 1 year renew
    Database::Result invoice_item = ctx.get_conn().exec_params(
        "SELECT id, date_from, date_to, registrar_credit_transaction_id FROM invoice_operation "
        " WHERE registrar_id = $1::bigint "
        " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz') "
        " AND object_id = $2::bigint "
        " AND operation_id = (SELECT id FROM enum_operation WHERE operation='RenewDomain') "
        " AND quantity = 1 "
        " AND registrar_credit_transaction_id = ("
            " SELECT id FROM registrar_credit_transaction"
            " WHERE balance_change = (SELECT -price FROM price_list"
                " WHERE operation_id = ("
                    "SELECT id FROM enum_operation WHERE operation='RenewDomain')"
                    " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
                    " ORDER BY valid_from DESC LIMIT 1)"
            " AND registrar_credit_id = (SELECT id FROM registrar_credit"
                " WHERE registrar_id = $1::bigint and zone_id = ("
                    "SELECT id FROM zone WHERE fqdn = 'cz')) ORDER BY id DESC LIMIT 1)",
        Database::query_param_list(registrar.data.id)
        (domain.data.id));

    BOOST_REQUIRE(invoice_item.size() == 1);

    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item[0]["date_from"])) == current_local_date);
    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item[0]["date_to"])) == domain.data.expiration_date);

    //new_exdate = current_exdate + 2 years
    const boost::gregorian::date new_exdate = boost::gregorian::from_simple_string(
                static_cast<std::string>(ctx.get_conn().exec_params(
                    "SELECT $1::date + '2 years'::interval",
                    Database::query_param_list(domain.data.expiration_date))[0][0]));

    //2. renew, for 2 years
    ::Epp::Domain::renew_domain_bill_item(
        domain.data.fqdn,
        domain.data.creation_time,
        registrar.data.id,
        domain.data.id,
        length_of_domain_registration_24_months,
        domain.data.expiration_date,
        new_exdate,
        ctx);

    //check credit change for 1 + 2 year renew
    BOOST_CHECK_EQUAL(
            Decimal(static_cast<std::string>(ctx.get_conn().exec_params(
                    "SELECT credit FROM registrar_credit"
                    " WHERE registrar_id = $1::bigint and zone_id = ("
                    "SELECT id FROM zone WHERE fqdn = 'cz')",
                    Database::query_param_list(registrar.data.id))[0][0])),
            Decimal(registrar_initial_credit) - Decimal(static_cast<std::string>(ctx.get_conn().exec(
                                                        "SELECT price * 3 FROM price_list"
                                                        " WHERE operation_id = ("
                                                        "SELECT id FROM enum_operation WHERE operation='RenewDomain')"
                                                        " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
                                                        " ORDER BY valid_from DESC LIMIT 1")[0][0])));

    //get created invoice item for 2 year renew
    Database::Result invoice_item2 = ctx.get_conn().exec_params(
        "SELECT id, date_from, date_to, registrar_credit_transaction_id FROM invoice_operation "
        " WHERE registrar_id = $1::bigint "
        " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz') "
        " AND object_id = $2::bigint "
        " AND operation_id = (SELECT id FROM enum_operation WHERE operation='RenewDomain') "
        " AND quantity = 2 "
        " AND registrar_credit_transaction_id = ("
            " SELECT id FROM registrar_credit_transaction"
            " WHERE balance_change = (SELECT -price*2 FROM price_list"
                " WHERE operation_id = ("
                    "SELECT id FROM enum_operation WHERE operation='RenewDomain')"
                    " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
                    " ORDER BY valid_from DESC LIMIT 1)"
            " AND registrar_credit_id = (SELECT id FROM registrar_credit"
                " WHERE registrar_id = $1::bigint and zone_id = ("
                    "SELECT id FROM zone WHERE fqdn = 'cz')) ORDER BY id DESC LIMIT 1)",
        Database::query_param_list(registrar.data.id)
        (domain.data.id));

    BOOST_REQUIRE(invoice_item2.size() == 1);

    //#18474 regression check
    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item2[0]["date_from"])) == domain.data.expiration_date);
    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item2[0]["date_to"])) == new_exdate);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
