/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/backend/epp/poll/poll_request_get_update_contact_details.hh"

#include "src/backend/epp/contact/impl/contact_output.hh"
#include "src/backend/epp/contact/info_contact.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/poll/message_type.hh"
#include "src/libfred/object_state/get_object_states_by_history_id.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/util/db/param_query_composition.hh"
#include "src/util/tz/get_psql_handle_of.hh"
#include "src/util/tz/utc.hh"

namespace Epp {
namespace Poll {

namespace {

struct HistoryContactData
{
    Epp::Contact::InfoContactOutputData old_data;
    Epp::Contact::InfoContactOutputData new_data;
};

HistoryContactData get_history_contact_data(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        unsigned long long old_history_id,
        unsigned long long new_history_id)
{
    const std::string session_registrar_handle =
            LibFred::InfoRegistrarById(registrar_id).exec(ctx).info_registrar_data.handle;

    const LibFred::InfoContactData old_history_data =
            LibFred::InfoContactHistoryByHistoryid(old_history_id).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()).info_contact_data;
    const bool old_info_include_authinfo = old_history_data.sponsoring_registrar_handle == session_registrar_handle;
    const std::vector<LibFred::ObjectStateData> old_contact_states_data =
            LibFred::GetObjectStatesByHistoryId(old_history_id).exec(ctx).object_state_at_end;

    const LibFred::InfoContactData new_history_data =
            LibFred::InfoContactHistoryByHistoryid(new_history_id).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()).info_contact_data;
    const bool new_info_include_authinfo = new_history_data.sponsoring_registrar_handle == session_registrar_handle;
    const std::vector<LibFred::ObjectStateData> new_contact_states_data =
            LibFred::GetObjectStatesByHistoryId(new_history_id).exec(ctx).object_state_at_begin;

    HistoryContactData retval;
    retval.old_data = Epp::Contact::get_info_contact_output(
            old_history_data,
            old_contact_states_data,
            old_info_include_authinfo);
    retval.new_data = Epp::Contact::get_info_contact_output(
            new_history_data,
            new_contact_states_data,
            new_info_include_authinfo);
    return retval;
}

} // namespace Epp::Poll::{anonymous}

PollRequestUpdateContactOutputData poll_request_get_update_contact_details(
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
              "JOIN contact_history ch ON ch.historyid=pea.objid "
              "JOIN history h1 ON h1.next=pea.objid "
              "JOIN message m ON m.id=pea.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pea.msgid=").param_bigint(message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(Epp::Poll::MessageType::update_contact));

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
        const HistoryContactData history_contact_data = get_history_contact_data(
                ctx,
                registrar_id,
                old_history_id,
                new_history_id);
        PollRequestUpdateContactOutputData retval;
        retval.old_data = history_contact_data.old_data;
        retval.new_data = history_contact_data.new_data;
        return retval;
    }
    catch (const LibFred::InfoContactHistoryByHistoryid::Exception&)
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
