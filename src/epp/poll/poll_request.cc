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

#include "src/epp/poll/poll_request.h"
#include "src/epp/poll/message_type.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "util/db/param_query_composition.h"

#include <cstddef>

namespace Epp {
namespace Poll {

namespace {

TransferEvent get_transfer_event(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT oh.trdate::date AS transfer_date, obr.name AS handle, r.handle AS client_id "
              "FROM poll_eppaction pe "
              "JOIN object_history oh ON oh.historyid=pe.objid "
              "JOIN registrar r ON r.id=oh.clid "
              "JOIN object_registry obr ON obr.id=oh.id "
              "WHERE pe.msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const boost::gregorian::date transfer_date = boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
    const std::string handle = static_cast<std::string>(sql_query_result[0][1]);
    const std::string client_id = static_cast<std::string>(sql_query_result[0][2]);

    TransferEvent ret;
    switch(_message_type)
    {
        case MessageType::transfer_contact:
        {
            const TransferEvent::Data<TransferEvent::transfer_contact> specific_message = { transfer_date, handle, client_id };
            ret.message = specific_message;
        }
        break;
        case MessageType::transfer_nsset:
        {
            const TransferEvent::Data<TransferEvent::transfer_nsset> specific_message = { transfer_date, handle, client_id };
            ret.message = specific_message;
        }
        break;
        case MessageType::transfer_domain:
        {
            const TransferEvent::Data<TransferEvent::transfer_domain> specific_message = { transfer_date, handle, client_id };
            ret.message = specific_message;
        }
        break;
        case MessageType::transfer_keyset:
        {
            const TransferEvent::Data<TransferEvent::transfer_keyset> specific_message = { transfer_date, handle, client_id };
            ret.message = specific_message;
        }
        break;
    }

    return ret;
}

MessageEvent get_message_event_delete(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT obr.erdate::date AS date, obr.name AS handle "
              "FROM poll_eppaction pe "
              "JOIN object_registry obr ON obr.historyid=pe.objid "
              "WHERE pe.msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0 || sql_query_result[0][0].isnull())
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const boost::gregorian::date date = boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
    const std::string handle = static_cast<std::string>(sql_query_result[0][1]);

    MessageEvent ret;
    switch(_message_type)
    {
        case MessageType::delete_domain:
        {
            const MessageEvent::Data<MessageEvent::delete_domain> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::delete_contact:
        {
            const MessageEvent::Data<MessageEvent::delete_contact> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
    }

    return ret;
}

MessageEvent get_message_event_validation(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT COALESCE(eh.exdate::date, os.valid_from::date) AS date, obr.name AS handle "
              "FROM poll_statechange ps "
              "JOIN object_state os ON os.id=ps.stateid "
              "JOIN object_registry obr ON obr.id=os.object_id "
              "LEFT JOIN enumval_history eh ON eh.historyid=os.ohid_from "
              "WHERE pe.msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const boost::gregorian::date date = boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
    const std::string handle = static_cast<std::string>(sql_query_result[0][1]);

    MessageEvent ret;
    switch(_message_type)
    {
        case MessageType::validation:
        {
            const MessageEvent::Data<MessageEvent::validation> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::outzone:
        {
            const MessageEvent::Data<MessageEvent::outzone> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
    }

    return ret;
}

MessageEvent get_message_event_rest(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT COALESCE(dh.exdate::date, os.valid_from::date) AS date, obr.name AS handle "
              "FROM poll_statechange ps "
              "JOIN object_state os ON os.id=ps.stateid "
              "JOIN object_registry obr ON obr.id=os.object_id "
              "LEFT JOIN domain_history dh ON dh.historyid=os.ohid_from "
              "WHERE pe.msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const boost::gregorian::date date = boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
    const std::string handle = static_cast<std::string>(sql_query_result[0][1]);

    MessageEvent ret;
    switch(_message_type)
    {
        case MessageType::idle_delete_contact:
        {
            const MessageEvent::Data<MessageEvent::idle_delete_contact> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::idle_delete_nsset:
        {
            const MessageEvent::Data<MessageEvent::idle_delete_nsset> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::idle_delete_domain:
        {
            const MessageEvent::Data<MessageEvent::idle_delete_domain> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::idle_delete_keyset:
        {
            const MessageEvent::Data<MessageEvent::idle_delete_keyset> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::imp_expiration:
        {
            const MessageEvent::Data<MessageEvent::imp_expiration> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::expiration:
        {
            const MessageEvent::Data<MessageEvent::expiration> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
        case MessageType::imp_validation:
        {
            const MessageEvent::Data<MessageEvent::imp_validation> specific_message = { date, handle };
            ret.message = specific_message;
        }
        break;
    }

    return ret;
}

LowCreditEvent get_low_credit_event(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT z.fqdn, pc.credit, pc.credlimit "
              "FROM poll_credit pc "
              "JOIN zone z ON z.id=pc.zone "
              "WHERE pc.msgid=")
        .param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    LowCreditEvent ret;
    ret.zone = static_cast<std::string>(sql_query_result[0][0]);
    ret.credit = static_cast<std::string>(sql_query_result[0][1]);
    ret.limit = static_cast<std::string>(sql_query_result[0][2]);

    return ret;
}

TechCheckEvent get_tech_check_event(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id)
{
    // perhaps these two queries should be merged into one... TODO
    Database::ParamQuery handle_fqdns_query;
    handle_fqdns_query("SELECT obr.name AS handle, "
                       "UNNEST(CASE WHEN ARRAY_LENGTH(cn.extra_fqdns, 1) IS NOT NULL "
                       "THEN cn.extra_fqdns ELSE ARRAY[NULL::VARCHAR] END) AS fqdns "
                       "FROM poll_techcheck pt "
                       "JOIN check_nsset cn ON cn.id=pt.cnid "
                       "JOIN nsset_history nh ON nh.historyid=cn.nsset_hid "
                       "JOIN object_registry obr ON obr.id=nh.id "
                       "WHERE pt.msgid=").param_bigint(_message_id);
    const Database::Result handle_fqdns_query_result = _ctx.get_conn().exec_params(handle_fqdns_query);
    if (handle_fqdns_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    // the following query is a way too slow... TODO
    Database::ParamQuery tests_query;
    tests_query("SELECT ct.name, cr.status, cr.note "
                "FROM poll_techcheck pt "
                "JOIN check_result cr ON cr.checkid=pt.cnid "
                "JOIN check_test ct ON ct.id=cr.testid "
                "WHERE pt.msgid=").param_bigint(_message_id);
    const Database::Result tests_query_result = _ctx.get_conn().exec_params(tests_query);
    if (tests_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    TechCheckEvent ret;
    ret.handle = static_cast<std::string>(handle_fqdns_query_result[0][0]);
    if (! handle_fqdns_query_result[0][0].isnull())
    {
        for (std::size_t i = 0; i < handle_fqdns_query_result.size(); ++i)
        {
            ret.fqdns.push_back(static_cast<std::string>(handle_fqdns_query_result[i][1]));
        }
    }

    for (std::size_t i = 0; i < tests_query_result.size(); ++i)
    {
        Test test;
        test.testname = static_cast<std::string>(tests_query_result[i][0]);
        test.status = static_cast<int>(tests_query_result[i][1]);
        test.note = static_cast<std::string>(tests_query_result[i][2]);
        ret.tests.push_back(test);
    }

    return ret;
}

RequestFeeInfoEvent get_request_fee_info_event(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT period_from, period_to, total_free_count, used_count, price "
              "FROM poll_request_fee "
              "WHERE msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    RequestFeeInfoEvent ret;
    ret.from = boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][0]));
    ret.to = boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][1]));
    ret.free_count = static_cast<unsigned long long>(sql_query_result[0][2]);
    ret.used_count = static_cast<unsigned long long>(sql_query_result[0][3]);
    ret.price = static_cast<std::string>(sql_query_result[0][4]);
    return ret;
}

UpdateInfoEvent get_update_info_event(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT h2.request_id "
              "FROM poll_eppaction pe "
              "JOIN history h1 ON h1.next=pe.objid "
              "JOIN history h2 ON h2.id=h1.next "
              "WHERE pe.msgid=").param_bigint(_message_id);
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    unsigned long long transaction_id = static_cast<unsigned long long>(sql_query_result[0][0]);

    UpdateInfoEvent ret;
    switch(_message_type)
    {
        case MessageType::update_domain:
        {
            const UpdateInfoEvent::Data<UpdateInfoEvent::update_domain> specific_message = { transaction_id, _message_id };
            ret.message = specific_message;
        }
        break;
        case MessageType::update_nsset:
        {
            const UpdateInfoEvent::Data<UpdateInfoEvent::update_nsset> specific_message = { transaction_id, _message_id };
            ret.message = specific_message;
        }
        break;
        case MessageType::update_keyset:
        {
            const UpdateInfoEvent::Data<UpdateInfoEvent::update_keyset> specific_message = { transaction_id, _message_id };
            ret.message = specific_message;
        }
        break;
    }

    return ret;
}

} // namespace Epp::Poll::{anonymous}

PollRequestOutputData poll_request(
    Fred::OperationContext& _ctx,
    unsigned long long _registrar_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    Database::ParamQuery sql_query;
    sql_query("SELECT m.id, mt.name, m.crdate, t.count "
              "FROM (SELECT min(id) AS id, count(*) FROM message WHERE clid=")
        .param_bigint(_registrar_id)(" AND exdate>CURRENT_TIMESTAMP AND NOT seen) AS t"
                                     " JOIN message m ON m.id=t.id"
                                     " JOIN messagetype mt on mt.id=m.msgtype");
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    PollRequestOutputData poll_request_output_data;

    poll_request_output_data.message_id = static_cast<unsigned long long>(sql_query_result[0][0]);
    poll_request_output_data.creation_time =
        boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][2]));
    poll_request_output_data.number_of_unseen_messages = static_cast<unsigned long long>(sql_query_result[0][3]);

    MessageType::Enum message_type =
        Conversion::Enums::from_db_handle<MessageType>(static_cast<std::string>(sql_query_result[0][1]));

    Database::ParamQuery specific_message_query;
    switch(message_type)
    {
        case MessageType::transfer_contact:
        case MessageType::transfer_nsset:
        case MessageType::transfer_domain:
        case MessageType::transfer_keyset:
            poll_request_output_data.message = get_transfer_event(_ctx, poll_request_output_data.message_id, message_type);
            break;
        case MessageType::delete_domain:
        case MessageType::delete_contact:
            poll_request_output_data.message = get_message_event_delete(_ctx, poll_request_output_data.message_id, message_type);
            break;
        case MessageType::validation:
        case MessageType::outzone:
            poll_request_output_data.message = get_message_event_validation(_ctx, poll_request_output_data.message_id, message_type);
            break;
        case MessageType::idle_delete_contact:
        case MessageType::idle_delete_nsset:
        case MessageType::idle_delete_domain:
        case MessageType::idle_delete_keyset:
        case MessageType::imp_expiration:
        case MessageType::expiration:
        case MessageType::imp_validation:
            poll_request_output_data.message = get_message_event_rest(_ctx, poll_request_output_data.message_id, message_type);
            break;
        case MessageType::credit:
            poll_request_output_data.message = get_low_credit_event(_ctx, poll_request_output_data.message_id);
            break;
        case MessageType::techcheck:
            poll_request_output_data.message = get_tech_check_event(_ctx, poll_request_output_data.message_id);
            break;
        case MessageType::request_fee_info:
            poll_request_output_data.message = get_request_fee_info_event(_ctx, poll_request_output_data.message_id);
            break;
        case MessageType::update_domain:
        case MessageType::update_nsset:
        case MessageType::update_keyset:
            poll_request_output_data.message = get_update_info_event(_ctx, poll_request_output_data.message_id, message_type);
            break;
        default:
            throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return poll_request_output_data;
}

} // namespace Epp::Poll
} // namespace Epp
