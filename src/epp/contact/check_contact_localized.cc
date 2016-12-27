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

#include "src/epp/contact/check_contact_localized.h"
#include "src/epp/contact/check_contact.h"

#include "src/epp/contact/impl/contact_handle_registration_obstruction.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/optional.hpp>

#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Contact {

typedef std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > HandleToObstruction;
typedef std::map<std::string, boost::optional<ContactHandleLocalizedRegistrationObstruction> > HandleToLocalizedObstruction;

CheckContactLocalizedResponse check_contact_localized(
        const std::set<std::string>& _contact_handles,
        const unsigned long long _registrar_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle)
{
    Fred::OperationContextCreator ctx;

    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CheckContact)));

        const HandleToObstruction check_contact_results =
                check_contact(
                        ctx,
                        _contact_handles,
                        _registrar_id);

        const LocalizedSuccessResponse localized_success_response =
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang);

        const HandleToLocalizedObstruction localized_check_contact_results =
                localize_check_results<ContactHandleRegistrationObstruction, ContactHandleLocalizedRegistrationObstruction, boost::optional>(
                        ctx,
                        check_contact_results,
                        _lang);

        return CheckContactLocalizedResponse(
                localized_success_response,
                localized_check_contact_results);

    }
    //catch (const AuthErrorServerClosingConnection&) {
    //    throw create_localized_fail_response(
    //            ctx,
    //            Response::authentication_error_server_closing_connection,
    //            std::set<Error>(),
    //            _lang);
    //}
    catch (const EppResponseFailure& e) {
        ctx.get_log().info(std::string("check_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        ctx.get_log().info(std::string("check_contact_localized failure: ") + e.what());
        throw create_localized_fail_response(
                ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch (...) {
        ctx.get_log().info("unexpected exception in check_contact_localized function");
        throw create_localized_fail_response(
                ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
