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

#include <boost/format.hpp>

#include <set>
#include <stdexcept>

namespace Epp {

namespace Domain {

DomainCheckResponse domain_check(
    const std::set<std::string>& _domain_fqdns,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
) {
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::DomainCheck)));

        Fred::OperationContextCreator ctx;

        const DomainFqdnToDomainRegistrationObstruction domain_fqdn_to_domain_registration_obstruction = domain_check_impl(ctx, _domain_fqdns, _registrar_id);

        const DomainFqdnToDomainLocalizedRegistrationObstruction domain_fqdn_to_domain_localized_registration_obstruction =
            create_domain_fqdn_to_domain_localized_registration_obstruction(ctx, domain_fqdn_to_domain_registration_obstruction, _lang);
        const LocalizedSuccessResponse localized_success_response =
            create_localized_success_response(Response::ok, ctx, _lang);

        return DomainCheckResponse(localized_success_response, domain_fqdn_to_domain_localized_registration_obstruction);

    }
    catch (const AuthErrorServerClosingConnection&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::failed,
            std::set<Error>(),
            _lang);
    }

}

}

}
