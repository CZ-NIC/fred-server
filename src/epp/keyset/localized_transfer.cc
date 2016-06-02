#include "src/epp/keyset/localized_transfer.h"
#include "src/epp/keyset/transfer.h"

#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/localization.h"

#include "src/fredlib/poll/create_transfer_contact_poll_message.h"

#include "util/log/context.h"

namespace Epp {
namespace KeySet {

LocalizedSuccessResponse get_transfer_localized(
    const std::string& _keyset_handle,
    const std::string& _authinfopw,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const std::string &_client_transaction_handles_prefix_not_to_nofify)
{
    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::KeySetTransfer)));

        Fred::OperationContextCreator ctx;

        const unsigned long long post_transfer_history_id = keyset_transfer(
            ctx,
            _keyset_handle,
            _authinfopw,
            _registrar_id,
            _logd_request_id);

        const LocalizedSuccessResponse result = create_localized_success_response(Response::ok, ctx, _lang);

        Fred::Poll::CreateTransferContactPollMessage(post_transfer_history_id).exec(ctx);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::transferred,
            post_transfer_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
            _client_transaction_handles_prefix_not_to_nofify);

        return result;

    }
    catch (const AuthErrorServerClosingConnection&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::authentication_error_server_closing_connection,
            std::set< Error >(),
            _lang);
    }
    catch (const NonexistentHandle&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::object_not_exist,
            std::set< Error >(),
            _lang);
    }
    catch (const ObjectNotEligibleForTransfer&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::not_eligible_for_transfer,
            std::set< Error >(),
            _lang);
    }
    catch (const ObjectStatusProhibitingOperation&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::status_prohibits_operation,
            std::set< Error >(),
            _lang);
    }
    catch (const AutorError&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::autor_error,
            std::set< Error >(),
            _lang);
    }
    catch (const LocalizedFailResponse&) {
        throw;
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::failed,
            std::set< Error >(),
            _lang);
    }
}

}//namespace Epp::KeySet
}//namespace Epp
