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
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "util/db/param_query_composition.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/epp/nsset/impl/nsset.h"

#include <boost/foreach.hpp>

namespace Epp {
namespace Poll {

namespace {

// could be merged with info_nsset code?
InfoNssetOutputData get_info_nsset_output_data(
    Fred::OperationContext& _ctx,
    unsigned long long _session_registrar_id,
    unsigned long long _history_id)
{
    const Fred::InfoNssetData info_nsset_data = Fred::InfoNssetHistoryByHistoryid(_history_id).exec(_ctx).info_nsset_data;

    std::vector<std::string> tech_contacts;
    tech_contacts.reserve(info_nsset_data.tech_contacts.size());
    BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_contact, info_nsset_data.tech_contacts)
    {
        tech_contacts.push_back(tech_contact.handle);
    }

    const std::vector<Fred::ObjectStateData> object_states_data = Fred::GetObjectStates(info_nsset_data.id).exec(_ctx);
    std::set<std::string> object_states;
    BOOST_FOREACH(const Fred::ObjectStateData& state, object_states_data)
    {
        object_states.insert(state.state_name);
    }

    const std::string callers_registrar_handle = Fred::InfoRegistrarById(_session_registrar_id).exec(_ctx).info_registrar_data.handle;
    const bool callers_is_sponsoring_registrar = info_nsset_data.sponsoring_registrar_handle == callers_registrar_handle;
    const bool authinfopw_has_to_be_hidden = !callers_is_sponsoring_registrar;

    return InfoNssetOutputData(
        info_nsset_data.handle,
        info_nsset_data.roid,
        info_nsset_data.sponsoring_registrar_handle,
        info_nsset_data.create_registrar_handle,
        info_nsset_data.update_registrar_handle,
        object_states,
        info_nsset_data.creation_time,
        info_nsset_data.update_time,
        info_nsset_data.transfer_time,
        authinfopw_has_to_be_hidden ? boost::optional<std::string>() : info_nsset_data.authinfopw,
        Epp::Nsset::make_epp_dnshosts_output(info_nsset_data.dns_hosts),
        tech_contacts,
        info_nsset_data.tech_check_level.get_value_or(0)
    );
}

}

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
              "WHERE pea.msgid=").param_bigint(_message_id)(" AND mt.name='update_nsset'");

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


    try {
        PollRequestUpdateNssetOutputData ret(
            get_info_nsset_output_data(_ctx, _registrar_id, old_history_id),
            get_info_nsset_output_data(_ctx, _registrar_id, new_history_id));
        return ret;
    }
    catch(const Fred::InfoNssetHistoryByHistoryid::Exception&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
}

} // namespace Epp::Poll
} // namespace Epp
