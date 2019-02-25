/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/poll/poll_request_get_update_domain_details.hh"
#include "src/backend/epp/poll/message_type.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/object_state/get_object_states_by_history_id.hh"
#include "libfred/registrar/info_registrar.hh"
#include "src/backend/epp/domain/impl/domain_output.hh"
#include "src/backend/epp/domain/info_domain.hh"
#include "util/db/param_query_composition.hh"
#include "src/util/tz/utc.hh"
#include "src/util/tz/get_psql_handle_of.hh"

namespace Epp {
namespace Poll {

namespace {

struct HistoryDomainData
{
    Epp::Domain::InfoDomainOutputData old_data;
    Epp::Domain::InfoDomainOutputData new_data;
};

HistoryDomainData get_history_domain_data(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        unsigned long long old_history_id,
        unsigned long long new_history_id)
{
    const std::string session_registrar_handle =
            LibFred::InfoRegistrarById(registrar_id).exec(ctx).info_registrar_data.handle;

    const LibFred::InfoDomainData old_history_data =
            LibFred::InfoDomainHistoryByHistoryid(old_history_id).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()).info_domain_data;
    const bool old_info_is_for_sponsoring_registrar = old_history_data.sponsoring_registrar_handle == session_registrar_handle;
    const std::vector<LibFred::ObjectStateData> old_domain_states_data =
            LibFred::GetObjectStatesByHistoryId(old_history_id).exec(ctx).object_state_at_end;

    const LibFred::InfoDomainData new_history_data =
            LibFred::InfoDomainHistoryByHistoryid(new_history_id).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()).info_domain_data;
    const bool new_info_is_for_sponsoring_registrar = new_history_data.sponsoring_registrar_handle == session_registrar_handle;
    const std::vector<LibFred::ObjectStateData> new_domain_states_data =
            LibFred::GetObjectStatesByHistoryId(new_history_id).exec(ctx).object_state_at_begin;

    HistoryDomainData retval;
    retval.old_data = Epp::Domain::get_info_domain_output(
            old_history_data,
            old_domain_states_data,
            old_info_is_for_sponsoring_registrar);
    retval.new_data = Epp::Domain::get_info_domain_output(
            new_history_data,
            new_domain_states_data,
            new_info_is_for_sponsoring_registrar);
    return retval;
}

} // namespace Epp::Poll::{anonymous}

PollRequestUpdateDomainOutputData poll_request_get_update_domain_details(
        LibFred::OperationContext& ctx,
        unsigned long long message_id,
        unsigned long long registrar_id)
{
    const bool registrar_is_authenticated = registrar_id != 0;
    if (!registrar_is_authenticated)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    Database::ParamQuery sql_query;
    sql_query("SELECT h1.id,h1.next "
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
        const HistoryDomainData history_domain_data = get_history_domain_data(
                ctx,
                registrar_id,
                old_history_id,
                new_history_id);
        PollRequestUpdateDomainOutputData retval;
        retval.old_data = history_domain_data.old_data;
        retval.new_data = history_domain_data.new_data;
        return retval;
    }
    catch (const LibFred::InfoDomainHistoryByHistoryid::Exception&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
    catch (const LibFred::OperationException&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
}

} // namespace Epp::Poll
} // namespace Epp
