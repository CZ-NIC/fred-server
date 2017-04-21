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

#include "src/epp/nsset/info_nsset_localized.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/exception.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/util.h"
#include "src/epp/localization.h"
#include "src/epp/notification_data.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/epp/nsset/info_nsset.h"
#include "src/epp/object_states_localized.h"
#include "src/epp/session_data.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <algorithm>
#include <boost/foreach.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

InfoNssetLocalizedResponse info_nsset_localized(
        const std::string& _nsset_handle,
        const InfoNssetConfigData& _info_nsset_config_data,
        const SessionData& _session_data)
{
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoNsset)));

    try
    {
        Fred::OperationContextCreator ctx;

        InfoNssetOutputData info_nsset_output_data =
                info_nsset(
                        ctx,
                        _nsset_handle,
                        _info_nsset_config_data,
                        _session_data);

        const InfoNssetLocalizedOutputData info_nsset_localized_output_data =
                InfoNssetLocalizedOutputData(
                        info_nsset_output_data.handle,
                        info_nsset_output_data.roid,
                        info_nsset_output_data.sponsoring_registrar_handle,
                        info_nsset_output_data.creating_registrar_handle,
                        info_nsset_output_data.last_update_registrar_handle,
                        localize_object_states<StatusValue>(ctx, info_nsset_output_data.states, _session_data.lang),
                        info_nsset_output_data.crdate,
                        info_nsset_output_data.last_update,
                        info_nsset_output_data.last_transfer,
                        info_nsset_output_data.authinfopw,
                        info_nsset_output_data.dns_hosts,
                        info_nsset_output_data.tech_contacts,
                        info_nsset_output_data.tech_check_level);

        return InfoNssetLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                info_nsset_localized_output_data);

    }
    catch (const EppResponseFailure& e)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_nsset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_nsset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in info_nsset_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Nsset
} // namespace Epp
