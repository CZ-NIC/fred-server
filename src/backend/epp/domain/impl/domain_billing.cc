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
#include "src/backend/epp/domain/impl/domain_billing.hh"
#include "src/backend/epp/exception.hh"
#include "libfred/registrar/credit/create_registrar_credit_transaction.hh"
#include "libfred/zone/zone.hh"
#include "util/decimal/decimal.hh"

namespace Epp {
namespace Domain {

void create_domain_bill_item(
        const std::string& _fqdn,
        const boost::posix_time::ptime& _domain_create_timestamp_utc,
        unsigned long long _sponsoring_registrar_id,
        unsigned long long _created_domain_id,
        LibFred::OperationContext& _ctx)
{
    const unsigned long long zone_id = LibFred::Zone::find_zone_in_fqdn(
            _ctx,
            LibFred::Zone::rem_trailing_dot(_fqdn)).id;

    // get_operation_payment_settings
    const Database::Result operation_price_list_result
        = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT enable_postpaid_operation, operation_id, price, quantity"
            " FROM price_list pl "
                " JOIN enum_operation eo ON pl.operation_id = eo.id "
                " JOIN zone z ON z.id = pl.zone_id "
            " WHERE pl.valid_from <= $1::timestamp "
                  " AND (pl.valid_to is NULL OR pl.valid_to > $1::timestamp ) "
            " AND pl.zone_id = $2::bigint AND eo.operation = $3::text "
            // clang-format on
            ,
            Database::query_param_list(_domain_create_timestamp_utc)(zone_id)("CreateDomain"));

    if (operation_price_list_result.size() != 1)
    {
        _ctx.get_log().error("price_list result");
        throw std::runtime_error("price_list result");
    }

    const bool enable_postpaid_operation = static_cast<bool>(operation_price_list_result[0][0]);
    const unsigned long long operation_id =
        static_cast<unsigned long long>(operation_price_list_result[0][1]);
    const Decimal price_list_price = static_cast<std::string>(operation_price_list_result[0][2]);
    const Decimal price_list_quantity = static_cast<std::string>(operation_price_list_result[0][3]);

    if (price_list_quantity == Decimal("0"))
    {
        _ctx.get_log().error("price_list_quantity == 0");
        throw std::runtime_error("price_list_quantity == 0");
    }

    const Decimal price = price_list_price * Decimal("1") / price_list_quantity; // count_price

    // get_registrar_credit - lock record in registrar_credit table for registrar and zone
    const Database::Result locked_registrar_credit_result
        = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT r.handle, "
                 "(SELECT z.fqdn "
                    "FROM zone z "
                   "WHERE z.id = $2::bigint), "
                 "(SELECT rc.credit "
                    "FROM registrar_credit rc "
                  "WHERE rc.registrar_id = $1::bigint "
                    "AND rc.zone_id = $2::bigint "
                    "FOR UPDATE) "
              "FROM registrar r "
             "WHERE r.id = $1::bigint ",
            // clang-format on
            Database::query_param_list(_sponsoring_registrar_id)(zone_id));

    if (locked_registrar_credit_result.size() != 1)
    {
        std::cout << locked_registrar_credit_result.size() << std::endl;
        _ctx.get_log().error("unable to get registrar_credit");
        throw std::runtime_error("unable to get registrar_credit");
    }

    const std::string registrar_handle = static_cast<std::string>(locked_registrar_credit_result[0][0]);
    const std::string zone_fqdn = static_cast<std::string>(locked_registrar_credit_result[0][1]);
    const Decimal registrar_credit_balance = static_cast<std::string>(locked_registrar_credit_result[0][2]);

    if ((price != Decimal("0"))
        && (registrar_credit_balance < price)
        && (!enable_postpaid_operation))
    {
        throw BillingFailure();
    }

    unsigned long long registrar_credit_transaction_id;
    // save info about debt into credit
    try
    {
        registrar_credit_transaction_id = LibFred::Registrar::Credit::CreateRegistrarCreditTransaction(
                                                registrar_handle,
                                                zone_fqdn,
                                                Decimal("0") - price)
                                            .exec(_ctx);
    }
    catch (...)
    {
        _ctx.get_log().error("charge_operation: registrar_credit_transaction failed");
        throw std::runtime_error("charge_operation: registrar_credit_transaction failed");
    }

    // new record to invoice_operation
    _ctx.get_conn().exec_params(
            // clang-format off
            "INSERT INTO invoice_operation "
            " (id, object_id, registrar_id, operation_id, zone_id" //4
            " , crdate, quantity, date_from,  date_to "
            " , registrar_credit_transaction_id) "
            "  VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint "
            " , CURRENT_TIMESTAMP::timestamp, $5::integer, $6::date, NULL::date "
            " , $7::bigint) "
            //" RETURNING id "
            // clang-format on
            ,
            Database::query_param_list(_created_domain_id)
                (_sponsoring_registrar_id)(operation_id)(zone_id)
                ("1")(
                    boost::date_time::c_local_adjustor<ptime>::utc_to_local(_domain_create_timestamp_utc).
                    date())
                (registrar_credit_transaction_id)
            );
}


void renew_domain_bill_item(
        const std::string& _fqdn,
        const boost::posix_time::ptime& _domain_renew_timestamp_utc,
        unsigned long long _sponsoring_registrar_id,
        unsigned long long _renewed_domain_id,
        int _length_of_domain_registration_in_months,
        const boost::gregorian::date& _old_domain_expiration_date_local,
        const boost::gregorian::date& _domain_expiration_date_local,
        LibFred::OperationContext& _ctx)
{
    const int length_of_domain_registration_in_years = _length_of_domain_registration_in_months / 12;

    _ctx.get_log().debug(
            boost::format("length_of_domain_registration_in_months: %1% length_of_domain_registration_in_years: %2%")
            % _length_of_domain_registration_in_months % length_of_domain_registration_in_years);

    // exception in find_zone_in_fqdn is not BillingFailure
    const unsigned long long zone_id =
            LibFred::Zone::find_zone_in_fqdn(
                    _ctx,
                    LibFred::Zone::rem_trailing_dot(_fqdn))
                    .id;

    // get_operation_payment_settings
    const Database::Result operation_price_list_result =
            _ctx.get_conn().exec_params(
                    // clang-format off
                    "SELECT enable_postpaid_operation, operation_id, price, quantity"
                    " FROM price_list pl "
                        " JOIN enum_operation eo ON pl.operation_id = eo.id "
                        " JOIN zone z ON z.id = pl.zone_id "
                    " WHERE pl.valid_from <= $1::timestamp "
                        " AND (pl.valid_to is NULL OR pl.valid_to > $1::timestamp ) "
                    " AND pl.zone_id = $2::bigint AND eo.operation = $3::text ",
                    // clang-format on
                    Database::query_param_list(_domain_renew_timestamp_utc)(zone_id)("RenewDomain"));

    if (operation_price_list_result.size() != 1)
    {
        _ctx.get_log().error("price_list result");
        throw std::runtime_error("price_list result");
    }

    const bool enable_postpaid_operation = static_cast<bool>(operation_price_list_result[0][0]);
    const unsigned long long operation_id =
        static_cast<unsigned long long>(operation_price_list_result[0][1]);
    const Decimal price_list_price = static_cast<std::string>(operation_price_list_result[0][2]);
    const Decimal price_list_quantity = static_cast<std::string>(operation_price_list_result[0][3]);

    if (price_list_quantity == Decimal("0"))
    {
        _ctx.get_log().error("price_list_quantity == 0");
        throw std::runtime_error("price_list_quantity == 0");
    }

    const Decimal price = price_list_price
                          * Decimal(boost::lexical_cast<std::string>(length_of_domain_registration_in_years))
                          / price_list_quantity;

    _ctx.get_log().debug(
            boost::format("price_list_price: %1% price_list_quantity: %2% price: %3%")
            % price_list_price.get_string() % price_list_quantity.get_string() % price.get_string());

    // get_registrar_credit - lock record in registrar_credit table for registrar and zone
    const Database::Result locked_registrar_credit_result =
            _ctx.get_conn().exec_params(
                    // clang-format off
                    "SELECT r.handle, "
                        "(SELECT z.fqdn "
                           "FROM zone z "
                          "WHERE z.id = $2::bigint), "
                        "(SELECT rc.credit "
                           "FROM registrar_credit rc "
                          "WHERE rc.registrar_id = $1::bigint "
                            "AND rc.zone_id = $2::bigint "
                            "FOR UPDATE) "
                      "FROM registrar r "
                     "WHERE r.id = $1::bigint ",
                    // clang-format on
                    Database::query_param_list(_sponsoring_registrar_id)(zone_id));

    if (locked_registrar_credit_result.size() != 1)
    {
        std::cout << locked_registrar_credit_result.size() << std::endl;
        _ctx.get_log().error("unable to get registrar_credit");
        throw std::runtime_error("unable to get registrar_credit");
    }

    const std::string registrar_handle = static_cast<std::string>(locked_registrar_credit_result[0][0]);
    const std::string zone_fqdn = static_cast<std::string>(locked_registrar_credit_result[0][1]);
    const Decimal registrar_credit_balance = static_cast<std::string>(locked_registrar_credit_result[0][2]);

    if ((price != Decimal("0"))
        && (registrar_credit_balance < price)
        && (!enable_postpaid_operation))
    {
        throw BillingFailure();
    }

    unsigned long long registrar_credit_transaction_id;
    // save info about debt into credit
    try
    {
        registrar_credit_transaction_id = LibFred::Registrar::Credit::CreateRegistrarCreditTransaction(
                                                registrar_handle,
                                                zone_fqdn,
                                                Decimal("0") - price)
                                            .exec(_ctx);
    }
    catch (...)
    {
        _ctx.get_log().error("charge_operation: registrar_credit_transaction failed");
        throw std::runtime_error("charge_operation: registrar_credit_transaction failed");
    }

    // new record to invoice_operation
    _ctx.get_conn().exec_params(
            // clang-format off
            "INSERT INTO invoice_operation "
            " (id, object_id, registrar_id, operation_id, zone_id" //4
            " , crdate, quantity, date_from,  date_to "
            " , registrar_credit_transaction_id) "
            "  VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint "
            " , CURRENT_TIMESTAMP::timestamp, $5::integer, $6::date, $7::date "
            " , $8::bigint) ",
            // clang-format on
            Database::query_param_list(_renewed_domain_id)
                (_sponsoring_registrar_id)(operation_id)(zone_id)
                (length_of_domain_registration_in_years)
                (_old_domain_expiration_date_local)
                (_domain_expiration_date_local)
                (registrar_credit_transaction_id));
}


} // namespace Epp::Domain
} // namespace Epp
