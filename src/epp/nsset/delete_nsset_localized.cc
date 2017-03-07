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

#include "src/epp/nsset/delete_nsset_localized.h"

#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "src/epp/util.h"
#include "src/epp/nsset/delete_nsset.h"
#include "util/log/context.h"

#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Nsset {

EppResponseSuccessLocalized delete_nsset_localized(
        const std::string& _nsset_handle,
        const SessionData& _session_data,
        const NotificationData& _notification_data)
{
    try
    {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::DeleteNsset)));

        Fred::OperationContextCreator ctx;

        const unsigned long long last_history_id_before_delete = delete_nsset(
                ctx,
                _nsset_handle,
                _session_data.registrar_id);

        const EppResponseSuccessLocalized epp_response_success_localized =
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::deleted,
                last_history_id_before_delete,
                _session_data,
                _notification_data);

        return epp_response_success_localized;

    }
    catch (const EppResponseFailure& e)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("delete_nsset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("delete_nsset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in delete_nsset_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Nsset
} // namespace Epp
