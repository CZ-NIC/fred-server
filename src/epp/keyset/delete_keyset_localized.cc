#include "src/epp/keyset/delete_keyset_localized.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/epp_response_success_localized.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "src/epp/impl/util.h"
#include "src/epp/keyset/delete_keyset.h"
#include "util/log/context.h"

#include <boost/format/free_funcs.hpp>

namespace Epp {
namespace Keyset {

EppResponseSuccessLocalized delete_keyset_localized(
        const std::string& _keyset_handle,
        const SessionData& _session_data,
        const NotificationData& _notification_data)
{
    try
    {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::DeleteKeyset)));

        Fred::OperationContextCreator ctx;

        const unsigned long long last_history_id_before_delete =
                delete_keyset(
                        ctx,
                        _keyset_handle,
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
        exception_localization_ctx.get_log().info(std::string("delete_keyset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(
                std::string("delete_keyset_localized failure: ") +
                e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in delete_keyset_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Keyset
} // namespace Epp
