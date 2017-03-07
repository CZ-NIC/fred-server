#include "src/fredlib/registrar/info_registrar.h"
#include "src/epp/domain/domain_renew.h"
#include "src/epp/domain/domain_billing.h"
#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/domain/domain_renew_impl.h"
#include "src/epp/domain/domain_billing.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"

#include "util/log/context.h"
#include "util/decimal/decimal.h"

namespace Epp {

    LocalizedRenewDomainResponse domain_renew(
        const DomainRenewInputData& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify,
        const bool _rifd_epp_operations_charging
    ) {

    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::DomainRenew) ) );

        Fred::OperationContextCreator ctx;

        const DomainRenewResult impl_result(
            domain_renew_impl(
                ctx,
                _data,
                _registrar_id,
                _logd_request_id
            )
        );

        const LocalizedRenewDomainResponse localized_result(
            create_localized_success_response(
                Response::ok,
                ctx,
                _lang
            ),
            impl_result.exdate
        );

        //tmp billing impl
        if(_rifd_epp_operations_charging
                && Fred::InfoRegistrarById(_registrar_id).exec(ctx)
                    .info_registrar_data.system.get_value_or(false) == false)
        {
            renew_domain_bill_item(
                _data.fqdn,
                impl_result.curent_time,
                _registrar_id,
                impl_result.domain_id,
                impl_result.length_of_domain_registration_in_years,
                impl_result.exdate,
                ctx);
        }

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::renewed,
            impl_result.domain_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
            _epp_notification_disabled,
            _client_transaction_handles_prefix_not_to_nofify
        );

        return localized_result;

    } catch(const AuthErrorServerClosingConnection& ) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );
    } catch(const ParameterValuePolicyError& e) {
            Fred::OperationContextCreator exception_localization_ctx;
            throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_policy_error,
                e.get(),
                _lang
            );
    } catch(const ZoneAuthorizationError&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authorization_error,
            std::set<Error>(),
            _lang
        );
    } catch(const AuthorizationError&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authorization_error,
            Error::of_scalar_parameter(Param::registrar_autor, Reason::unauthorized_registrar),
            _lang
        );
    } catch(const ObjectDoesNotExist& ) {
            Fred::OperationContextCreator exception_localization_ctx;
            throw create_localized_fail_response(
                exception_localization_ctx,
                Response::object_not_exist,
                std::set<Error>(),
                _lang
            );
    } catch(const ParameterValueRangeError& e) {
            Fred::OperationContextCreator exception_localization_ctx;
            throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_range_error,
                e.get(),
                _lang
            );
    } catch(const ObjectStatusProhibitsOperation&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::status_prohibits_operation,
            std::set<Error>(),
            _lang
        );
    } catch(const BillingFailure&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::billing_failure,
            std::set<Error>(),
            _lang
        );
    } catch(const LocalizedFailResponse& ) {
        throw;
    } catch(...) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::failed,
            std::set<Error>(),
            _lang
        );
    }

}

}