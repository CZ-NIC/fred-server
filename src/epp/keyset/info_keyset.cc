/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/keyset/info_keyset.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/keyset/status_value.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

InfoKeysetOutputData info_keyset(
        Fred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const InfoKeysetConfigData& _info_keyset_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        InfoKeysetOutputData result;
        static const char* const utc_timezone = "UTC";
        const Fred::InfoKeysetData info_keyset_data =
            Fred::InfoKeysetByHandle(_keyset_handle).exec(_ctx, utc_timezone).info_keyset_data;
        result.handle = info_keyset_data.handle;
        result.roid = info_keyset_data.roid;
        result.sponsoring_registrar_handle = info_keyset_data.sponsoring_registrar_handle;
        result.creating_registrar_handle = info_keyset_data.create_registrar_handle;
        result.last_update_registrar_handle = info_keyset_data.update_registrar_handle;
        {

            typedef std::vector<Fred::ObjectStateData> ObjectStatesData;

            ObjectStatesData keyset_states_data = Fred::GetObjectStates(info_keyset_data.id).exec(_ctx);
            for (ObjectStatesData::const_iterator object_state = keyset_states_data.begin();
                 object_state != keyset_states_data.end(); ++object_state)
            {
                if (object_state->is_external)
                {
                    result.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(
                            Conversion::Enums::from_db_handle<Fred::Object_State>(
                                    object_state->state_name)));
                }
            }
        }
        result.crdate = info_keyset_data.creation_time;
        result.last_update = info_keyset_data.update_time;
        result.last_transfer = info_keyset_data.transfer_time;
        // show object authinfopw only to sponsoring registrar
        const unsigned long long sponsoring_registrar_id =
            Fred::InfoRegistrarByHandle(info_keyset_data.sponsoring_registrar_handle).exec(_ctx).info_registrar_data.id;
        if (sponsoring_registrar_id == _session_data.registrar_id)
        {
            result.authinfopw = info_keyset_data.authinfopw;
        }
        // result.ds_records = ... // Fred::InfoKeysetData doesn't contain any ds record informations
        {

            typedef std::vector<Fred::DnsKey> FredDnsKeys;
            for (FredDnsKeys::const_iterator fred_dns_key = info_keyset_data.dns_keys.begin();
                 fred_dns_key != info_keyset_data.dns_keys.end(); ++fred_dns_key)
            {
                result.dns_keys.insert(
                        Keyset::DnsKey(
                                fred_dns_key->get_flags(),
                                fred_dns_key->get_protocol(),
                                fred_dns_key->get_alg(),
                                fred_dns_key->get_key()));
            }
        }
        {

            typedef std::vector<Fred::ObjectIdHandlePair> FredObjectIdHandle;
            for (FredObjectIdHandle::const_iterator tech_contact = info_keyset_data.tech_contacts.begin();
                 tech_contact != info_keyset_data.tech_contacts.end(); ++tech_contact)
            {
                result.tech_contacts.insert(tech_contact->handle);
            }
        }
        return result;
    }
    catch (const Fred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        throw;
    }
}


} // namespace Epp::Keyset
} // namespace Epp
