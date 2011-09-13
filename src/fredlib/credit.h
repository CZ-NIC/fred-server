/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @file credit.h
 *  credit header
 */



#ifndef CREDIT_H_
#define CREDIT_H_

#include "decimal/decimal.h"
#include "types/money.h"
#include "db_settings.h"
#include "log/logger.h"
#include "log/context.h"


namespace Fred
{
    namespace Credit
    {

        struct add_credit_to_invoice
        {
            unsigned long long operator()(
                unsigned long long registrar_id
                , unsigned long long zone_id
                , Money price
                , unsigned long long invoice_id)
            {
                Logging::Context ctx("add_credit");

                Database::Connection conn = Database::Manager::acquire();

                Database::Result registrar_credit_id_result
                    = conn.exec_params("SELECT id FROM registrar_credit "
                            " WHERE registrar_id = $1::bigint AND zone_id = $2::bigint"
                            ,Database::query_param_list(registrar_id)(zone_id));

                unsigned long long registrar_credit_id = 0;
                if(registrar_credit_id_result.size() == 1 )
                {
                    registrar_credit_id = registrar_credit_id_result[0][0];
                }
                if(registrar_credit_id == 0)
                {
                    throw std::runtime_error("pay_invoice: registrar_credit not found");
                }

                Database::Result registrar_credit_transaction_id_result
                    = conn.exec_params(
                        "INSERT INTO registrar_credit_transaction "
                        " (id,balance_change, registrar_credit_id) "
                        " VALUES (DEFAULT, $1::numeric, $2::bigint) "
                        " RETURNING id"
                        ,Database::query_param_list(price.get_string())
                        (registrar_credit_id));

                unsigned long long registrar_credit_transaction_id = 0;
                if(registrar_credit_transaction_id_result.size() == 1 )
                {
                    registrar_credit_transaction_id = registrar_credit_transaction_id_result[0][0];
                }
                if(registrar_credit_transaction_id == 0)
                {
                    throw std::runtime_error("pay_invoice: registrar_credit_transaction not found");
                }

                //insert_invoice_registrar_credit_transaction_map
                conn.exec_params(
                "INSERT INTO invoice_registrar_credit_transaction_map "
                       " (invoice_id, registrar_credit_transaction_id) "
                       " VALUES ($1::bigint, $2::bigint) "
                ,Database::query_param_list(invoice_id)
                        (registrar_credit_transaction_id));

                return registrar_credit_transaction_id;

            }//operator()

        };//struct add_credit_to_invoice
    };//namespace Credit
};//namespace Fred


#endif // CREDIT_H_
