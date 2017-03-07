/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "src/epp/nsset/impl/get_nsset_info.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/fredlib/object_state/get_object_states.h"

#include <vector>
#include <set>
#include <string>

namespace Epp {
namespace Nsset {

InfoNssetOutputData get_nsset_info(
    const Fred::InfoNssetData& data,
    const std::vector<Fred::ObjectStateData>& object_states_data,
    bool authinfopw_has_to_be_hidden)
{
    std::set<std::string> object_states;
    for (std::vector<Fred::ObjectStateData>::const_iterator object_state_it = object_states_data.begin();
        object_state_it != object_states_data.end();
        ++object_state_it)
    {
        object_states.insert(object_state_it->state_name);
    }

    std::vector<std::string> tech_contacts;
    tech_contacts.reserve(data.tech_contacts.size());
    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator object_id_handle_pair = data.tech_contacts.begin();
         object_id_handle_pair != data.tech_contacts.end();
         ++object_id_handle_pair)
    {
        tech_contacts.push_back(object_id_handle_pair->handle);
    }

    return InfoNssetOutputData(
        data.handle,
        data.roid,
        data.sponsoring_registrar_handle,
        data.create_registrar_handle,
        data.update_registrar_handle,
        object_states,
        data.creation_time,
        data.update_time,
        data.transfer_time,
        authinfopw_has_to_be_hidden ? boost::optional<std::string>() : data.authinfopw,
        make_epp_dnshosts_output(data.dns_hosts),
        tech_contacts,
        data.tech_check_level.get_value_or(0));
}

} // namespace Epp::Nsset
} // namespace Epp
