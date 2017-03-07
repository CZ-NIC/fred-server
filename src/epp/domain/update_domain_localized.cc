#include "src/epp/domain/update_domain_localized.h"

#include "src/epp/domain/update_domain_input_data.h"
#include "src/epp/domain/update_domain.h"
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
#include "src/fredlib/domain/enum_validation_extension.h"
#include "util/db/nullable.h"
#include "util/log/context.h"
#include "util/optional_value.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace Epp {
namespace Domain {

EppResponseSuccessLocalized update_domain_localized(
        const UpdateDomainInputData& _update_domain_input_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const Optional<unsigned long long>& _logd_request_id,
        const bool _rifd_epp_update_domain_keyset_clear)
{

    try
    {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::UpdateDomain)));

        Fred::OperationContextCreator ctx;

        const unsigned long long domain_new_history_id =
                update_domain(
                        ctx,
                        _update_domain_input_data,
                        _session_data.registrar_id,
                        _logd_request_id,
                        _rifd_epp_update_domain_keyset_clear);

        const EppResponseSuccessLocalized epp_response_success_localized =
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::updated,
                domain_new_history_id,
                _session_data,
                _notification_data);

        return epp_response_success_localized;

    }
    catch (const EppResponseFailure& e)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("update_domain_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(
                std::string("update_domain_localized failure: ") +
                e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in update_domain_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Domain
} // namespace Epp
