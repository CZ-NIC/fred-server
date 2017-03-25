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

#include "src/epp/nsset/info_nsset.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/epp/nsset/status_value.h"
#include "src/fredlib/nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

InfoNssetOutputData info_nsset(
        Fred::OperationContext& _ctx,
        const std::string& _nsset_handle,
        const InfoNssetConfigData& _info_nsset_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const Fred::InfoNssetData info_nsset_data =
            Fred::InfoNssetByHandle(_nsset_handle).exec(_ctx, "UTC").info_nsset_data;

        // tech contact handle list
        std::vector<std::string> tech_contacts;
        tech_contacts.reserve(info_nsset_data.tech_contacts.size());
        BOOST_FOREACH(const Fred::ObjectIdHandlePair & tech_contact, info_nsset_data.tech_contacts) {
            tech_contacts.push_back(tech_contact.handle);
        }

        const std::string session_registrar_handle =
                Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle;
        const bool authinfopw_has_to_be_hidden = info_nsset_data.sponsoring_registrar_handle !=
                                                 session_registrar_handle;

        std::set<StatusValue::Enum> info_nsset_output_data_states;
        {
            typedef std::vector<Fred::ObjectStateData> ObjectStatesData;

            ObjectStatesData domain_states_data = Fred::GetObjectStates(info_nsset_data.id).exec(_ctx);
            for (ObjectStatesData::const_iterator object_state = domain_states_data.begin();
                 object_state != domain_states_data.end(); ++object_state)
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
                info_nsset_data.handle,
                info_nsset_data.roid,
                info_nsset_data.sponsoring_registrar_handle,
                info_nsset_data.create_registrar_handle,
                info_nsset_data.update_registrar_handle,
                info_nsset_output_data_states,
                info_nsset_data.creation_time,
                info_nsset_data.update_time,
                info_nsset_data.transfer_time,
                // show object authinfopw only to sponsoring registrar
                authinfopw_has_to_be_hidden ? boost::optional<std::string>() : info_nsset_data.authinfopw,
                make_epp_dnshosts_output(info_nsset_data.dns_hosts),
                tech_contacts,
                info_nsset_data.tech_check_level.get_value_or(0));
    }
    catch (const Fred::InfoNssetByHandle::Exception& e)
    {

        if (e.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}


} // namespace Epp::Nsset
} // namespace Epp
