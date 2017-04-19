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

#include "src/epp/poll/poll_request_get_update_nsset_details_localized.h"
#include "src/epp/poll/poll_request_get_update_nsset_details.h"
#include "src/epp/nsset/info_nsset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/epp/impl/action.h"
#include "src/epp/localization.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_response_success_localized.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <stdexcept>

namespace Epp {
namespace Poll {

namespace {

// a bunch of workarounds -- taken from info_nsset_localized.cc
// TODO udelat pres ziskani externich stavu, zatim na to neni ve fredlibu rozhrani
// TODO Fred::GetObjectStates pro "handle"
void filter_states(
    Fred::OperationContext& _ctx,
    Epp::Nsset::InfoNssetOutputData& _output_data)
{
    std::set<Epp::Nsset::StatusValue::Enum> filtered_states;

    const std::vector<Fred::ObjectStateData> state_definitions =
        Fred::GetObjectStates(
            Fred::InfoNssetByHandle(_output_data.handle).exec(_ctx).info_nsset_data.id).exec(_ctx);

    for(std::set<Epp::Nsset::StatusValue::Enum>::const_iterator state_it = _output_data.states.begin();
        state_it != _output_data.states.end();
        ++state_it)
    {
        for(std::vector<Fred::ObjectStateData>::const_iterator state_def_it = state_definitions.begin();
            state_def_it != state_definitions.end();
            ++state_def_it)
        {
            if (state_def_it->is_external)
            {
                filtered_states.insert(*state_it);
            }
        }
    }
    _output_data.states = filtered_states;
}

} // namespace Epp::Poll::{anonymous}

PollRequestUpdateNssetLocalizedResponse poll_request_get_update_nsset_details_localized(
    unsigned long long _message_id,
    const SessionData& _session_data)
{
    Logging::Context logging_ctx("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::PollResponse)));

    try {
        Fred::OperationContextCreator ctx;

        PollRequestUpdateNssetOutputData output_data =
            poll_request_get_update_nsset_details(ctx, _message_id, _session_data.registrar_id);

        filter_states(ctx, output_data.old_data);
        filter_states(ctx, output_data.new_data);

        const PollRequestUpdateNssetLocalizedOutputData localized_output_data =
            PollRequestUpdateNssetLocalizedOutputData(
                Epp::Nsset::InfoNssetLocalizedOutputData(
                    output_data.old_data.handle,
                    output_data.old_data.roid,
                    output_data.old_data.sponsoring_registrar_handle,
                    output_data.old_data.creating_registrar_handle,
                    output_data.old_data.last_update_registrar_handle,
                    localize_object_states<Epp::Nsset::StatusValue>(ctx, output_data.old_data.states, _session_data.lang),
                    output_data.old_data.crdate,
                    output_data.old_data.last_update,
                    output_data.old_data.last_transfer,
                    output_data.old_data.authinfopw,
                    output_data.old_data.dns_hosts,
                    output_data.old_data.tech_contacts,
                    output_data.old_data.tech_check_level),
                Epp::Nsset::InfoNssetLocalizedOutputData(
                    output_data.new_data.handle,
                    output_data.new_data.roid,
                    output_data.new_data.sponsoring_registrar_handle,
                    output_data.new_data.creating_registrar_handle,
                    output_data.new_data.last_update_registrar_handle,
                    localize_object_states<Epp::Nsset::StatusValue>(ctx, output_data.new_data.states, _session_data.lang),
                    output_data.new_data.crdate,
                    output_data.new_data.last_update,
                    output_data.new_data.last_transfer,
                    output_data.new_data.authinfopw,
                    output_data.new_data.dns_hosts,
                    output_data.new_data.tech_contacts,
                    output_data.new_data.tech_check_level));

        return PollRequestUpdateNssetLocalizedResponse(
            EppResponseSuccessLocalized(
                ctx,
                EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                _session_data.lang),
            localized_output_data);
    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_nsset_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            e,
            _session_data.lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_nsset_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("unexpected exception in poll_request_nsset_update_details_localized function"));
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
}

} // namespace Epp::Poll
} // namespace Epp
