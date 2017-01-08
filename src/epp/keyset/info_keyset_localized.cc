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

#include "src/epp/keyset/info_keyset_localized.h"
#include "src/epp/keyset/info_keyset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_success.h"
#include "src/epp/impl/localization.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

InfoKeysetLocalizedResponse info_keyset_localized(
        const std::string& _keyset_handle,
        const unsigned long long _registrar_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle)
{
    // since no changes are comitted this transaction is reused for everything
    Fred::OperationContextCreator ctx;

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoKeyset)));

        Fred::OperationContextCreator ctx;
        const InfoKeysetOutputData info_keyset_data =
                info_keyset(
                        ctx,
                        _keyset_handle,
                        _registrar_id);

        const InfoKeysetLocalizedOutputData info_keyset_localized_output_data =
                InfoKeysetLocalizedOutputData(
                        info_keyset_data.handle,
                        info_keyset_data.roid,
                        info_keyset_data.sponsoring_registrar_handle,
                        info_keyset_data.creating_registrar_handle,
                        info_keyset_data.last_update_registrar_handle,
                        localize_object_states(ctx, info_keyset_data.states, _lang),
                        info_keyset_data.crdate,
                        info_keyset_data.last_update,
                        info_keyset_data.last_transfer,
                        info_keyset_data.auth_info_pw,
                        info_keyset_data.ds_records,
                        info_keyset_data.dns_keys,
                        info_keyset_data.tech_contacts);

        return InfoKeysetLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _lang),
                info_keyset_localized_output_data);

    }
    catch (const EppResponseFailure& e) {
        ctx.get_log().info(std::string("info_keyset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        ctx.get_log().info(std::string("info_keyset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        ctx.get_log().info("unexpected exception in info_keyset_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Keyset
} // namespace Epp
