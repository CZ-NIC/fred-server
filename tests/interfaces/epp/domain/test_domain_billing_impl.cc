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

/**
 *  @file
 */

#include "src/epp/domain/domain_billing.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "util/decimal/decimal.h"

#include <vector>
#include <string>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(Billing)

BOOST_FIXTURE_TEST_CASE(create_domain_no_pricelist, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //no pricelist
    ctx.get_conn().exec("UPDATE enum_operation SET operation='xCreateDomain' WHERE operation = 'CreateDomain';");

    BOOST_CHECK_THROW(
        Epp::create_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(create_domain_no_pricelist_quantity, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //no pricelist quantity
    ctx.get_conn().exec(
        "UPDATE price_list SET quantity = 0 where id = (SELECT id FROM price_list"
        " WHERE operation_id = (SELECT id FROM enum_operation WHERE operation='CreateDomain')"
            " and zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
            " ORDER BY valid_from DESC LIMIT 1);");

    BOOST_CHECK_THROW(
        Epp::create_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
        ctx),
        std::runtime_error);
}


BOOST_FIXTURE_TEST_CASE(create_domain_no_credit_record, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    BOOST_CHECK_THROW(
        Epp::create_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(create_domain_no_money, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //no money
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "(0, ($1::bigint), ((SELECT id FROM zone WHERE fqdn = 'cz'))) ",
            Database::query_param_list(info_registrar_data_.id));

    BOOST_CHECK_NO_THROW(
        Epp::create_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
        ctx));
}

BOOST_FIXTURE_TEST_CASE(create_domain, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //10000 money
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "(10000, ($1::bigint), ((SELECT id FROM zone WHERE fqdn = 'cz'))) ",
            Database::query_param_list(info_registrar_data_.id));

    //charge create, zero price
    Epp::create_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
        ctx);

    //no credit change check
    BOOST_CHECK(Decimal(static_cast<std::string>(ctx.get_conn().exec_params(
        "SELECT credit FROM registrar_credit"
        " WHERE registrar_id = $1::bigint and zone_id = ("
        "SELECT id FROM zone WHERE fqdn = 'cz')",
        Database::query_param_list(info_registrar_data_.id))[0][0])) == Decimal("10000"));

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
        Database::query_param_list(info_registrar_data_.id)
        (info_data.id));

    BOOST_REQUIRE(invoice_item.size() == 1);

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item[0]["date_from"])) == current_local_date);
    BOOST_CHECK(invoice_item[0]["date_to"].isnull());
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_pricelist, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    //no pricelist
    ctx.get_conn().exec("UPDATE enum_operation SET operation='xRenewDomain' WHERE operation = 'RenewDomain';");

    BOOST_CHECK_THROW(
        Epp::renew_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
            1,
            current_local_date,
            info_data.expiration_date,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_pricelist_quantity, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    //no pricelist quantity
    ctx.get_conn().exec(
        "UPDATE price_list SET quantity = 0 where id = (SELECT id FROM price_list"
        " WHERE operation_id = (SELECT id FROM enum_operation WHERE operation='RenewDomain')"
            " and zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
            " ORDER BY valid_from DESC LIMIT 1);");

    BOOST_CHECK_THROW(
        Epp::renew_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
            1,
            current_local_date,
            info_data.expiration_date,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_credit_record, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    BOOST_CHECK_THROW(
        Epp::renew_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
            1,
            current_local_date,
            info_data.expiration_date,
        ctx),
        std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(renew_domain_no_money, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    //no money
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "(0, ($1::bigint), ((SELECT id FROM zone WHERE fqdn = 'cz'))) ",
            Database::query_param_list(info_registrar_data_.id));

    BOOST_CHECK_THROW(
        Epp::renew_domain_bill_item(fqdn2,
            info_data.creation_time,
            info_registrar_data_.id,
            info_data.id,
            1,
            current_local_date,
            info_data.expiration_date,
        ctx),
        Epp::BillingFailure);
}

BOOST_FIXTURE_TEST_CASE(renew_domain, HasDomainData)
{
    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(fqdn2).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    //10000 money
    ctx.get_conn().exec_params(
        "INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
            "(10000, ($1::bigint), ((SELECT id FROM zone WHERE fqdn = 'cz'))) ",
            Database::query_param_list(info_registrar_data_.id));

    //1. renew
    Epp::renew_domain_bill_item(
        fqdn2,
        info_data.creation_time,
        info_registrar_data_.id,
        info_data.id,
        1,
        current_local_date,
        info_data.expiration_date,
        ctx);

    //check credit change for 1 year renew
    BOOST_CHECK(Decimal(static_cast<std::string>(ctx.get_conn().exec_params(
        "SELECT credit FROM registrar_credit"
        " WHERE registrar_id = $1::bigint and zone_id = ("
        "SELECT id FROM zone WHERE fqdn = 'cz')",
        Database::query_param_list(info_registrar_data_.id))[0][0]))
            == Decimal("10000") - Decimal(static_cast<std::string>(ctx.get_conn().exec(
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
        Database::query_param_list(info_registrar_data_.id)
        (info_data.id));

    BOOST_REQUIRE(invoice_item.size() == 1);

    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item[0]["date_from"])) == current_local_date);
    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item[0]["date_to"])) == info_data.expiration_date);

    //new_exdate = current_exdate + 2 years
    const boost::gregorian::date new_exdate = boost::gregorian::from_simple_string(
                static_cast<std::string>(ctx.get_conn().exec_params(
                    "SELECT $1::date + '2 years'::interval",
                    Database::query_param_list(info_data.expiration_date))[0][0]));

    //2. renew, for 2 years
    Epp::renew_domain_bill_item(
        fqdn2,
        info_data.creation_time,
        info_registrar_data_.id,
        info_data.id,
        2,
        info_data.expiration_date,
        new_exdate,
        ctx);

    //check credit change for 1 + 2 year renew
    BOOST_CHECK(Decimal(static_cast<std::string>(ctx.get_conn().exec_params(
        "SELECT credit FROM registrar_credit"
        " WHERE registrar_id = $1::bigint and zone_id = ("
        "SELECT id FROM zone WHERE fqdn = 'cz')",
        Database::query_param_list(info_registrar_data_.id))[0][0]))
            == Decimal("10000") - Decimal(static_cast<std::string>(ctx.get_conn().exec(
                "SELECT price * 3 FROM price_list"
                " WHERE operation_id = ("
                    "SELECT id FROM enum_operation WHERE operation='RenewDomain')"
                    " AND zone_id = (SELECT id FROM zone WHERE fqdn = 'cz')"
                    " ORDER BY valid_from DESC LIMIT 1")[0][0]))
    );

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
        Database::query_param_list(info_registrar_data_.id)
        (info_data.id));

    BOOST_REQUIRE(invoice_item2.size() == 1);

    //#18474 regression check
    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item2[0]["date_from"])) == info_data.expiration_date);
    BOOST_CHECK(boost::gregorian::from_string(static_cast<std::string>(invoice_item2[0]["date_to"])) == new_exdate);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
