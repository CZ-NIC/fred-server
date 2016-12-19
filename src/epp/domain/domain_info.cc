#include "src/epp/domain/domain_info.h"

#include "src/epp/impl/action.h"
#include "src/epp/domain/domain_info_impl.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/object_states_localized.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/log/context.h"

#include <boost/format.hpp>

#include <set>
#include <stdexcept>

namespace Epp {

namespace Domain {

DomainInfoResponse domain_info(
    const std::string& _domain_fqdn,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
) {
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::DomainInfo)));

        Fred::OperationContextCreator ctx;

        const DomainInfoOutputData domain_info_output_data = domain_info_impl(ctx, _domain_fqdn, _registrar_id);

        const std::string callers_registrar_handle = Fred::InfoRegistrarById(_registrar_id).exec(ctx).info_registrar_data.handle;
        const bool callers_is_sponsoring_registrar = domain_info_output_data.sponsoring_registrar_handle == callers_registrar_handle;
        const bool authinfo_has_to_be_hidden = !callers_is_sponsoring_registrar;

        DomainInfoLocalizedOutputData localized_domain_info_output_data;

        localized_domain_info_output_data.roid = domain_info_output_data.roid;
        localized_domain_info_output_data.fqdn = domain_info_output_data.fqdn;
        localized_domain_info_output_data.registrant = domain_info_output_data.registrant;
        localized_domain_info_output_data.nsset = domain_info_output_data.nsset;
        localized_domain_info_output_data.keyset = domain_info_output_data.keyset;
        localized_domain_info_output_data.localized_external_states =
            localize_object_states(ctx, domain_info_output_data.states, _lang);
        localized_domain_info_output_data.sponsoring_registrar_handle = domain_info_output_data.sponsoring_registrar_handle;
        localized_domain_info_output_data.creating_registrar_handle = domain_info_output_data.creating_registrar_handle;
        localized_domain_info_output_data.last_update_registrar_handle = domain_info_output_data.last_update_registrar_handle;
        localized_domain_info_output_data.crdate = domain_info_output_data.crdate;
        localized_domain_info_output_data.last_update = domain_info_output_data.last_update;
        localized_domain_info_output_data.last_transfer = domain_info_output_data.last_transfer;
        localized_domain_info_output_data.exdate = domain_info_output_data.exdate;
        localized_domain_info_output_data.auth_info_pw =
            authinfo_has_to_be_hidden ? Nullable<std::string>() : domain_info_output_data.auth_info_pw;
        localized_domain_info_output_data.admin = domain_info_output_data.admin;
        localized_domain_info_output_data.ext_enum_domain_validation = domain_info_output_data.ext_enum_domain_validation;
        localized_domain_info_output_data.tmpcontact = domain_info_output_data.tmpcontact;

        const LocalizedSuccessResponse localized_success_response =
            create_localized_success_response(ctx, Response::ok, _lang);

        return DomainInfoResponse(
            localized_success_response,
            localized_domain_info_output_data
        );
    }
    catch (const AuthErrorServerClosingConnection&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );
    }
    catch (const NonexistentHandle&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::object_not_exist,
            std::set<Error>(),
            _lang
        );
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

}

}
