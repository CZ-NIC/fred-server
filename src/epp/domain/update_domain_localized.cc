#include "src/epp/domain/update_domain_localized.h"
#include "src/epp/domain/update_domain.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "util/db/nullable.h"
#include "util/log/context.h"
#include "util/optional_value.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

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
        const std::vector<EnumValidationExtension>& _enum_validation_list,
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
                        EppResultCode::command_completed_successfully,
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
    catch (const EppResponseFailure& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("update_domain_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("update_domain_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in update_domain_localized function");
        throw EppResponseFailureLocalized(
                exception_localization_ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Domain
} // namespace Epp
