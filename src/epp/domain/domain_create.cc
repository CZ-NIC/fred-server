#include "src/fredlib/registrar/info_registrar.h"
#include "src/epp/domain/domain_create.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/domain/domain_create_impl.h"
#include "src/epp/domain/domain_billing.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"

#include "util/log/context.h"
#include "util/decimal/decimal.h"

namespace Epp {

    LocalizedCreateDomainResponse domain_create(
        const DomainCreateInputData& _data,
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
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::DomainCreate) ) );

        Fred::OperationContextCreator ctx;

        const DomainCreateResult impl_result(
            domain_create_impl(
                ctx,
                _data,
                _registrar_id,
                _logd_request_id
            )
        );

        const LocalizedCreateDomainResponse localized_result(
            create_localized_success_response(
                ctx,
                Response::ok,
                _lang
            ),
            impl_result.crtime,
            impl_result.exdate
        );

        if(_rifd_epp_operations_charging
                && Fred::InfoRegistrarById(_registrar_id).exec(ctx)
                    .info_registrar_data.system.get_value_or(false) == false)
        {
            create_domain_bill_item(
                _data.fqdn,
                impl_result.crtime,
                _registrar_id,
                impl_result.id,
                ctx);

            renew_domain_bill_item(
                _data.fqdn,
                impl_result.crtime,
                _registrar_id,
                impl_result.id,
                impl_result.length_of_domain_registration_in_years,
                impl_result.old_exdate,
                impl_result.exdate,
                ctx);
        }

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::created,
            impl_result.create_history_id,
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
    } catch(const AuthorizationError&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authorization_error,
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
    } catch(const ParameterValueRangeError& e) {
            Fred::OperationContextCreator exception_localization_ctx;
            throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_range_error,
                e.get(),
                _lang
            );
    } catch(const RequiredParameterMissing& ) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_missing,
            std::set<Error>(),
            _lang
        );
    } catch(const ParameterValueSyntaxError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_value_syntax_error,
            e.get(),
            _lang
        );
    } catch(const ObjectExists&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::object_exist,
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
