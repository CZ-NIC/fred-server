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

#include "src/backend/epp/poll/poll_acknowledgement.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "util/db/param_query_composition.hh"

namespace Epp {
namespace Poll {

PollAcknowledgementOutputData poll_acknowledgement(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    Database::ParamQuery sql_command;
    sql_command("UPDATE message m "
                "SET seen=true "
                "WHERE NOT seen AND "
                      "id=").param_bigint(_message_id)(" AND "
                      "clid=").param_bigint(_registrar_id)(" "
                "RETURNING (SELECT COUNT(*) FROM message WHERE clid=m.clid AND CURRENT_TIMESTAMP<exdate AND id!=m.id AND NOT seen),"
                          "(SELECT MIN(id) FROM message WHERE clid=m.clid AND CURRENT_TIMESTAMP<exdate AND id!=m.id AND NOT seen)");
    const Database::Result sql_command_result = _ctx.get_conn().exec_params(sql_command);
    if (sql_command_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_command_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    PollAcknowledgementOutputData poll_acknowledgement_output_data;

    poll_acknowledgement_output_data.number_of_unseen_messages =
        static_cast<unsigned long long>(sql_command_result[0][0]);
    if (poll_acknowledgement_output_data.number_of_unseen_messages == 0)
    {
        return poll_acknowledgement_output_data;
    }

    poll_acknowledgement_output_data.oldest_unseen_message_id =
        static_cast<unsigned long long>(sql_command_result[0][1]);

    return poll_acknowledgement_output_data;
}

} // namespace Epp::Poll
} // namespace Epp
