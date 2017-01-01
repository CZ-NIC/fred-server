#include "src/epp/domain/renew_domain_localized.h"
#include "src/epp/domain/renew_domain.h"

#include "src/epp/domain/impl/domain_billing.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/localization.h"
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
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify,
        const bool _rifd_epp_operations_charging)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::RenewDomain)));

        Fred::OperationContextCreator ctx;

        const RenewDomainResult renew_domain_result(
                renew_domain(
                        ctx,
                        _data,
                        _registrar_id,
                        _logd_request_id));

        const RenewDomainLocalizedResponse localized_result(
                create_localized_success_response(
                        ctx,
                        EppResultCode::command_completed_successfully,
                        _lang),
                renew_domain_result.exdate);

        // tmp billing impl
        if(_rifd_epp_operations_charging
                && Fred::InfoRegistrarById(_registrar_id).exec(ctx)
                    .info_registrar_data.system.get_value_or(false) == false)
        {
            renew_domain_bill_item(
                    _data.fqdn,
                    renew_domain_result.curent_time,
                    _registrar_id,
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
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _client_transaction_handles_prefix_not_to_nofify);

        return localized_result;

    }
    catch (const BillingFailure&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::billing_failure)),
                _lang);
    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("renew_domain_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("renew_domain_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in renew_domain_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Domain
} // namespace Epp
