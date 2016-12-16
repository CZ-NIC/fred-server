#include "src/epp/contact/update_contact_localized.h"
#include "src/epp/contact/update_contact.h"

#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/impl/post_contact_update_hooks.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/util.h"

#include "util/log/context.h"

#include <string>

namespace Epp {
namespace Contact {

LocalizedSuccessResponse update_contact_localized(
        const std::string& _contact_handle,
        const ContactChange& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const bool _epp_update_contact_enqueue_check,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_notify)
{
    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::UpdateContact)));

        if (_data.disclose.is_initialized()) {
            _data.disclose->check_validity();
        }

        Fred::OperationContextCreator ctx;

        const unsigned long long new_history_id =
                update_contact(
                        ctx,
                        _contact_handle,
                        _data,
                        _registrar_id,
                        _logd_request_id);


        post_contact_update_hooks(
                ctx,
                _contact_handle,
                _logd_request_id,
                _epp_update_contact_enqueue_check);

        const LocalizedSuccessResponse localized_result =
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::updated,
                new_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _client_transaction_handles_prefix_not_to_notify);

        return localized_result;

    }
    catch(const AuthErrorServerClosingConnection&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang);
    }
    catch(const NonexistentHandle&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::object_not_exist,
                std::set<Error>(),
                _lang);
    }
    catch(const AuthorizationError&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authorization_error,
                Error::of_scalar_parameter(Param::registrar_autor, Reason::unauthorized_registrar),
                _lang);
    }
    catch(const ObjectStatusProhibitsOperation&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::status_prohibits_operation,
                std::set<Error>(),
                _lang);
    }
    catch(const SsnWithoutSsnType&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_missing,
                std::set<Error>(),
                _lang);
    }
    catch(const SsnTypeWithoutSsn&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_missing,
                std::set<Error>(),
                _lang);
    }
    catch(const AggregatedParamErrors& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_policy_error,
                e.get(),
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("update_contact_localized failure: ") + e.what());
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch(...) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
