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

#include "src/epp/poll/poll_request_get_update_domain_details.h"
#include "src/epp/poll/message_type.h"
#include "src/epp/domain/impl/get_domain_info.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "util/db/param_query_composition.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {
namespace Poll {

namespace {

Epp::Domain::InfoDomainOutputData get_info_domain_output_data(
    Fred::OperationContext& _ctx,
    unsigned long long _session_registrar_id,
    unsigned long long _history_id)
{
    const Fred::InfoDomainData info_domain_data =
        Fred::InfoDomainHistoryByHistoryid(_history_id).exec(_ctx).info_domain_data;

    const std::vector<Fred::ObjectStateData> domain_states_data = Fred::GetObjectStates(info_domain_data.id).exec(_ctx);

    const std::string callers_registrar_handle =
        Fred::InfoRegistrarById(_session_registrar_id).exec(_ctx).info_registrar_data.handle;
    const bool callers_is_sponsoring_registrar = info_domain_data.sponsoring_registrar_handle == callers_registrar_handle;
    const bool authinfopw_has_to_be_hidden = !callers_is_sponsoring_registrar;

    return Epp::Domain::get_domain_info(info_domain_data, domain_states_data, authinfopw_has_to_be_hidden);
}

} // namespace Epp::Poll::{anonymous}

PollRequestUpdateDomainOutputData poll_request_get_update_domain_details(
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
              "JOIN domain_history dh ON dh.historyid=pea.objid "
              "JOIN history h1 ON h1.next=pea.objid "
              "JOIN message m ON m.id=pea.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pea.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(Epp::Poll::MessageType::update_domain));

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

    PollRequestUpdateDomainOutputData ret;
    try {
        ret.old_data = get_info_domain_output_data(_ctx, _registrar_id, old_history_id);
        ret.new_data = get_info_domain_output_data(_ctx, _registrar_id, new_history_id);
    }
    catch (const Fred::InfoDomainHistoryByHistoryid::Exception&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
    catch (const Fred::OperationException&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

} // namespace Epp::Poll
} // namespace Epp
