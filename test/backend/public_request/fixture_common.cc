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
#include "test/backend/public_request/fixture_common.hh"
#include "libfred/db_settings.hh"

Database::Result get_db_public_request(
    const ::LibFred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status)
{
    return ctx.get_conn().exec_params(
            "SELECT id,request_type,create_time,status,resolve_time,reason,email_to_answer,"
                   "answer_email_id,registrar_id,create_request_id,resolve_request_id "
            "FROM public_request "
            "WHERE id=$1::bigint AND "
                  "request_type=$2::smallint AND "
                  "status=$3::smallint AND "
                  "email_to_answer IS NULL AND "
                  "registrar_id IS NULL ",
            Database::query_param_list(id)(type)(status));
}

Database::Result get_db_public_request(
    const ::LibFred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status,
    const std::string& email_to_answer)
{
    return ctx.get_conn().exec_params(
            "SELECT id,request_type,create_time,status,resolve_time,reason,email_to_answer,"
                   "answer_email_id,registrar_id,create_request_id,resolve_request_id "
            "FROM public_request "
            "WHERE id=$1::bigint AND "
                  "request_type=$2::smallint AND "
                  "status=$3::smallint AND "
                  "email_to_answer=$4::text AND "
                  "registrar_id IS NULL ",
            Database::query_param_list(id)(type)(status)(email_to_answer));
}
