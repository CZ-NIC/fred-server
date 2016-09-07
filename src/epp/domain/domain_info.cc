#include "src/epp/domain/domain_info.h"

#include "src/epp/domain/domain_info_impl.h"

#include "src/epp/action.h"
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

DomainInfoResponse domain_info(
    const std::string &domain_fqdn,
    const unsigned long long registrar_id,
    const SessionLang::Enum lang,
    const std::string &server_transaction_handle
) {
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % registrar_id));
        Logging::Context logging_ctx3(server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>(Action::DomainInfo)));

        Fred::OperationContextCreator ctx;

        const DomainInfoOutputData domain_info_output_data = domain_info_impl(ctx, domain_fqdn, registrar_id);

        const LocalizedSuccessResponse localized_success_response =
            create_localized_success_response(Response::ok, ctx, lang);

        return DomainInfoResponse(
            localized_success_response,
            domain_info_output_data
        );
    }
    catch (const AuthErrorServerClosingConnection& e) {
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

