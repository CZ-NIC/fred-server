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

#include "src/epp/poll/poll_request_get_update_keyset_details.h"
#include "src/epp/keyset/impl/keyset_output.h"
#include "src/epp/poll/message_type.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "util/db/param_query_composition.h"
#include "src/fredlib/keyset/info_keyset.h"

namespace Epp {
namespace Poll {

PollRequestUpdateKeysetOutputData poll_request_get_update_keyset_details(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    Database::ParamQuery sql_query;
    sql_query("SELECT h1.id, h1.next "
              "FROM poll_eppaction pea "
              "JOIN keyset_history kh ON kh.historyid=pea.objid "
              "JOIN history h1 ON h1.next=pea.objid "
              "JOIN message m ON m.id=pea.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pea.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(Epp::Poll::MessageType::update_keyset));

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const unsigned long long old_history_id = static_cast<unsigned long long>(sql_query_result[0][0]);
    const unsigned long long new_history_id = static_cast<unsigned long long>(sql_query_result[0][1]);

    PollRequestUpdateKeysetOutputData ret;
    try {
        const Fred::InfoKeysetData old_history_data =
            Fred::InfoKeysetHistoryByHistoryid(old_history_id).exec(_ctx).info_keyset_data;
        ret.old_data = Epp::Keyset::get_info_keyset_output(_ctx, old_history_data, _registrar_id);
        const Fred::InfoKeysetData new_history_data =
            Fred::InfoKeysetHistoryByHistoryid(new_history_id).exec(_ctx).info_keyset_data;
        ret.new_data = Epp::Keyset::get_info_keyset_output(_ctx, new_history_data, _registrar_id);

    }
    catch (const Fred::InfoKeysetHistoryByHistoryid::Exception&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
    catch (const Fred::OperationException&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

} // namespace Epp::Poll
} // namespace Epp
