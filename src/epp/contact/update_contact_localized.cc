#include "src/epp/contact/update_contact_localized.h"
#include "src/epp/contact/update_contact.h"

#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/impl/post_contact_update_hooks.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/util.h"
#include "util/log/context.h"
#include "util/optional_value.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <string>
#include <set>

namespace Epp {
namespace Contact {

EppResponseSuccessLocalized update_contact_localized(
        const std::string& _contact_handle,
        const ContactChange& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const bool _epp_update_contact_enqueue_check,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix)
{
    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::UpdateContact)));

        if (_data.disclose.is_initialized()) {
            _data.disclose->check_validity();
        }

        Fred::OperationContextCreator ctx;

        const unsigned long long contact_new_history_id =
                update_contact(
                        ctx,
                        _contact_handle,
                        _data,
                        _registrar_id,
                        _logd_request_id);


        post_contact_update_hooks(
                ctx,
                _contact_handle,
                _logd_request_id,
                _epp_update_contact_enqueue_check);

        const EppResponseSuccessLocalized epp_response_success_localized =
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::updated,
                contact_new_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _dont_notify_client_transaction_handles_with_this_prefix);

        return epp_response_success_localized;

    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("update_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("update_contact_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in update_contact_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
