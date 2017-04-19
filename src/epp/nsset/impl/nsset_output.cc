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

#include "src/epp/nsset/impl/nsset_output.h"

#include "src/fredlib/nsset.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/epp/nsset/status_value.h"

#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#include <set>
#include <vector>
#include <string>

namespace Epp {
namespace Nsset {

InfoNssetOutputData get_info_nsset_output(
    Fred::OperationContext& _ctx,
    const Fred::InfoNssetData& _data,
    unsigned long long _registrar_id)
{
    std::vector<std::string> tech_contacts;
    tech_contacts.reserve(_data.tech_contacts.size());
    BOOST_FOREACH(const Fred::ObjectIdHandlePair & tech_contact, _data.tech_contacts) {
        tech_contacts.push_back(tech_contact.handle);
    }

    const std::string session_registrar_handle =
        Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle;
    const bool authinfopw_has_to_be_hidden = _data.sponsoring_registrar_handle != session_registrar_handle;

    std::set<StatusValue::Enum> info_nsset_output_data_states;
    {
        typedef std::vector<Fred::ObjectStateData> ObjectStatesData;

        ObjectStatesData domain_states_data = Fred::GetObjectStates(_data.id).exec(_ctx);
        for (ObjectStatesData::const_iterator object_state = domain_states_data.begin();
             object_state != domain_states_data.end();
             ++object_state)
        {
            if (object_state->is_external)
            {
                info_nsset_output_data_states.insert(Conversion::Enums::from_fred_object_state<Epp::Nsset::StatusValue>(
                                                         Conversion::Enums::from_db_handle<Fred::Object_State>(
                                                             object_state->state_name)));
            }
        }
    }

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
        // show object authinfopw only to sponsoring registrar
        authinfopw_has_to_be_hidden ? boost::optional<std::string>() : _data.authinfopw,
        make_epp_dnshosts_output(_data.dns_hosts),
        tech_contacts,
        _data.tech_check_level.get_value_or(0));
}

} // namespace Epp::Nsset
} // namespace Epp
