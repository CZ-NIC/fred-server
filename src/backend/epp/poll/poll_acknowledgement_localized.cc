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

#include "src/backend/epp/poll/poll_acknowledgement_localized.hh"
#include "src/backend/epp/poll/poll_acknowledgement.hh"

#include "src/backend/epp/impl/action.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "libfred/opcontext.hh"
#include "util/log/context.hh"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <string>
#include <stdexcept>

namespace Epp {
namespace Poll {

PollAcknowledgementLocalizedResponse poll_acknowledgement_localized(
    const std::string& _message_id,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle)
{
    Logging::Context logging_ctx("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
    Logging::Context logging_ctx3(_server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::PollAcknowledgement)));

    try {
        LibFred::OperationContextCreator ctx;

        unsigned long long message_id;
        try {
            message_id = boost::lexical_cast<unsigned long long>(_message_id);
        }
        catch (const boost::bad_lexical_cast&) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        const PollAcknowledgementOutputData output_data = poll_acknowledgement(ctx, message_id, _registrar_id);

        std::string oldest_unseen_message_id;
        if (output_data.number_of_unseen_messages > 0)
        {
            oldest_unseen_message_id = boost::lexical_cast<std::string>(output_data.oldest_unseen_message_id);
        }

        const PollAcknowledgementLocalizedOutputData localized_output_data =
            PollAcknowledgementLocalizedOutputData(
                output_data.number_of_unseen_messages,
                oldest_unseen_message_id);

        const PollAcknowledgementLocalizedResponse ret(
            EppResponseSuccessLocalized(
                ctx,
                EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                _lang),
            localized_output_data);

        ctx.commit_transaction();
        return ret;
    }
    catch (const EppResponseFailure& e) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_acknowledgement_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            e,
            _lang);
    }
    catch (const std::exception& e) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_acknowledgement_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _lang);
    }
    catch (...) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("unexpected exception in poll_acknowledgement_localized function"));
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _lang);
    }
}

} // namespace Epp::Poll
} // namespace Epp
