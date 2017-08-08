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

#include "src/fredlib/poll/get_request_fee_message.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/poll/message_type.h"
#include "util/db/param_query_composition.h"

#include <string>

namespace Fred {
namespace Poll {

namespace {

RequestFeeInfoEvent get_request_fee_info_message_impl(
        Fred::OperationContext& ctx,
        unsigned long long registrar_id,
        const Database::ParamQuery& query_part)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT prf.period_from, prf.period_to, prf.total_free_count, prf.used_count, prf.price "
              "FROM poll_request_fee prf "
              "JOIN message m ON m.id=prf.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE m.clid=").param_bigint(registrar_id)
              (" AND mt.name=").param_text(Conversion::Enums::to_db_handle(MessageType::request_fee_info))
              (query_part);

    const Database::Result sql_query_result = ctx.get_conn().exec_params(sql_query);
    switch (sql_query_result.size())
    {
        case 0:
        {
            struct NotFound : OperationException
            {
                const char* what() const throw() { return "no message was found"; }
            };
            throw NotFound();
        }
        case 1:
            break;
        default:
        {
            struct TooManyRows : InternalError
            {
                TooManyRows() : InternalError("too many rows") { }
            };
            throw TooManyRows();
        }
    }

    RequestFeeInfoEvent ret;
    ret.from = boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][0]));
    ret.to = boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][1]));
    ret.free_count = static_cast<unsigned long long>(sql_query_result[0][2]);
    ret.used_count = static_cast<unsigned long long>(sql_query_result[0][3]);
    ret.price = static_cast<std::string>(sql_query_result[0][4]);
    return ret;
}

} // namespace Fred::Poll::{anonymous}

RequestFeeInfoEvent get_request_fee_info_message(
        Fred::OperationContext& ctx,
        unsigned long long registrar_id,
        const boost::posix_time::ptime& period_to,
        const std::string& time_zone)
{
    Database::ParamQuery by_period_to_sql_part(" AND period_to = (");
    by_period_to_sql_part.param_timestamp(period_to)(" AT TIME ZONE ").param_text(time_zone)(" AT TIME ZONE 'UTC')");
    return get_request_fee_info_message_impl(ctx, registrar_id, by_period_to_sql_part);
}

RequestFeeInfoEvent get_last_request_fee_info_message(
        Fred::OperationContext& ctx,
        unsigned long long registrar_id)
{
    const Database::ParamQuery last_sql_part(" ORDER BY m.id DESC LIMIT 1");
    return get_request_fee_info_message_impl(ctx, registrar_id, last_sql_part);
}

} // namespace Fred::Poll
} // namespace Fred
