#include "src/epp/domain/renew_domain_localized.h"
#include "src/epp/domain/renew_domain.h"

#include "src/epp/domain/impl/domain_billing.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/notification_data.h"
#include "src/epp/impl/session_data.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/decimal/decimal.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Domain {

RenewDomainLocalizedResponse renew_domain_localized(
        const RenewDomainInputData& _data,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const Optional<unsigned long long>& _logd_request_id,
        const bool _rifd_epp_operations_charging)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::RenewDomain)));

        Fred::OperationContextCreator ctx;

        const RenewDomainResult renew_domain_result(
                renew_domain(
                        ctx,
                        _data,
                        _session_data.registrar_id,
                        _logd_request_id));

        const RenewDomainLocalizedResponse renew_domain_localized_response(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                renew_domain_result.exdate);

        // tmp billing impl
        if (_rifd_epp_operations_charging
                && Fred::InfoRegistrarById(_session_data.registrar_id).exec(ctx)
                    .info_registrar_data.system.get_value_or(false) == false)
        {
            renew_domain_bill_item(
                    _data.fqdn,
                    renew_domain_result.curent_time,
                    _session_data.registrar_id,
                    renew_domain_result.domain_id,
                    renew_domain_result.length_of_domain_registration_in_years,
                    renew_domain_result.old_exdate,
                    renew_domain_result.exdate,
                    ctx);
        }

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::renewed,
                renew_domain_result.domain_history_id,
                _session_data,
                _notification_data);

        return renew_domain_localized_response;

    }
    catch (const BillingFailure&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::billing_failure)),
                _session_data.lang);
    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("renew_domain_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("renew_domain_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in renew_domain_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}

} // namespace Epp::Domain
} // namespace Epp
