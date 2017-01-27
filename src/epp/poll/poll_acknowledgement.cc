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

#include "src/epp/poll/poll_acknowledgement.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "util/db/param_query_composition.h"

#include <boost/lexical_cast.hpp>

namespace Epp {
namespace Poll {

PollAcknowledgementOutputData poll_acknowledgement(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    PollAcknowledgementOutputData poll_acknowledgement_output_data;

    Database::ParamQuery mark_message_as_seen;
    mark_message_as_seen("UPDATE message SET seen='t' WHERE id=")
        .param_bigint(_message_id)(" AND clid=")
        .param_bigint(_registrar_id);

    const Database::Result mark_message_result = _ctx.get_conn().exec_params(mark_message_as_seen);
    if(mark_message_result.rows_affected() != 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    Database::ParamQuery get_number_of_unseen_messages;
    get_number_of_unseen_messages("SELECT COUNT(DISTINCT id) FROM message WHERE clid=")
        .param_bigint(_registrar_id)(" AND exdate>CURRENT_TIMESTAMP AND NOT seen");

    const Database::Result number_of_unseen_messages_result = _ctx.get_conn().exec_params(get_number_of_unseen_messages);
    if(number_of_unseen_messages_result.size() != 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    poll_acknowledgement_output_data.number_of_unseen_messages =
        static_cast<unsigned long long>(number_of_unseen_messages_result[0][0]);
    if(poll_acknowledgement_output_data.number_of_unseen_messages == 0)
    {
        throw EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully_no_messages));
    }

    Database::ParamQuery get_oldest_unseen_message_id;
    get_oldest_unseen_message_id("SELECT id FROM message WHERE clid=")
        .param_bigint(_registrar_id)(" AND exdate>CURRENT_TIMESTAMP AND NOT seen ORDER BY id ASC LIMIT 1");

    const Database::Result oldest_unseen_message_id_result = _ctx.get_conn().exec_params(get_oldest_unseen_message_id);
    if(oldest_unseen_message_id_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    poll_acknowledgement_output_data.oldest_unseen_message_id =
        static_cast<std::string>(oldest_unseen_message_id_result[0][0]);

    return poll_acknowledgement_output_data;
}

} // namespace Epp::Poll
} // namespace Poll
