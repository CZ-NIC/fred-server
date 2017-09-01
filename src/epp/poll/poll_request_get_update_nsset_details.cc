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

#include "src/epp/poll/poll_request_get_update_nsset_details.h"
#include "src/epp/nsset/impl/nsset_output.h"
#include "src/epp/poll/message_type.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "util/db/param_query_composition.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {
namespace Poll {

namespace {

Epp::Nsset::InfoNssetOutputData get_nsset_output_data_by_history_id(
    Fred::OperationContext& _ctx,
    unsigned long long _history_id,
    unsigned long long _registrar_id)
{
    const Fred::InfoNssetData info_nsset_data =
        Fred::InfoNssetHistoryByHistoryid(_history_id).exec(_ctx).info_nsset_data;

    const std::string session_registrar_handle =
        Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle;
    const bool info_is_for_sponsoring_registrar =
        info_nsset_data.sponsoring_registrar_handle == session_registrar_handle;

    const std::vector<Fred::ObjectStateData> object_states_data =
        Fred::GetObjectStates(info_nsset_data.id).exec(_ctx);

    return Epp::Nsset::get_info_nsset_output(info_nsset_data, object_states_data, info_is_for_sponsoring_registrar);
}

} // namespace Epp::Poll::{anonymous}

PollRequestUpdateNssetOutputData poll_request_get_update_nsset_details(
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
              "JOIN nsset_history nh ON nh.historyid=pea.objid "
              "JOIN history h1 ON h1.next=pea.objid "
              "JOIN message m ON m.id=pea.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pea.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(Epp::Poll::MessageType::update_nsset));

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

    try {
        const PollRequestUpdateNssetOutputData ret(
            get_nsset_output_data_by_history_id(_ctx, old_history_id, _registrar_id),
            get_nsset_output_data_by_history_id(_ctx, new_history_id, _registrar_id));
        return ret;
    }
    catch (const Fred::InfoNssetHistoryByHistoryid::Exception&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
    catch (const Fred::OperationException&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
}

} // namespace Epp::Poll
} // namespace Epp
