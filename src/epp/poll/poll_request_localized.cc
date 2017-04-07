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

#include "src/epp/poll/poll_request_localized.h"

#include "src/epp/impl/action.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_success.h"
#include "src/epp/epp_result_failure.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <stdexcept>
#include <string>

namespace Epp {
namespace Poll {

PollRequestLocalizedResponse poll_request_localized(
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::PollResponse)));//?

        Fred::OperationContextCreator ctx;

        const PollRequestOutputData output_data = poll_request(ctx, _registrar_id);

        const PollRequestLocalizedOutputData localized_output_data =
            PollRequestLocalizedOutputData(
                output_data.message_id,
                output_data.creation_time,
                output_data.number_of_unseen_messages,
                output_data.message);

        PollRequestLocalizedResponse ret(
            EppResponseSuccessLocalized(
                ctx,
                EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully_ack_to_dequeue)),
                _lang),
            localized_output_data);

        ctx.commit_transaction();
        return ret;
    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            e,
            _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _lang);
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("unexpected exception in poll_request_localized function"));
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _lang);
    }
}

} // namespace Epp::Poll
} // namespace Epp
