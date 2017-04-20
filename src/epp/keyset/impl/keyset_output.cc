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

#include "src/epp/keyset/impl/keyset_output.h"
#include "src/epp/keyset/status_value.h"
#include "src/fredlib/keyset/info_keyset.h"

namespace Epp {
namespace Keyset {

InfoKeysetOutputData get_info_keyset_output(
    const Fred::InfoKeysetData& _data,
    const std::vector<Fred::ObjectStateData>& _object_state_data,
    bool _info_is_for_sponsored_registrar)
{
    InfoKeysetOutputData ret;

    ret.handle = _data.handle;
    ret.roid = _data.roid;
    ret.sponsoring_registrar_handle = _data.sponsoring_registrar_handle;
    ret.creating_registrar_handle = _data.create_registrar_handle;
    ret.last_update_registrar_handle = _data.update_registrar_handle;
    {

        for (std::vector<Fred::ObjectStateData>::const_iterator object_state = _object_state_data.begin();
             object_state != _object_state_data.end();
             ++object_state)
        {
            if (object_state->is_external)
            {
                ret.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(
                                      Conversion::Enums::from_db_handle<Fred::Object_State>(
                                          object_state->state_name)));
            }
        }
    }
    ret.crdate = _data.creation_time;
    ret.last_update = _data.update_time;
    ret.last_transfer = _data.transfer_time;
    // show object authinfopw only to sponsoring registrar
    ret.authinfopw = _info_is_for_sponsored_registrar ? _data.authinfopw : boost::optional<std::string>();
    // ret.ds_records = ... // Fred::InfoKeysetData doesn't contain any ds record informations
    {
        typedef std::vector<Fred::DnsKey> FredDnsKeys;
        for (FredDnsKeys::const_iterator fred_dns_key = _data.dns_keys.begin();
             fred_dns_key != _data.dns_keys.end(); ++fred_dns_key)
        {
            ret.dns_keys.insert(
                Keyset::DnsKey(
                    fred_dns_key->get_flags(),
                    fred_dns_key->get_protocol(),
                    fred_dns_key->get_alg(),
                    fred_dns_key->get_key()));
        }
    }
    {
        typedef std::vector<Fred::ObjectIdHandlePair> FredObjectIdHandle;
        for (FredObjectIdHandle::const_iterator tech_contact = _data.tech_contacts.begin();
             tech_contact != _data.tech_contacts.end(); ++tech_contact)
        {
            ret.tech_contacts.insert(tech_contact->handle);
        }
    }
    return ret;
}

} // namespace Epp::Keyset
} // namespace Epp
