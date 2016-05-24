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

/**
 *  @file
 */

#include "src/epp/nsset/nsset_info.h"

#include "src/epp/nsset/nsset_info_impl.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/localization.h"
#include "src/epp/action.h"

#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/nsset/info_nsset.h"

#include "util/log/context.h"

#include <algorithm>
#include <set>
#include <vector>
#include <boost/foreach.hpp>

namespace Epp {

LocalizedInfoNssetResponse nsset_info(
    const std::string& _handle,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
) {
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
    Logging::Context logging_ctx3(_server_transaction_handle);
    Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::NSsetInfo)));

    /* since no changes are comitted this transaction is reused for everything */

    Fred::OperationContextCreator ctx;

    try {
        NssetInfoOutputData payload = nsset_info_impl(ctx, _handle, _lang, _registrar_id);

        /* show object authinfo only to sponsoring registrar */
        if(payload.sponsoring_registrar_handle != Fred::InfoRegistrarById(_registrar_id).exec(ctx).info_registrar_data.handle) {
            payload.auth_info_pw = std::string();
        }

        // hide internal states
        {
            std::set<std::string> filtered_states;

            // TODO udelat pres ziskani externich stavu, zatim na to neni ve fredlibu rozhrani
            //TODO Fred::GetObjectStates pro "handle"
            const std::vector<Fred::ObjectStateData> state_definitions =
                Fred::GetObjectStates(
                    Fred::InfoNssetByHandle(payload.handle).exec(ctx).info_nsset_data.id
                ).exec(ctx);

            BOOST_FOREACH(const std::string& state, payload.states) {
                BOOST_FOREACH(const Fred::ObjectStateData& state_def, state_definitions) {
                    if(state_def.is_external) {
                        filtered_states.insert(state);
                    }
                }
            }

            payload.states = filtered_states;
        }

        /* XXX HACK: OK state */
        if( payload.states.empty() ) {
            payload.states.insert("ok");
        }

        std::vector<DNShost> dns_hosts;
        dns_hosts.reserve(payload.dns_hosts.size());
        BOOST_FOREACH(const DNShostData& host, payload.dns_hosts) {
            dns_hosts.push_back(DNShost(host.fqdn, host.inet_addr));
        }

        return LocalizedInfoNssetResponse(
            create_localized_success_response(Response::ok, ctx, _lang),
            LocalizedNssetInfoOutputData(
                payload.handle,
                payload.roid,
                payload.sponsoring_registrar_handle,
                payload.creating_registrar_handle,
                payload.last_update_registrar_handle,
                get_object_state_descriptions(ctx, payload.states, _lang),
                payload.crdate,
                payload.last_update,
                payload.last_transfer,
                payload.auth_info_pw,
                dns_hosts,
                payload.tech_contacts,
                payload.tech_check_level
            )
        );

    } catch (const AuthErrorServerClosingConnection& e) {
        throw create_localized_fail_response(
            ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );

    } catch (const NonexistentHandle& e) {
        throw create_localized_fail_response(
            ctx,
            Response::object_not_exist,
            std::set<Error>(),
            _lang
        );
    }
}

}
