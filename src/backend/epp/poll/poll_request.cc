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

#include "src/backend/epp/poll/poll_request.hh"
#include "src/backend/epp/poll/message_type.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "util/db/param_query_composition.hh"

#include <cstddef>
#include <stdexcept>

namespace Epp {
namespace Poll {

Test::Test(const std::string& _testname, const std::string& _note, int _status)
    : testname(_testname),
      note(_note),
      status(_status)
{
}

bool Test::is_test_successful()const
{
    switch (status)
    {
        case 0:
            return true;
        case 1:
            return false;
        case 2:
            return false;
    }
    struct UnexpectedStatusValue:std::runtime_error
    {
        UnexpectedStatusValue():std::runtime_error("Unexpected value of status") { }
    };
    throw UnexpectedStatusValue();
}

int Test::get_status()const
{
    return status;
}

namespace {

TransferEvent get_transfer_event(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT oh.trdate::date, obr.name, r.handle "
              "FROM poll_eppaction pe "
              "JOIN object_history oh ON oh.historyid=pe.objid "
              "JOIN registrar r ON r.id=oh.clid "
              "JOIN object_registry obr ON obr.id=oh.id "
              "JOIN message m ON m.id=pe.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pe.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const boost::gregorian::date transfer_date =
        boost::gregorian::from_string(static_cast<std::string>(sql_query_result[0][0]));
    const std::string object_handle = static_cast<std::string>(sql_query_result[0][1]);
    const std::string dst_registrar_handle = static_cast<std::string>(sql_query_result[0][2]);

    TransferEvent ret;
    switch (_message_type)
    {
        case MessageType::transfer_contact:
        {
            const TransferEvent::Data<TransferEvent::TransferContact> specific_message =
                { transfer_date, object_handle, dst_registrar_handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::transfer_nsset:
        {
            const TransferEvent::Data<TransferEvent::TransferNsset> specific_message =
                { transfer_date, object_handle, dst_registrar_handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::transfer_domain:
        {
            const TransferEvent::Data<TransferEvent::TransferDomain> specific_message =
                { transfer_date, object_handle, dst_registrar_handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::transfer_keyset:
        {
            const TransferEvent::Data<TransferEvent::TransferKeyset> specific_message =
                { transfer_date, object_handle, dst_registrar_handle };
            ret.message = specific_message;
            break;
        }
        default:
            throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

MessageEvent get_message_event_delete(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT obr.erdate::date, obr.name "
              "FROM poll_eppaction pe "
              "JOIN object_registry obr ON obr.historyid=pe.objid "
              "JOIN message m ON m.id=pe.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE obr.erdate IS NOT NULL AND pe.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
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
    switch (_message_type)
    {
        case MessageType::delete_domain:
        {
            const MessageEvent::Data<MessageEvent::DeleteDomain> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::delete_contact:
        {
            const MessageEvent::Data<MessageEvent::DeleteContact> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        default:
            throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

MessageEvent get_message_event_validation(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT COALESCE(eh.exdate::date, os.valid_from::date), obr.name "
              "FROM poll_statechange ps "
              "JOIN object_state os ON os.id=ps.stateid "
              "JOIN object_registry obr ON obr.id=os.object_id "
              "JOIN message m ON m.id=ps.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "LEFT JOIN enumval_history eh ON eh.historyid=os.ohid_from "
              "WHERE ps.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
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
    switch (_message_type)
    {
        case MessageType::validation:
        {
            const MessageEvent::Data<MessageEvent::Validation> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::imp_validation:
        {
            const MessageEvent::Data<MessageEvent::ImpValidation> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        default:
            throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

MessageEvent get_message_event_rest(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT COALESCE(dh.exdate::date, os.valid_from::date), obr.name "
              "FROM poll_statechange ps "
              "JOIN object_state os ON os.id=ps.stateid "
              "JOIN object_registry obr ON obr.id=os.object_id "
              "JOIN message m ON m.id=ps.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "LEFT JOIN domain_history dh ON dh.historyid=os.ohid_from "
              "WHERE ps.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
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
    switch (_message_type)
    {
        case MessageType::idle_delete_contact:
        {
            const MessageEvent::Data<MessageEvent::IdleDeleteContact> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::idle_delete_nsset:
        {
            const MessageEvent::Data<MessageEvent::IdleDeleteNsset> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::idle_delete_domain:
        {
            const MessageEvent::Data<MessageEvent::IdleDeleteDomain> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::idle_delete_keyset:
        {
            const MessageEvent::Data<MessageEvent::IdleDeleteKeyset> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::imp_expiration:
        {
            const MessageEvent::Data<MessageEvent::ImpExpiration> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::expiration:
        {
            const MessageEvent::Data<MessageEvent::Expiration> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        case MessageType::outzone:
        {
            const MessageEvent::Data<MessageEvent::Outzone> specific_message = { date, handle };
            ret.message = specific_message;
            break;
        }
        default:
            throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

LowCreditEvent get_low_credit_event(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT z.fqdn, pc.credit, pc.credlimit "
              "FROM poll_credit pc "
              "JOIN zone z ON z.id=pc.zone "
              "JOIN message m ON m.id=pc.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pc.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
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

std::vector<Test> get_tech_check_event_tests(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT ct.name, cr.status, cr.note "
              "FROM poll_techcheck pt "
              "JOIN check_result cr ON cr.checkid=pt.cnid "
              "JOIN check_test ct ON ct.id=cr.testid "
              "JOIN message m ON m.id=pt.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pt.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);

    std::vector<Test> ret;
    for (std::size_t idx = 0; idx < sql_query_result.size(); ++idx)
    {
        const std::string testname = static_cast<std::string>(sql_query_result[idx][0]);
        const int status = static_cast<int>(sql_query_result[idx][1]);
        const std::string note = static_cast<std::string>(sql_query_result[idx][2]);
        ret.push_back(Test(testname, note, status));
    }

    return ret;
}

TechCheckEvent get_tech_check_event(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT obr.name, "
              "UNNEST(CASE WHEN COALESCE(ARRAY_LENGTH(cn.extra_fqdns, 1), 0) > 0 "
              "THEN cn.extra_fqdns ELSE ARRAY[NULL::VARCHAR] END) "
              "FROM poll_techcheck pt "
              "JOIN check_nsset cn ON cn.id=pt.cnid "
              "JOIN nsset_history nh ON nh.historyid=cn.nsset_hid "
              "JOIN object_registry obr ON obr.id=nh.id "
              "JOIN message m ON m.id=pt.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pt.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    TechCheckEvent ret;
    ret.handle = static_cast<std::string>(sql_query_result[0][0]);
    if (!sql_query_result[0][1].isnull())
    {
        for (std::size_t i = 0; i < sql_query_result.size(); ++i)
        {
            ret.fqdns.push_back(static_cast<std::string>(sql_query_result[i][1]));
        }
    }

    ret.tests = get_tech_check_event_tests(_ctx, _message_id, _message_type);

    return ret;
}

RequestFeeInfoEvent get_request_fee_info_event(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT prf.period_from, prf.period_to, prf.total_free_count, prf.used_count, prf.price "
              "FROM poll_request_fee prf "
              "JOIN message m ON m.id=prf.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE prf.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
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
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    MessageType::Enum _message_type)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT h2.request_id "
              "FROM poll_eppaction pe "
              "JOIN history h1 ON h1.next=pe.objid "
              "JOIN history h2 ON h2.id=h1.next "
              "JOIN message m ON m.id=pe.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE pe.msgid=").param_bigint(_message_id)(" AND mt.name=")
              .param_text(Conversion::Enums::to_db_handle(_message_type));
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const unsigned long long transaction_id = static_cast<unsigned long long>(sql_query_result[0][0]);

    UpdateInfoEvent ret;
    switch (_message_type)
    {
        case MessageType::update_contact:
        {
            const UpdateInfoEvent::Data<UpdateInfoEvent::UpdateContact> specific_message = { transaction_id, _message_id };
            ret.message = specific_message;
            break;
        }
        case MessageType::update_domain:
        {
            const UpdateInfoEvent::Data<UpdateInfoEvent::UpdateDomain> specific_message = { transaction_id, _message_id };
            ret.message = specific_message;
            break;
        }
        case MessageType::update_nsset:
        {
            const UpdateInfoEvent::Data<UpdateInfoEvent::UpdateNsset> specific_message = { transaction_id, _message_id };
            ret.message = specific_message;
            break;
        }
        case MessageType::update_keyset:
        {
            const UpdateInfoEvent::Data<UpdateInfoEvent::UpdateKeyset> specific_message = { transaction_id, _message_id };
            ret.message = specific_message;
            break;
        }
        default:
            throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

} // namespace Epp::Poll::{anonymous}

PollRequestOutputData poll_request(
    LibFred::OperationContext& _ctx,
    unsigned long long _registrar_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    Database::ParamQuery sql_query;
    sql_query("SELECT m.id, mt.name, m.crdate, t.cnt "
              "FROM (SELECT min(id) AS id, count(*) AS cnt FROM message WHERE clid=")
        .param_bigint(_registrar_id)(" AND exdate>CURRENT_TIMESTAMP AND NOT seen) AS t "
              "JOIN message m ON m.id=t.id "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "FOR SHARE OF m");
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    PollRequestOutputData poll_request_output_data;
    if (sql_query_result.size() == 0)
    {
        poll_request_output_data.number_of_unseen_messages = 0;
        return poll_request_output_data;
    }
    if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    poll_request_output_data.message_id = static_cast<unsigned long long>(sql_query_result[0][0]);
    poll_request_output_data.creation_time =
        boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][2]));
    poll_request_output_data.number_of_unseen_messages = static_cast<unsigned long long>(sql_query_result[0][3]);

    const MessageType::Enum message_type =
        Conversion::Enums::from_db_handle<MessageType>(static_cast<std::string>(sql_query_result[0][1]));

    Database::ParamQuery specific_message_query;
    switch (message_type)
    {
        case MessageType::transfer_contact:
        case MessageType::transfer_nsset:
        case MessageType::transfer_domain:
        case MessageType::transfer_keyset:
            poll_request_output_data.message = get_transfer_event(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
        case MessageType::delete_domain:
        case MessageType::delete_contact:
            poll_request_output_data.message = get_message_event_delete(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
        case MessageType::validation:
        case MessageType::imp_validation:
            poll_request_output_data.message = get_message_event_validation(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
        case MessageType::idle_delete_contact:
        case MessageType::idle_delete_nsset:
        case MessageType::idle_delete_domain:
        case MessageType::idle_delete_keyset:
        case MessageType::imp_expiration:
        case MessageType::expiration:
        case MessageType::outzone:
            poll_request_output_data.message = get_message_event_rest(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
        case MessageType::credit:
            poll_request_output_data.message = get_low_credit_event(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
        case MessageType::techcheck:
            poll_request_output_data.message = get_tech_check_event(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
        case MessageType::request_fee_info:
            poll_request_output_data.message = get_request_fee_info_event(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
        case MessageType::update_contact:
        case MessageType::update_domain:
        case MessageType::update_nsset:
        case MessageType::update_keyset:
            poll_request_output_data.message = get_update_info_event(_ctx, poll_request_output_data.message_id, message_type);
            return poll_request_output_data;
    }
    throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
}

} // namespace Epp::Poll
} // namespace Epp
