#include "src/epp/contact/contact_transfer.h"

#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/contact/contact_transfer_impl.h"
#include "src/epp/localization.h"

#include "src/fredlib/poll/create_transfer_contact_poll_message.h"

#include "util/log/context.h"

namespace Epp {

LocalizedSuccessResponse contact_transfer(
    const std::string& _contact_handle,
    const std::string& _authinfopw,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const std::string& _client_transaction_handles_prefix_not_to_nofify
) {

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::ContactUpdate)));

        Fred::OperationContextCreator ctx;

        const unsigned long long post_transfer_history_id = contact_transfer_impl(
            ctx,
            _contact_handle,
            _authinfopw,
            _registrar_id,
            _logd_request_id
        );

        const LocalizedSuccessResponse result = create_localized_success_response(Response::ok, ctx, _lang);

        Fred::Poll::CreateTransferContactPollMessage(post_transfer_history_id).exec(ctx);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::transferred,
            post_transfer_history_id,
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

    } catch(const InvalidHandle& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parametr_error,
            Error(Param::contact_handle, 0, Reason::bad_format_contact_handle),
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

    } catch(const ObjectNotEligibleForTransfer& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::not_eligible_for_transfer,
            std::set<Error>(),
            _lang
        );

    } catch(const ObjectStatusProhibitingOperation& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::status_prohibits_operation,
            std::set<Error>(),
            _lang
        );

    } catch(const AutorError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::autor_error,
            std::set<Error>(),
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
