#include "src/epp/keyset/create_keyset_localized.h"
#include "src/epp/keyset/create_keyset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <string>

namespace Epp {
namespace Keyset {

ResponseOfCreate create_keyset_localized(
        const std::string& _keyset_handle,
        const Optional<std::string>& _auth_info_pw,
        const std::vector<std::string>& _tech_contacts,
        const std::vector<Keyset::DsRecord>& _ds_records,
        const std::vector<Keyset::DnsKey>& _dns_keys,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CreateKeyset)));

        Fred::OperationContextCreator ctx;

        const CreateKeysetResult result =
            create_keyset(
                    ctx, _keyset_handle,
                    _auth_info_pw,
                    _tech_contacts,
                    _ds_records,
                    _dns_keys,
                    _registrar_id,
                    _logd_request_id);

        const ResponseOfCreate localized_result(
                create_localized_success_response(
                        ctx,
                        EppResultCode::command_completed_successfully,
                        _lang),
                result.crdate);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::created,
                result.create_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _dont_notify_client_transaction_handles_with_this_prefix);

        return localized_result;

    }
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("create_keyset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("create_keyset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in create_keyset_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Keyset
} // namespace Epp
