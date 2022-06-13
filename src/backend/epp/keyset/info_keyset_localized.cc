/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/keyset/info_keyset_localized.hh"

#include "src/backend/epp/impl/action.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/localization.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/opcontext.hh"
#include "util/log/context.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

InfoKeysetLocalizedResponse info_keyset_localized(
        const std::string& _keyset_handle,
        const InfoKeysetConfigData& _info_keyset_config_data,
        const SessionData& _session_data)
{
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoKeyset)));

    try
    {
        LibFred::OperationContextCreator ctx;

        const InfoKeysetOutputData info_keyset_data =
                info_keyset(
                        ctx,
                        _keyset_handle,
                        _info_keyset_config_data,
                        _session_data);

        const InfoKeysetLocalizedOutputData info_keyset_localized_output_data =
                InfoKeysetLocalizedOutputData(
                        info_keyset_data.handle,
                        info_keyset_data.roid,
                        info_keyset_data.sponsoring_registrar_handle,
                        info_keyset_data.creating_registrar_handle,
                        info_keyset_data.last_update_registrar_handle,
                        localize_object_states<StatusValue>(ctx, info_keyset_data.states, _session_data.lang),
                        info_keyset_data.crdate,
                        info_keyset_data.last_update,
                        info_keyset_data.last_transfer,
                        info_keyset_data.ds_records,
                        info_keyset_data.dns_keys,
                        info_keyset_data.tech_contacts);

        return InfoKeysetLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                info_keyset_localized_output_data);

    }
    catch (const EppResponseFailure& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_keyset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_keyset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in info_keyset_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Keyset
} // namespace Epp
