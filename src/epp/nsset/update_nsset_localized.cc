#include "src/epp/nsset/update_nsset_localized.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/nsset/update_nsset.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/util.h"
#include "src/epp/impl/localization.h"

#include "util/log/context.h"

#include <string>

namespace Epp {
namespace Nsset {

LocalizedSuccessResponse update_nsset_localized(
    const UpdateNssetInputData& _data,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    const bool _epp_update_nsset_enqueue_check,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string& _client_transaction_handles_prefix_not_to_nofify
) {

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::UpdateNsset)));

        Fred::OperationContextCreator ctx;

        const unsigned long long new_history_id = update_nsset(
            ctx,
            _data,
            _registrar_id,
            _logd_request_id
        );

        const LocalizedSuccessResponse result = create_localized_success_response(ctx, Response::ok, _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::updated,
            new_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
            _epp_notification_disabled,
            _client_transaction_handles_prefix_not_to_nofify
        );

        return result;

    } catch(const AuthErrorServerClosingConnection& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );

    } catch(const NonexistentHandle& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::object_not_exist,
            std::set<Error>(),
            _lang
        );

    } catch(const AuthorizationError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authorization_error,
            Util::set_of<Error>(Error::of_scalar_parameter(Param::registrar_autor, Reason::unauthorized_registrar)),
            _lang
        );

    } catch(const ObjectStatusProhibitsOperation& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::status_prohibits_operation,
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

    } catch(const ParameterValueRangeError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_value_range_error,
            e.get(),
            _lang
        );

    } catch(const LocalizedFailResponse&) {
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

} // namespace Epp::Nsset
} // namespace Epp
