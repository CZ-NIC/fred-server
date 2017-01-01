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

#include "src/epp/nsset/check_nsset_localized.h"
#include "src/epp/nsset/check_nsset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/impl/nsset_handle_registration_obstruction.h"
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
#include <utility>

namespace Epp {
namespace Nsset {

CheckNssetLocalizedResponse check_nsset_localized(
        const std::set<std::string>& _nsset_handles,
        const unsigned long long _registrar_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle)
{
    Fred::OperationContextCreator ctx;

    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CheckNsset)));

        const std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > check_nsset_results =
                check_nsset(
                        ctx,
                        _nsset_handles,
                        _registrar_id);

        const LocalizedSuccessResponse localized_success_response =
                create_localized_success_response(
                        ctx,
                        EppResultCode::command_completed_successfully,
                        _lang);

        const std::map<std::string, boost::optional<NssetHandleLocalizedRegistrationObstruction> > localized_check_nsset_results =
                localize_check_results<NssetHandleRegistrationObstruction, NssetHandleLocalizedRegistrationObstruction, boost::optional>(
                        ctx,
                        check_nsset_results,
                        _lang);

        return CheckNssetLocalizedResponse(
                localized_success_response,
                localized_check_nsset_results);

    }
    catch (const EppResponseFailure& e) {
        ctx.get_log().info(std::string("check_nsset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        ctx.get_log().info(std::string("check_nsset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        ctx.get_log().info("unexpected exception in check_nsset_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Nsset
} // namespace Epp
