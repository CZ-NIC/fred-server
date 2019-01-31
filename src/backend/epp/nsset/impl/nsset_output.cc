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

#include "src/backend/epp/nsset/impl/nsset_output.hh"

#include "src/deprecated/libfred/registrable_object/nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_data.hh"
#include "src/backend/epp/nsset/impl/nsset.hh"
#include "src/backend/epp/nsset/status_value.hh"

#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#include <set>
#include <vector>
#include <string>

namespace Epp {
namespace Nsset {

InfoNssetOutputData get_info_nsset_output(
    const LibFred::InfoNssetData& _data,
    const std::vector<LibFred::ObjectStateData>& _object_state_data,
    bool _info_is_for_sponsoring_registrar)
{
    std::vector<std::string> tech_contacts;
    tech_contacts.reserve(_data.tech_contacts.size());
    BOOST_FOREACH(const LibFred::ObjectIdHandlePair & tech_contact, _data.tech_contacts) {
        tech_contacts.push_back(tech_contact.handle);
    }

    std::set<StatusValue::Enum> info_nsset_output_data_states;
    {
        for (std::vector<LibFred::ObjectStateData>::const_iterator object_state = _object_state_data.begin();
             object_state != _object_state_data.end();
             ++object_state)
        {
            if (object_state->is_external)
            {
                info_nsset_output_data_states.insert(Conversion::Enums::from_fred_object_state<Epp::Nsset::StatusValue>(
                                                         Conversion::Enums::from_db_handle<LibFred::Object_State>(
                                                             object_state->state_name)));
            }
        }
    }

    const bool authinfopw_has_to_be_hidden = !_info_is_for_sponsoring_registrar;
    return InfoNssetOutputData(
        _data.handle,
        _data.roid,
        _data.sponsoring_registrar_handle,
        _data.create_registrar_handle,
        _data.update_registrar_handle,
        info_nsset_output_data_states,
        _data.creation_time,
        _data.update_time,
        _data.transfer_time,
        authinfopw_has_to_be_hidden ? boost::optional<std::string>() : _data.authinfopw,
        make_epp_dnshosts_output(_data.dns_hosts),
        tech_contacts,
        _data.tech_check_level.get_value_or(0));
}

} // namespace Epp::Nsset
} // namespace Epp
