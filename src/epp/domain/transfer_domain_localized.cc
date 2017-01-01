#include "src/epp/domain/transfer_domain_localized.h"
#include "src/epp/domain/transfer_domain.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/poll/create_transfer_domain_poll_message.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Domain {

LocalizedSuccessResponse transfer_domain_localized(
        const std::string& _domain_fqdn,
        const std::string& _authinfopw,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
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
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::UpdateDomain)));

        Fred::OperationContextCreator ctx;

        const unsigned long long post_transfer_history_id =
                transfer_domain(
                        ctx,
                        _domain_fqdn,
                        _authinfopw,
                        _registrar_id,
                        _logd_request_id);

        const LocalizedSuccessResponse result =
                create_localized_success_response(
                        ctx,
                        EppResultCode::command_completed_successfully,
                        _lang);

        Fred::Poll::CreateTransferDomainPollMessage(post_transfer_history_id).exec(ctx);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::transferred,
                post_transfer_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _dont_notify_client_transaction_handles_with_this_prefix);

        return result;

    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("transfer_domain_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("transfer_domain_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in transfer_domain_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Domain
} // namespace Epp
