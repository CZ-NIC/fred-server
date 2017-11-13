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
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/epp/domain/impl/domain_output.h"
#include "src/epp/domain/info_domain.h"
#include "util/db/param_query_composition.h"
#include "util/tz/utc.hh"
#include "util/tz/get_psql_handle_of.hh"

namespace Epp {
namespace Poll {

namespace {

Epp::Domain::InfoDomainOutputData get_domain_output_data_by_history_id(
        Fred::OperationContext& ctx,
        unsigned long long history_id,
        unsigned long long registrar_id)
{
    const Fred::InfoDomainData info_domain_data =
        Fred::InfoDomainHistoryByHistoryid(history_id).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()).info_domain_data;

    const std::string session_registrar_handle =
        Fred::InfoRegistrarById(registrar_id).exec(ctx).info_registrar_data.handle;
    const bool info_is_for_sponsoring_registrar =
        info_domain_data.sponsoring_registrar_handle == session_registrar_handle;

    const std::vector<Fred::ObjectStateData> object_state_data =
        Fred::GetObjectStates(info_domain_data.id).exec(ctx);

    return Epp::Domain::get_info_domain_output(info_domain_data, object_state_data, info_is_for_sponsoring_registrar);
}

}//namespace Epp::Poll::{anonymous}

PollRequestUpdateDomainOutputData poll_request_get_update_domain_details(
        Fred::OperationContext& ctx,
        unsigned long long message_id,
        unsigned long long registrar_id)
{
    const bool registrar_is_authenticated = registrar_id != 0;
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
              "WHERE pea.msgid=").param_bigint(message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(Epp::Poll::MessageType::update_domain));

    const Database::Result sql_query_result = ctx.get_conn().exec_params(sql_query);
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

    try
    {
        PollRequestUpdateDomainOutputData ret;
        ret.old_data = get_domain_output_data_by_history_id(ctx, old_history_id, registrar_id);
        ret.new_data = get_domain_output_data_by_history_id(ctx, new_history_id, registrar_id);
        return ret;
    }
    catch (const Fred::InfoDomainHistoryByHistoryid::Exception&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
    catch (const Fred::OperationException&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
}

}//namespace Epp::Poll
}//namespace Epp
