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

#include "src/epp/impl/action.h"

#include "src/epp/credit/client_credit_localized.h"
#include "src/epp/credit/client_credit.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_response_success_localized.h"

#include "util/log/context.h"

namespace Epp {
namespace Credit {

ClientCreditLocalizedResponse client_credit_localized(const SessionData& _session_data)
{
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::ClientCredit)));

    try
    {
        Fred::OperationContextCreator ctx;

        const ClientCreditOutputData client_credit_output_data =
                client_credit(ctx, _session_data.registrar_id);

        return ClientCreditLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                client_credit_output_data);
    }
    catch (const EppResponseFailure& e)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("client_credit_localized: ") + e.what());
        throw EppResponseFailureLocalized(ctx, e, _session_data.lang);
    }
    catch (const std::exception& e)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("client_credit_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in client_credit_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}

}//namespace Epp::Credit
}//namespace Epp
