/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file credit.cc
 *  credit implementation
 */

#include "src/deprecated/libfred/credit.hh"
#include "src/deprecated/libfred/db_settings.hh"

namespace LibFred
{
    namespace Credit
    {

         unsigned long long add_credit_to_invoice(
             unsigned long long registrar_id
             , unsigned long long zone_id
             , Money price
             , unsigned long long invoice_id)
         {
             Logging::Context ctx("add_credit");

             Database::Connection conn = Database::Manager::acquire();

             Database::Result registrar_credit_id_result
                 = conn.exec_params(
                         "SELECT id FROM registrar_credit "
                         " WHERE registrar_id = $1::bigint AND zone_id = $2::bigint"
                         , Database::query_param_list(registrar_id)(zone_id));

             unsigned long long registrar_credit_id = 0;
             if(registrar_credit_id_result.size() == 1 )
             {
                 registrar_credit_id = registrar_credit_id_result[0][0];
             }
             if(registrar_credit_id == 0)
             {
                 throw std::runtime_error("add_credit_to_invoice: registrar_credit not found");
             }

             Database::Result registrar_credit_transaction_id_result
                 = conn.exec_params(
                     "INSERT INTO registrar_credit_transaction "
                     " (id,balance_change, registrar_credit_id) "
                     " VALUES (DEFAULT, $1::numeric, $2::bigint) "
                     " RETURNING id"
                     , Database::query_param_list
                            (price.get_string())
                            (registrar_credit_id));

             unsigned long long registrar_credit_transaction_id = 0;
             if(registrar_credit_transaction_id_result.size() == 1 )
             {
                 registrar_credit_transaction_id = registrar_credit_transaction_id_result[0][0];
             }
             if(registrar_credit_transaction_id == 0)
             {
                 throw std::runtime_error("add_credit_to_invoice: registrar_credit_transaction not found");
             }

             //insert_invoice_registrar_credit_transaction_map
             conn.exec_params(
                "INSERT INTO invoice_registrar_credit_transaction_map "
                " (invoice_id, registrar_credit_transaction_id) "
                " VALUES ($1::bigint, $2::bigint) "
                , Database::query_param_list
                     (invoice_id)
                     (registrar_credit_transaction_id));

             return registrar_credit_transaction_id;
         }

         void init_new_registrar_credit (Database::ID reg_id, Database::ID zone_id) 
         {
            Database::Connection conn = Database::Manager::acquire();
            // init credit for a new registrar to 0
            conn.exec_params("INSERT INTO registrar_credit (credit, registrar_id, zone_id) VALUES "
                "(0, "
                "($1::bigint), "
                "($2::bigint)) "
                "ON CONFLICT DO NOTHING",
                Database::query_param_list
                        (reg_id)
                        (zone_id) );
         }


    }//namespace Credit
} // namespace LibFred
