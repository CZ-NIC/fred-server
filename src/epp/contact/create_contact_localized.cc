/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/contact/create_contact_localized.h"
#include "src/epp/contact/create_contact.h"
#include "src/epp/contact/contact_change.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_response_success_localized.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

CreateContactLocalizedResponse create_contact_localized(
        const std::string& contact_handle,
        const CreateContactInputData& create_contact_input_data,
        const CreateContactConfigData& create_contact_config_data,
        const SessionData& session_data,
        const NotificationData& notification_data)
{
    Logging::Context logging_ctx("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % session_data.registrar_id));
    Logging::Context logging_ctx3(session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CreateContact) ) );

    try
    {
        Fred::OperationContextCreator ctx;

        const CreateContactResult result(
                create_contact(
                        ctx,
                        contact_handle,
                        create_contact_input_data,
                        create_contact_config_data,
                        session_data));

        const CreateContactLocalizedResponse response(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        session_data.lang),
                result.crdate);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::created,
                result.create_history_id,
                session_data,
                notification_data);

        return response;
    }
    catch (const EppResponseFailure& e)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("create_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                session_data.lang);
    }
    catch (const std::exception& e)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("create_contact_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                session_data.lang);
    }
    catch (...)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in create_contact_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                session_data.lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
