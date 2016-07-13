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

#include "src/epp/nsset/nsset_info_impl.h"

#include <fredlib/nsset.h>
#include <fredlib/registrar.h>
#include "src/fredlib/object_state/get_object_states.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/nsset_impl.h"

#include <boost/foreach.hpp>

#include <iterator>
#include <vector>
#include <algorithm>

namespace Epp {

static std::set<std::string> convert_object_states(const std::vector<Fred::ObjectStateData>& _object_states) {
    std::set<std::string> result;

    BOOST_FOREACH(const Fred::ObjectStateData& state, _object_states) {
        result.insert(state.state_name);
    }

    return result;
}

NssetInfoOutputData nsset_info_impl(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    const SessionLang::Enum _object_state_description_lang,
    const unsigned long long _session_registrar_id
) {
    if( _session_registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    try {
        const Fred::InfoNssetData nsset_info_data = Fred::InfoNssetByHandle(_handle).exec(_ctx, "UTC").info_nsset_data;

        //tech contact handle list
        std::vector<std::string> tech_contacts;
        tech_contacts.reserve(nsset_info_data.tech_contacts.size());
        BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_contact, nsset_info_data.tech_contacts) {
            tech_contacts.push_back(tech_contact.handle);
        }

        return NssetInfoOutputData(
            nsset_info_data.handle,
            nsset_info_data.roid,
            nsset_info_data.sponsoring_registrar_handle,
            nsset_info_data.create_registrar_handle,
            nsset_info_data.update_registrar_handle,
            convert_object_states(Fred::GetObjectStates(nsset_info_data.id).exec(_ctx) ),
            nsset_info_data.creation_time,
            nsset_info_data.update_time,
            nsset_info_data.transfer_time,
            nsset_info_data.authinfopw,
            make_epp_dnshosts_data(nsset_info_data.dns_hosts),
            tech_contacts,
            nsset_info_data.tech_check_level.get_value_or(0)
        );

    } catch (const Fred::InfoNssetByHandle::Exception& e) {

        if(e.is_set_unknown_handle()) {
            throw NonexistentHandle();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
