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

#include "src/epp/keyset/impl/get_keyset_info.h"

namespace Epp {
namespace Keyset {

InfoKeysetOutputData get_keyset_info(
    const Fred::InfoKeysetData& data,
    const std::vector<Fred::ObjectStateData>& keyset_states_data,
    bool authinfopw_has_to_be_hidden)
{
    InfoKeysetOutputData ret;

    ret.handle = data.handle;
    ret.roid = data.roid;
    ret.sponsoring_registrar_handle = data.sponsoring_registrar_handle;
    ret.creating_registrar_handle = data.create_registrar_handle;
    ret.last_update_registrar_handle = data.update_registrar_handle;
    for (std::vector<Fred::ObjectStateData>::const_iterator data_it = keyset_states_data.begin();
         data_it != keyset_states_data.end();
         ++data_it)
    {
        ret.states.insert(Conversion::Enums::from_status_value_name<Epp::Keyset::StatusValue>(data_it->state_name));
    }
    ret.crdate = data.creation_time;
    ret.last_update = data.update_time;
    ret.last_transfer = data.transfer_time;
    if (!authinfopw_has_to_be_hidden)
    {
        ret.authinfopw = data.authinfopw;
    }

    for (std::vector<Fred::DnsKey>::const_iterator data_it = data.dns_keys.begin();
         data_it != data.dns_keys.end();
         ++data_it)
    {
        ret.dns_keys.insert(Keyset::DnsKey(data_it->get_flags(),
                                           data_it->get_protocol(),
                                           data_it->get_alg(),
                                           data_it->get_key()));
    }
    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator data_it = data.tech_contacts.begin();
         data_it != data.tech_contacts.end();
         ++data_it)
    {
        ret.tech_contacts.insert(data_it->handle);
    }

    return ret;
}

} // namespace Epp::Keyset
} // namespace Epp
