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

#include "src/libfred/poll/create_request_fee_info_message.hh"
#include "src/util/db/query_param.hh"
#include "src/libfred/opexception.hh"
#include "src/libfred/poll/message_type.hh"

namespace LibFred {
namespace Poll {

unsigned long long CreateRequestFeeInfoMessage::exec(OperationContext& _ctx) const
{
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "WITH create_new_message AS ( "
           "INSERT INTO message (clid, crdate, exdate, msgtype) "
           "SELECT $2::bigint, current_timestamp, current_timestamp + interval '7 days', id "
           "FROM messagetype WHERE name=$3::text "
           "RETURNING id AS msgid) "
        "INSERT INTO poll_request_fee (msgid, period_from, period_to, total_free_count, used_count, price) "
        "SELECT msgid, ($4::timestamp AT TIME ZONE $1::text) AT TIME ZONE 'UTC', "
        "($5::timestamp AT TIME ZONE $1::text) AT TIME ZONE 'UTC', "
        "$6::bigint, $7::bigint, $8::numeric(10,2) FROM create_new_message "
        "RETURNING msgid",
        Database::query_param_list
        (time_zone_)
        (registrar_id_)
        (Conversion::Enums::to_db_handle(MessageType::request_fee_info))
        (period_from_)
        (period_to_)
        (total_free_count_)
        (request_count_)
        (price_.get_string()));

    if (sql_query_result.size() == 1)
    {
        return static_cast<unsigned long long>(sql_query_result[0][0]);
    }

    struct UnexpectedNumberOfRows : InternalError
    {
        UnexpectedNumberOfRows() : InternalError("unexpected number of rows") { }
    };
    throw UnexpectedNumberOfRows();
}

} // namespace LibFred::Poll
} // namespace LibFred
