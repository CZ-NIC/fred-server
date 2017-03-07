#include "src/epp/contact/create_contact_localized.h"
#include "src/epp/contact/create_contact.h"
#include "src/epp/contact/contact_change.h"

#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
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
        const std::string& _contact_handle,
        const CreateContactInputData& _create_contact_input_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const Optional<unsigned long long>& _logd_request_id)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CreateContact) ) );

        Fred::OperationContextCreator ctx;

        const CreateContactResult create_contact_result(
                create_contact(
                        ctx,
                        _contact_handle,
                        _create_contact_input_data,
                        _session_data.registrar_id,
                        _logd_request_id));

        const CreateContactLocalizedResponse create_contact_localized_response(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                create_contact_result.crdate);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::created,
                create_contact_result.create_history_id,
                _session_data,
                _notification_data);

        return create_contact_localized_response;

    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("create_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("create_contact_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in create_contact_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
