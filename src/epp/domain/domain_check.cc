#include "src/epp/domain/domain_check.h"

#include "src/epp/domain/domain_check_impl.h"
#include "src/epp/domain/domain_check_localization.h"
#include "src/epp/domain/domain_registration_obstruction.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"
#include "src/epp/session_lang.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <set>
#include <stdexcept>

namespace Epp {

namespace Domain {

namespace {

struct AuthentizationInvalid{};

} // namespace Epp::{anonymous}

DomainCheckResponse domain_check(
    const std::set<std::string> &domain_fqdns,
    unsigned long long registrar_id,
    SessionLang::Enum lang,
    const std::string &server_transaction_handle
) {
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % registrar_id));
        Logging::Context logging_ctx3(server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>(Action::DomainCheck)));

        bool authentization_invalid = !registrar_id;
        if (authentization_invalid) {
            throw AuthentizationInvalid();
        }

        Fred::OperationContextCreator ctx;

        const DomainFqdnToDomainRegistrationObstruction domain_fqdn_to_domain_registration_obstruction = domain_check_impl(ctx, domain_fqdns, registrar_id);

        const DomainFqdnToDomainLocalizedRegistrationObstruction domain_fqdn_to_domain_localized_registration_obstruction =
            create_domain_fqdn_to_domain_localized_registration_obstruction(ctx, domain_fqdn_to_domain_registration_obstruction, lang);
        const LocalizedSuccessResponse localized_success_response =
            create_localized_success_response(Response::ok, ctx, lang);

        return DomainCheckResponse(localized_success_response, domain_fqdn_to_domain_localized_registration_obstruction);

    }
    catch (AuthentizationInvalid&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            lang
        );
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::failed,
            std::set<Error>(),
            lang);
    }

}

}

}
