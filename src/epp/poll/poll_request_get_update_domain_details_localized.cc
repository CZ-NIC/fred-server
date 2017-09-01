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

#include "src/epp/poll/poll_request_get_update_domain_details_localized.h"
#include "src/epp/poll/poll_request_get_update_domain_details.h"
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

PollRequestUpdateDomainLocalizedResponse poll_request_get_update_domain_details_localized(
    unsigned long long _message_id,
    const SessionData& _session_data)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::PollResponse)));

        Fred::OperationContextCreator ctx;

        const PollRequestUpdateDomainOutputData output_data =
            poll_request_get_update_domain_details(ctx, _message_id, _session_data.registrar_id);

        const PollRequestUpdateDomainLocalizedOutputData localized_output_data =
            PollRequestUpdateDomainLocalizedOutputData(
                Epp::Domain::InfoDomainLocalizedOutputData(
                    output_data.old_data.roid,
                    output_data.old_data.fqdn,
                    output_data.old_data.registrant,
                    output_data.old_data.nsset,
                    output_data.old_data.keyset,
                    localize_object_states<Epp::Domain::StatusValue>(ctx, output_data.old_data.states, _session_data.lang),
                    output_data.old_data.sponsoring_registrar_handle,
                    output_data.old_data.creating_registrar_handle,
                    output_data.old_data.last_update_registrar_handle,
                    output_data.old_data.crdate,
                    output_data.old_data.last_update,
                    output_data.old_data.last_transfer,
                    output_data.old_data.exdate,
                    output_data.old_data.authinfopw,
                    output_data.old_data.admin,
                    output_data.old_data.ext_enum_domain_validation,
                    output_data.old_data.tmpcontact),
                Epp::Domain::InfoDomainLocalizedOutputData(
                    output_data.new_data.roid,
                    output_data.new_data.fqdn,
                    output_data.new_data.registrant,
                    output_data.new_data.nsset,
                    output_data.new_data.keyset,
                    localize_object_states<Epp::Domain::StatusValue>(ctx, output_data.new_data.states, _session_data.lang),
                    output_data.new_data.sponsoring_registrar_handle,
                    output_data.new_data.creating_registrar_handle,
                    output_data.new_data.last_update_registrar_handle,
                    output_data.new_data.crdate,
                    output_data.new_data.last_update,
                    output_data.new_data.last_transfer,
                    output_data.new_data.exdate,
                    output_data.new_data.authinfopw,
                    output_data.new_data.admin,
                    output_data.new_data.ext_enum_domain_validation,
                    output_data.new_data.tmpcontact));

        return PollRequestUpdateDomainLocalizedResponse(
            EppResponseSuccessLocalized(
                ctx,
                EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                _session_data.lang),
            localized_output_data);
    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_domain_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            e,
            _session_data.lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_domain_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("unexpected exception in poll_request_domain_update_details_localized function"));
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
}

} // namespace Epp::Poll
} // namespace Epp
