#include "src/epp/contact/contact_update.h"

#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/contact/contact_update_impl.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/impl/util.h"
#include "src/epp/localization.h"

#include "src/epp/contact/post_contact_update_hooks.h"

#include "util/log/context.h"

#include <string>

namespace Epp {

LocalizedSuccessResponse contact_update(
    const std::string &_contact_handle,
    const ContactUpdateInputData &_data,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    bool _epp_update_contact_enqueue_check,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const std::string &_client_transaction_handles_prefix_not_to_nofify)
{

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::ContactUpdate)));

        Fred::OperationContextCreator ctx;

        const unsigned long long new_history_id = contact_update_impl(
            ctx,
            _contact_handle,
            _data,
            _registrar_id,
            _logd_request_id
        );


        post_contact_update_hooks(
            ctx,
            _contact_handle,
            _logd_request_id,
            _epp_update_contact_enqueue_check
        );

        const LocalizedSuccessResponse result = create_localized_success_response(Response::ok, ctx, _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::updated,
            new_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
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
            std::set<Error>(),
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

    } catch(const SsnWithoutSsnType& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_missing,
            std::set<Error>(),
            _lang
        );

    } catch(const SsnTypeWithoutSsn& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_missing,
            std::set<Error>(),
            _lang
        );

    } catch(const AggregatedParamErrors& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_value_policy_error,
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

}
