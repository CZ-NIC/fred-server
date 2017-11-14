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

#include "src/epp/nsset/impl/nsset_output.h"
#include "src/epp/nsset/info_nsset.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <string>

namespace Epp {
namespace Nsset {

InfoNssetOutputData::InfoNssetOutputData()
{ }

InfoNssetOutputData::InfoNssetOutputData(
        const std::string& _handle,
        const std::string& _roid,
        const std::string& _sponsoring_registrar_handle,
        const std::string& _creating_registrar_handle,
        const Nullable<std::string>& _last_update_registrar_handle,
        const std::set<StatusValue::Enum>& _states,
        const boost::posix_time::ptime& _crdate,
        const Nullable<boost::posix_time::ptime>& _last_update,
        const Nullable<boost::posix_time::ptime>& _last_transfer,
        const boost::optional<std::string>& _authinfopw,
        const std::vector<DnsHostOutput>& _dns_hosts,
        const std::vector<std::string>& _tech_contacts,
        short _tech_check_level)
    : handle(_handle),
      roid(_roid),
      sponsoring_registrar_handle(_sponsoring_registrar_handle),
      creating_registrar_handle(_creating_registrar_handle),
      last_update_registrar_handle(_last_update_registrar_handle),
      states(_states),
      crdate(_crdate),
      last_update(_last_update),
      last_transfer(_last_transfer),
      authinfopw(_authinfopw),
      dns_hosts(_dns_hosts),
      tech_contacts(_tech_contacts),
      tech_check_level(_tech_check_level)
{ }

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

        const std::string session_registrar_handle =
            Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle;
        const bool info_is_for_sponsoring_registrar =
            info_nsset_data.sponsoring_registrar_handle == session_registrar_handle;

        const std::vector<Fred::ObjectStateData> object_states_data =
            Fred::GetObjectStates(info_nsset_data.id).exec(_ctx);

        return get_info_nsset_output(info_nsset_data, object_states_data, info_is_for_sponsoring_registrar);
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
