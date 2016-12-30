#include "src/epp/keyset/delete_keyset_localized.h"
#include "src/epp/keyset/delete_keyset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/parameter_errors.h"
#include "src/epp/impl/util.h"
#include "util/log/context.h"

#include <boost/format/free_funcs.hpp>

namespace Epp {
namespace Keyset {

LocalizedSuccessResponse delete_keyset_localized(
        const std::string& _keyset_handle,
        const unsigned long long _registrar_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify)
{
    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::DeleteKeyset)));

        Fred::OperationContextCreator ctx;

        const unsigned long long last_history_id_before_delete = delete_keyset(
                ctx,
                _keyset_handle,
                _registrar_id);

        const LocalizedSuccessResponse result = create_localized_success_response(
                ctx,
                Response::ok,
                _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::deleted,
                last_history_id_before_delete,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _client_transaction_handles_prefix_not_to_nofify);

        return result;

    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("delete_keyset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("delete_keyset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in delete_keyset_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Keyset
} // namespace Epp
