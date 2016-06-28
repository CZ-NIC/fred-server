#include "src/epp/nsset/nsset_delete.h"

#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/nsset/nsset_delete_impl.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/localization.h"
#include "src/epp/action.h"

#include "util/log/context.h"

namespace Epp {

LocalizedSuccessResponse nsset_delete(
    const std::string& _handle,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const std::string& _client_transaction_handles_prefix_not_to_nofify
) {

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::NSsetDelete)));

        Fred::OperationContextCreator ctx;

        const unsigned long long last_history_id_before_delete = nsset_delete_impl(
            ctx,
            _handle,
            _registrar_id
        );

        const LocalizedSuccessResponse result = create_localized_success_response(Response::ok, ctx, _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::deleted,
            last_history_id_before_delete,
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

    } catch(const AutorError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::autor_error,
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

    } catch(const ObjectAssotiationProhibitsOperation& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::prohibits_operation,
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
