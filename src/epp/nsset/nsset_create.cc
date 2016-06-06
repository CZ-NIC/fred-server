#include "src/epp/nsset/nsset_create.h"

#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/nsset/nsset_create_impl.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"

#include "util/log/context.h"

namespace Epp {

    LocalizedCreateNssetResponse nsset_create(
        const NssetCreateInputData& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const std::string& _client_transaction_handles_prefix_not_to_nofify
    ) {

    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::NSsetCreate) ) );

        Fred::OperationContextCreator ctx;

        const NssetCreateResult impl_result(
            nsset_create_impl(
                ctx,
                _data,
                _registrar_id,
                _logd_request_id
            )
        );

        const LocalizedCreateNssetResponse localized_result(
            create_localized_success_response(
                Response::ok,
                ctx,
                _lang
            ),
            impl_result.crdate
        );

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::created,
            impl_result.create_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
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

    } catch(const ObjectExists&) {

        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::object_exist,
            std::set<Error>(),
            _lang
        );

    } catch(const AggregatedParamErrors& e) {

        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parametr_error,
            e.get(),
            _lang
        );

    } catch(const ParameterValuePolicyError& e) {

        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parametr_value_policy_error,
            e.get(),
            _lang
        );

    } catch(const RequiredParameterMissing& ) {

        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parametr_missing,
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
