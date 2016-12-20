#include "src/epp/domain/update_domain_localized.h"
#include "src/epp/domain/update_domain.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "util/db/nullable.h"
#include "util/log/context.h"
#include "util/optional_value.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <string>

namespace Epp {
namespace Domain {

LocalizedSuccessResponse update_domain_localized(
        const std::string& _domain_fqdn,
        const Optional<std::string>& _registrant_chg,
        const Optional<std::string>& _auth_info_pw_chg,
        const Optional<Nullable<std::string> >& _nsset_chg,
        const Optional<Nullable<std::string> >& _keyset_chg,
        const std::vector<std::string>& _admin_contacts_add,
        const std::vector<std::string>& _admin_contacts_rem,
        const std::vector<std::string>& _tmpcontacts_rem,
        const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const bool _epp_update_domain_enqueue_check,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix,
        const bool _rifd_epp_update_domain_keyset_clear)
{

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::UpdateDomain)));

        Fred::OperationContextCreator ctx;

        const unsigned long long domain_new_history_id =
                update_domain(
                        ctx,
                        _domain_fqdn,
                        _registrant_chg,
                        _auth_info_pw_chg,
                        _nsset_chg,
                        _keyset_chg,
                        _admin_contacts_add,
                        _admin_contacts_rem,
                        _tmpcontacts_rem,
                        _enum_validation_list,
                        _registrar_id,
                        _logd_request_id,
                        _rifd_epp_update_domain_keyset_clear);

        const LocalizedSuccessResponse localized_success_response =
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::updated,
                domain_new_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _dont_notify_client_transaction_handles_with_this_prefix);

        return localized_success_response;

    }
    catch (const AuthErrorServerClosingConnection&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang);
    }
    catch (const NonexistentHandle&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::object_not_exist,
                std::set<Error>(),
                _lang);
    }
    catch (const AuthorizationError&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authorization_error,
                Error::of_scalar_parameter(Param::registrar_autor, Reason::unauthorized_registrar),
                _lang);
    }
    catch (const ObjectStatusProhibitsOperation&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::status_prohibits_operation,
                std::set<Error>(),
                _lang);
    }
    catch (const ParameterValuePolicyError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_policy_error,
                e.get(),
                _lang);
    }
    catch (const ParameterValueRangeError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_range_error,
                e.get(),
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("update_domain_localized failure: ") + e.what());
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
}

} // namespace Epp::Domain
} // namespace Epp
