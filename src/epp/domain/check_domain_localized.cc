/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/domain/check_domain_localized.h"
#include "src/epp/domain/check_domain.h"

#include "src/epp/domain/impl/domain_registration_obstruction.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Domain {

CheckDomainLocalizedResponse check_domain_localized(
        const std::set<std::string>& _domain_fqdns,
        const unsigned long long _registrar_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle)
{
    Fred::OperationContextCreator ctx;

    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CheckDomain)));

        const std::map<std::string, Nullable<DomainRegistrationObstruction::Enum> > check_domain_results =
                check_domain(
                        ctx,
                        _domain_fqdns,
                        _registrar_id);

        const LocalizedSuccessResponse localized_success_response =
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang);

        const std::map<std::string, boost::optional<DomainLocalizedRegistrationObstruction> > localized_check_results =
                localize_check_results<DomainRegistrationObstruction, DomainLocalizedRegistrationObstruction, boost::optional>(
                        ctx,
                        check_domain_results,
                        _lang);

        return CheckDomainLocalizedResponse(
                localized_success_response,
                localized_check_results);
    }
    catch (const AuthErrorServerClosingConnection&) {
        throw create_localized_fail_response(
                ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang);
    }
    catch (const std::exception& e) {
        ctx.get_log().info(std::string("check_domain_localized failure: ") + e.what());
        throw create_localized_fail_response(
                ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch (...) {
        ctx.get_log().info("unexpected exception in check_domain_localized function");
        throw create_localized_fail_response(
                ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }

}

} // namespace Epp::Domain
} // namespace Epp
