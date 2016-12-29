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
#include "src/epp/nsset/info_nsset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/foreach.hpp>

namespace Epp {
namespace Nsset {

InfoNssetLocalizedResponse info_nsset_localized(
        const std::string& _nsset_handle,
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
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoNsset)));

        InfoNssetOutputData info_nsset_data =
                info_nsset(
                        ctx,
                        _nsset_handle,
                        _lang,
                        _registrar_id);

        // hide internal states
        {
            std::set<std::string> filtered_states;

            // TODO udelat pres ziskani externich stavu, zatim na to neni ve fredlibu rozhrani
            // TODO Fred::GetObjectStates pro "handle"
            const std::vector<Fred::ObjectStateData> state_definitions =
                Fred::GetObjectStates(
                    Fred::InfoNssetByHandle(info_nsset_data.handle).exec(ctx).info_nsset_data.id
                ).exec(ctx);

            BOOST_FOREACH(const std::string& state, info_nsset_data.states) {
                BOOST_FOREACH(const Fred::ObjectStateData& state_def, state_definitions) {
                    if(state_def.is_external) {
                        filtered_states.insert(state);
                    }
                }
            }

            info_nsset_data.states = filtered_states;
        }

        /* XXX HACK: OK state */
        if (info_nsset_data.states.empty()) {
            info_nsset_data.states.insert("ok");
        }

        return InfoNssetLocalizedResponse(
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang),
                InfoNssetLocalizedOutputData(
                        info_nsset_data.handle,
                        info_nsset_data.roid,
                        info_nsset_data.sponsoring_registrar_handle,
                        info_nsset_data.creating_registrar_handle,
                        info_nsset_data.last_update_registrar_handle,
                        localize_object_states_deprecated(ctx, info_nsset_data.states, _lang),
                        info_nsset_data.crdate,
                        info_nsset_data.last_update,
                        info_nsset_data.last_transfer,
                        info_nsset_data.auth_info_pw,
                        info_nsset_data.dns_hosts,
                        info_nsset_data.tech_contacts,
                        info_nsset_data.tech_check_level));

    }
    catch (const EppResponseFailure& e) {
        ctx.get_log().info(std::string("info_nsset_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _lang);
    }
    catch (const std::exception& e) {
        ctx.get_log().info(std::string("info_nsset_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
    catch (...) {
        ctx.get_log().info("unexpected exception in info_nsset_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _lang);
    }
}

} // namespace Epp::Nsset
} // namespace Epp
