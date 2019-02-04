/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "src/backend/epp/keyset/transfer_keyset_localized.hh"

#include "src/backend/epp/impl/action.hh"
#include "src/backend/epp/impl/conditionally_enqueue_notification.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/localization.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/keyset/transfer_keyset.hh"
#include "util/log/context.hh"

namespace Epp {
namespace Keyset {

EppResponseSuccessLocalized transfer_keyset_localized(
        const std::string& _keyset_handle,
        const std::string& _authinfopw,
        const TransferKeysetConfigData& _transfer_keyset_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data)
{
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::TransferKeyset)));

    try
    {
        LibFred::OperationContextCreator ctx;

        const unsigned long long post_transfer_history_id =
                transfer_keyset(
                        ctx,
                        _keyset_handle,
                        _authinfopw,
                        _transfer_keyset_config_data,
                        _session_data);

        const EppResponseSuccessLocalized epp_response_success_localized =
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::transferred,
                post_transfer_history_id,
                _session_data,
                _notification_data);

        return epp_response_success_localized;

    }
    catch (const EppResponseFailure& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("transfer_keyset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("transfer_keyset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in transfer_keyset_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Keyset
} // namespace Epp
