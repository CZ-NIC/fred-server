#include "src/epp/contact/delete_contact_localized.h"
#include "src/epp/contact/delete_contact.h"

#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/action.h"

#include "util/log/context.h"

#include <boost/format/free_funcs.hpp>

namespace Epp {
namespace Contact {

LocalizedSuccessResponse delete_contact_localized(
        const std::string& _contact_handle,
        const unsigned long long _registrar_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix)
{
    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::DeleteContact)));

        Fred::OperationContextCreator ctx;

        const unsigned long long last_history_id_before_delete =
                delete_contact(
                        ctx,
                        _contact_handle,
                        _registrar_id);

        const LocalizedSuccessResponse result =
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::deleted,
                last_history_id_before_delete,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _dont_notify_client_transaction_handles_with_this_prefix);

        return result;

    }
    //catch (const AuthErrorServerClosingConnection&) {
    //    Fred::OperationContextCreator exception_localization_ctx;
    //    throw create_localized_fail_response(
    //            exception_localization_ctx,
    //            Response::authentication_error_server_closing_connection,
    //            std::set<Error>(),
    //            _lang);
    //}
    //catch (const NonexistentHandle&) {
    //    Fred::OperationContextCreator exception_localization_ctx;
    //    throw create_localized_fail_response(
    //            exception_localization_ctx,
    //            Response::object_not_exist,
    //            std::set<Error>(),
    //            _lang);
    //}
    //catch (const AuthorizationError&) {
    //    Fred::OperationContextCreator exception_localization_ctx;
    //    throw create_localized_fail_response(
    //            exception_localization_ctx,
    //            Response::authorization_error,
    //            Error::of_scalar_parameter(Param::registrar_autor, Reason::unauthorized_registrar),
    //            _lang);
    //}
    //catch (const ObjectStatusProhibitsOperation&) {
    //    Fred::OperationContextCreator exception_localization_ctx;
    //    throw create_localized_fail_response(
    //            exception_localization_ctx,
    //            Response::status_prohibits_operation,
    //            std::set<Error>(),
    //            _lang);
    //}
    //catch (const ObjectAssociationProhibitsOperation&) {
    //    Fred::OperationContextCreator exception_localization_ctx;
    //    throw create_localized_fail_response(
    //            exception_localization_ctx,
    //            Response::object_association_prohibits_operation,
    //            std::set<Error>(),
    //            _lang);
    //}
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("delete_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("delete_contact_localized failure: ") + e.what());
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in delete_contact_localized function");
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
