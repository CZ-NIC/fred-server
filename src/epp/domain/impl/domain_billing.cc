/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "src/epp/domain/impl/domain_billing.h"
#include "src/epp/impl/exception.h"
#include "src/fredlib/zone/zone.h"
#include "util/decimal/decimal.h"

namespace Epp
{
    void create_domain_bill_item(const std::string& fqdn,
            const boost::posix_time::ptime& domain_create_timestamp_utc,
            unsigned long long sponsoring_registrar_id,
            unsigned long long created_domain_id,
            Fred::OperationContext& ctx)
    {
        const unsigned long long zone_id = Fred::Zone::find_zone_in_fqdn(
                ctx, Fred::Zone::rem_trailing_dot(fqdn)).id;

        //get_operation_payment_settings
        const Database::Result operation_price_list_result
            = ctx.get_conn().exec_params(
                "SELECT enable_postpaid_operation, operation_id, price, quantity"
                " FROM price_list pl "
                    " JOIN enum_operation eo ON pl.operation_id = eo.id "
                    " JOIN zone z ON z.id = pl.zone_id "
                " WHERE pl.valid_from <= $1::timestamp "
                      " AND (pl.valid_to is NULL OR pl.valid_to > $1::timestamp ) "
                " AND pl.zone_id = $2::bigint AND eo.operation = $3::text "
            , Database::query_param_list(domain_create_timestamp_utc)(zone_id)("CreateDomain"));

        if(operation_price_list_result.size() != 1)
        {
            ctx.get_log().error("price_list result");
            throw std::runtime_error("price_list result");
        }

        const bool enable_postpaid_operation = static_cast<bool>(operation_price_list_result[0][0]);
        const unsigned long long operation_id = static_cast<unsigned long long>(operation_price_list_result[0][1]);
        const Decimal price_list_price = static_cast<std::string>(operation_price_list_result[0][2]);
        const Decimal  price_list_quantity = static_cast<std::string>(operation_price_list_result[0][3]);

        if(price_list_quantity == Decimal("0"))
        {
            ctx.get_log().error("price_list_quantity == 0");
            throw std::runtime_error("price_list_quantity == 0");
        }

        const Decimal price =  price_list_price * Decimal("1") / price_list_quantity;//count_price

        //get_registrar_credit - lock record in registrar_credit table for registrar and zone
        const Database::Result locked_registrar_credit_result
            = ctx.get_conn().exec_params(
                "SELECT id, credit "
                    " FROM registrar_credit "
                    " WHERE registrar_id = $1::bigint "
                        " AND zone_id = $2::bigint "
                " FOR UPDATE "
            , Database::query_param_list(sponsoring_registrar_id)(zone_id));

        if(locked_registrar_credit_result.size() != 1)
        {
            ctx.get_log().error("unable to get registrar_credit");
            throw std::runtime_error("unable to get registrar_credit");
        }

        const unsigned long long registrar_credit_id = static_cast<unsigned long long>(locked_registrar_credit_result[0][0]);
        const Decimal registrar_credit_balance = static_cast<std::string>(locked_registrar_credit_result[0][1]);

        if((price != Decimal("0"))
        && (registrar_credit_balance < price)
        && (!enable_postpaid_operation))
        {
            throw BillingFailure();
        }

        // save info about debt into credit
        const Database::Result registrar_credit_transaction_result
            = ctx.get_conn().exec_params(
                "INSERT INTO registrar_credit_transaction "
                    " (id, balance_change, registrar_credit_id) "
                    " VALUES (DEFAULT, $1::numeric , $2::bigint) "
                " RETURNING id "
            , Database::query_param_list(Decimal("0") - price)(registrar_credit_id));

        if(registrar_credit_transaction_result.size() != 1)
        {
            ctx.get_log().error("charge_operation: registrar_credit_transaction failed");
            throw std::runtime_error("charge_operation: registrar_credit_transaction failed");
        }

        const unsigned long long registrar_credit_transaction_id = static_cast<unsigned long long>(registrar_credit_transaction_result[0][0]);

        // new record to invoice_operation
        ctx.get_conn().exec_params(
            "INSERT INTO invoice_operation "
            " (id, object_id, registrar_id, operation_id, zone_id" //4
            " , crdate, quantity, date_from,  date_to "
            " , registrar_credit_transaction_id) "
            "  VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint "
            " , CURRENT_TIMESTAMP::timestamp, $5::integer, $6::date, NULL::date "
            " , $7::bigint) "
            //" RETURNING id "
            , Database::query_param_list(created_domain_id)
            (sponsoring_registrar_id)(operation_id)(zone_id)
            ("1")(boost::date_time::c_local_adjustor<ptime>::utc_to_local(domain_create_timestamp_utc).date())
            (registrar_credit_transaction_id)
            );
    }

    void renew_domain_bill_item(
        const std::string& fqdn,
        const boost::posix_time::ptime& domain_renew_timestamp_utc,
        unsigned long long sponsoring_registrar_id,
        unsigned long long renewed_domain_id,
        int length_of_domain_registration_in_years,
        const boost::gregorian::date& old_domain_expiration_date_local,
        const boost::gregorian::date& domain_expiration_date_local,
        Fred::OperationContext& ctx)
    {
        //exception in find_zone_in_fqdn is not BillingFailure
        const unsigned long long zone_id = Fred::Zone::find_zone_in_fqdn(
                ctx, Fred::Zone::rem_trailing_dot(fqdn)).id;

        //get_operation_payment_settings
        const Database::Result operation_price_list_result
            = ctx.get_conn().exec_params(
                "SELECT enable_postpaid_operation, operation_id, price, quantity"
                " FROM price_list pl "
                    " JOIN enum_operation eo ON pl.operation_id = eo.id "
                    " JOIN zone z ON z.id = pl.zone_id "
                " WHERE pl.valid_from <= $1::timestamp "
                    " AND (pl.valid_to is NULL OR pl.valid_to > $1::timestamp ) "
                " AND pl.zone_id = $2::bigint AND eo.operation = $3::text "
            , Database::query_param_list(domain_renew_timestamp_utc)(zone_id)("RenewDomain"));

        if(operation_price_list_result.size() != 1)
        {
            ctx.get_log().error("price_list result");
            throw std::runtime_error("price_list result");
        }

        const bool enable_postpaid_operation = static_cast<bool>(operation_price_list_result[0][0]);
        const unsigned long long operation_id = static_cast<unsigned long long>(operation_price_list_result[0][1]);
        const Decimal price_list_price = static_cast<std::string>(operation_price_list_result[0][2]);
        const Decimal  price_list_quantity = static_cast<std::string>(operation_price_list_result[0][3]);

        if(price_list_quantity == Decimal("0"))
        {
            ctx.get_log().error("price_list_quantity == 0");
            throw std::runtime_error("price_list_quantity == 0");
        }

        const Decimal price =  price_list_price
            * Decimal(boost::lexical_cast<std::string>(length_of_domain_registration_in_years))
            / price_list_quantity;

        ctx.get_log().debug(boost::format("price_list_price: %1% price_list_quantity: %2% price: %3%")
        % price_list_price.get_string() % price_list_quantity.get_string() % price.get_string());

        //get_registrar_credit - lock record in registrar_credit table for registrar and zone
        const Database::Result locked_registrar_credit_result
            = ctx.get_conn().exec_params(
                "SELECT id, credit "
                    " FROM registrar_credit "
                    " WHERE registrar_id = $1::bigint "
                        " AND zone_id = $2::bigint "
                " FOR UPDATE "
            , Database::query_param_list(sponsoring_registrar_id)(zone_id));

        if(locked_registrar_credit_result.size() != 1)
        {
            ctx.get_log().error("unable to get registrar_credit");
            throw std::runtime_error("unable to get registrar_credit");
        }

        const unsigned long long registrar_credit_id = static_cast<unsigned long long>(locked_registrar_credit_result[0][0]);
        const Decimal registrar_credit_balance = static_cast<std::string>(locked_registrar_credit_result[0][1]);

        if((price != Decimal("0"))
        && (registrar_credit_balance < price)
        && (!enable_postpaid_operation))
        {
            throw BillingFailure();
        }

        // save info about debt into credit
        const Database::Result registrar_credit_transaction_result
            = ctx.get_conn().exec_params(
                "INSERT INTO registrar_credit_transaction "
                    " (id, balance_change, registrar_credit_id) "
                    " VALUES (DEFAULT, $1::numeric , $2::bigint) "
                " RETURNING id "
            , Database::query_param_list(Decimal("0") - price)(registrar_credit_id));

        if(registrar_credit_transaction_result.size() != 1)
        {
            ctx.get_log().error("charge_operation: registrar_credit_transaction failed");
            throw std::runtime_error("charge_operation: registrar_credit_transaction failed");
        }

        const unsigned long long registrar_credit_transaction_id = static_cast<unsigned long long>(registrar_credit_transaction_result[0][0]);

        // new record to invoice_operation
        ctx.get_conn().exec_params(
            "INSERT INTO invoice_operation "
            " (id, object_id, registrar_id, operation_id, zone_id" //4
            " , crdate, quantity, date_from,  date_to "
            " , registrar_credit_transaction_id) "
            "  VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint "
            " , CURRENT_TIMESTAMP::timestamp, $5::integer, $6::date, $7::date "
            " , $8::bigint) "
            //" RETURNING id "
            , Database::query_param_list(renewed_domain_id)
            (sponsoring_registrar_id)(operation_id)(zone_id)
            (length_of_domain_registration_in_years)
            (old_domain_expiration_date_local)
            (domain_expiration_date_local)
            (registrar_credit_transaction_id)
            );
    }

}//namespace Epp
