#include "src/epp/nsset/create_nsset_localized.h"
#include "src/epp/nsset/create_nsset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"

#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <string>

namespace Epp {
namespace Nsset {

CreateNssetLocalizedResponse create_nsset_localized(
        const CreateNssetInputData& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>( Action::CreateNsset) ) );

        Fred::OperationContextCreator ctx;

        const CreateNssetResult impl_result(
                create_nsset(
                        ctx,
                        _data,
                        _registrar_id,
                        _logd_request_id));

        const CreateNssetLocalizedResponse localized_result(
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang),
                impl_result.crdate);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::created,
                impl_result.create_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _client_transaction_handles_prefix_not_to_nofify);

        return localized_result;

    }
    catch(const AuthErrorServerClosingConnection& ) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang);
    }
    catch(const ObjectExists&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::object_exist,
                std::set<Error>(),
                _lang);
    }
    catch(const AggregatedParamErrors& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_syntax_error,
                e.get(),
                _lang);
    }
    catch(const ParameterValuePolicyError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_policy_error,
                e.get(),
                _lang);
    }
    catch(const ParameterValueRangeError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_range_error,
                e.get(),
                _lang);
    }
    catch(const RequiredParameterMissing&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_missing,
                std::set<Error>(),
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("create_nsset_localized failure: ") + e.what());
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in create_nsset_localized function");
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }


}

} // namespace Epp::Nsset
} // namespace Epp
