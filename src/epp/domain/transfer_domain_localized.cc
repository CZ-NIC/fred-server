#include "src/epp/domain/transfer_domain_localized.h"

#include "src/epp/domain/transfer_domain.h"
#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_response_success_localized.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "src/epp/localization.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "src/epp/util.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Domain {

EppResponseSuccessLocalized transfer_domain_localized(
        const std::string& _domain_fqdn,
        const std::string& _authinfopw,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const Optional<unsigned long long>& _logd_request_id)
{
    try
    {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::UpdateDomain)));

        Fred::OperationContextCreator ctx;

        const unsigned long long post_transfer_history_id =
                transfer_domain(
                        ctx,
                        _domain_fqdn,
                        _authinfopw,
                        _session_data.registrar_id,
                        _logd_request_id);

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
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("transfer_domain_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("transfer_domain_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in transfer_domain_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Domain
} // namespace Epp
