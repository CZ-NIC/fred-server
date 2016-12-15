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

#include "src/epp/nsset/info_nsset.h"

#include "src/fredlib/nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/impl/nsset.h"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <iterator>
#include <string>
#include <vector>
#include <algorithm>

namespace Epp {
namespace Nsset {

static std::set<std::string> convert_object_states(const std::vector<Fred::ObjectStateData>& _object_states) {
    std::set<std::string> result;

    BOOST_FOREACH(const Fred::ObjectStateData& state, _object_states) {
        result.insert(state.state_name);
    }

    return result;
}

InfoNssetOutputData info_nsset(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    const SessionLang::Enum _object_state_description_lang,
    const unsigned long long _session_registrar_id
) {
    if( _session_registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    try {
        const Fred::InfoNssetData info_nsset_data = Fred::InfoNssetByHandle(_handle).exec(_ctx, "UTC").info_nsset_data;

        //tech contact handle list
        std::vector<std::string> tech_contacts;
        tech_contacts.reserve(info_nsset_data.tech_contacts.size());
        BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_contact, info_nsset_data.tech_contacts) {
            tech_contacts.push_back(tech_contact.handle);
        }

        const std::string callers_registrar_handle = Fred::InfoRegistrarById(_session_registrar_id).exec(_ctx).info_registrar_data.handle;
        const bool callers_is_sponsoring_registrar = info_nsset_data.sponsoring_registrar_handle == callers_registrar_handle;
        const bool authinfo_has_to_be_hidden = !callers_is_sponsoring_registrar;

        return InfoNssetOutputData(
            info_nsset_data.handle,
            info_nsset_data.roid,
            info_nsset_data.sponsoring_registrar_handle,
            info_nsset_data.create_registrar_handle,
            info_nsset_data.update_registrar_handle,
            convert_object_states(Fred::GetObjectStates(info_nsset_data.id).exec(_ctx) ),
            info_nsset_data.creation_time,
            info_nsset_data.update_time,
            info_nsset_data.transfer_time,
            // show object authinfo only to sponsoring registrar
            authinfo_has_to_be_hidden ? boost::optional<std::string>() : info_nsset_data.authinfopw,
            make_epp_dnshosts_output(info_nsset_data.dns_hosts),
            tech_contacts,
            info_nsset_data.tech_check_level.get_value_or(0)
        );

    } catch (const Fred::InfoNssetByHandle::Exception& e) {

        if(e.is_set_unknown_handle()) {
            throw NonexistentHandle();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

} // namespace Epp::Nsset
} // namespace Epp
