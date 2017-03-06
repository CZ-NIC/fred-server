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
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "util/db/param_query_composition.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {
namespace Poll {

namespace {

// this could be shared with info_keyset
Epp::Keyset::InfoKeysetOutputData get_info_keyset_output_data(
    Fred::OperationContext& _ctx,
    unsigned long long _registrar_id,
    unsigned long long _history_id)
{
    Epp::Keyset::InfoKeysetOutputData ret;

    const Fred::InfoKeysetData data = Fred::InfoKeysetHistoryByHistoryid(_history_id).exec(_ctx).info_keyset_data;
    ret.handle = data.handle;
    ret.roid = data.roid;
    ret.sponsoring_registrar_handle = data.sponsoring_registrar_handle;
    ret.creating_registrar_handle = data.create_registrar_handle;
    ret.last_update_registrar_handle = data.update_registrar_handle;
    {
        typedef std::vector<Fred::ObjectStateData> ObjectStatesData;
        ObjectStatesData keyset_states_data = Fred::GetObjectStates(data.id).exec(_ctx);
        for (ObjectStatesData::const_iterator data_ptr = keyset_states_data.begin();
             data_ptr != keyset_states_data.end(); ++data_ptr)
        {
            ret.states.insert(Conversion::Enums::from_db_handle<Fred::Object_State>(data_ptr->state_name));
        }
    }
    ret.crdate = data.creation_time;
    ret.last_update = data.update_time;
    ret.last_transfer = data.transfer_time;
    if (Fred::InfoRegistrarByHandle(data.sponsoring_registrar_handle).exec(_ctx).info_registrar_data.id == _registrar_id)
    {
        ret.authinfopw = data.authinfopw;
    }
    {
        typedef std::vector<Fred::DnsKey> FredDnsKeys;
        for (FredDnsKeys::const_iterator data_ptr = data.dns_keys.begin();
             data_ptr != data.dns_keys.end(); ++data_ptr)
        {
            ret.dns_keys.insert(Keyset::DnsKey(data_ptr->get_flags(),
                                               data_ptr->get_protocol(),
                                               data_ptr->get_alg(),
                                               data_ptr->get_key()));
        }
    }
    {
        typedef std::vector<Fred::ObjectIdHandlePair> FredObjectIdHandle;
        for (FredObjectIdHandle::const_iterator data_ptr = data.tech_contacts.begin();
             data_ptr != data.tech_contacts.end(); ++data_ptr)
        {
            ret.tech_contacts.insert(data_ptr->handle);
        }
    }

    return ret;
}

}

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
              "WHERE pea.msgid=").param_bigint(_message_id)(" AND mt.name='update_keyset'");

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    else if (sql_query_result.size() > 1)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    const unsigned long long old_history_id = static_cast<unsigned long long>(sql_query_result[0][0]);
    const unsigned long long new_history_id = static_cast<unsigned long long>(sql_query_result[0][1]);

    PollRequestUpdateKeysetOutputData ret;
    try {
        ret.old_data = get_info_keyset_output_data(_ctx, _registrar_id, old_history_id);
        ret.new_data = get_info_keyset_output_data(_ctx, _registrar_id, new_history_id);
    }
    catch(const Fred::InfoKeysetHistoryByHistoryid::Exception&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }

    return ret;
}

} // namespace Epp::Poll
} // namespace Epp
