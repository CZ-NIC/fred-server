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

#include "src/epp/keyset/check_keyset_localized.h"

#include "src/epp/impl/action.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_response_success_localized.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "src/epp/session_data.h"
#include "src/epp/impl/util.h"
#include "src/epp/keyset/check_keyset.h"
#include "src/epp/keyset/keyset_handle_registration_obstruction.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Keyset {

CheckKeysetLocalizedResponse check_keyset_localized(
        const std::set<std::string>& _keyset_handles,
        const SessionData& _session_data)
{
    Fred::OperationContextCreator ctx;

    try
    {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CheckKeyset)));

        const std::map<std::string, Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> >
                check_keyset_results =
                        check_keyset(
                                ctx,
                                _keyset_handles,
                                _session_data.registrar_id);

        return CheckKeysetLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                localize_check_results<KeysetHandleRegistrationObstruction,
                        CheckKeysetLocalizedResponse::Result, Nullable>(
                        ctx,
                        check_keyset_results,
                        _session_data.lang));

    }
    catch (const EppResponseFailure& e)
    {
        ctx.get_log().info(std::string("check_keyset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        ctx.get_log().info(std::string("check_keyset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        ctx.get_log().info("unexpected exception in check_keyset_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Keyset
} // namespace Epp
