#include "src/epp/contact/transfer_contact_localized.h"
#include "src/epp/contact/transfer_contact.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/localization.h"

#include "util/log/context.h"

namespace Epp {
namespace Contact {

LocalizedSuccessResponse transfer_contact_localized(
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
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

        Fred::OperationContextCreator ctx;

        const unsigned long long post_transfer_history_id =
                transfer_contact(
                        ctx,
                        _contact_handle,
                        _authinfopw,
                        _registrar_id,
                        _logd_request_id);

        const LocalizedSuccessResponse result =
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::transferred,
                post_transfer_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _client_transaction_handles_prefix_not_to_notify);

        return result;

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
    catch(const ObjectNotEligibleForTransfer&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::not_eligible_for_transfer,
                std::set<Error>(),
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
    catch(const AuthorizationInformationError&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authorization_information_error,
                std::set<Error>(),
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("transfer_contact_localized failure: ") + e.what());
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch(...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in transfer_contact_localized function");
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
