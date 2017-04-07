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

#include "src/epp/domain/impl/get_domain_info.h"

#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/object/object_id_handle_pair.h"
#include "util/db/nullable.h"
#include "util/enum_conversion.h"

#include <string>
#include <iterator>

namespace Epp {
namespace Domain {

InfoDomainOutputData get_domain_info(
    const Fred::InfoDomainData& data,
    const std::vector<Fred::ObjectStateData>& object_states_data,
    bool authinfopw_has_to_be_hidden)
{
    InfoDomainOutputData ret;

    ret.roid = data.roid;
    ret.fqdn = data.fqdn;
    ret.registrant = data.registrant.handle;
    ret.nsset = data.nsset.isnull()
        ? Nullable<std::string>()
        : Nullable<std::string>(data.nsset.get_value().handle);
    ret.keyset = data.keyset.isnull()
        ? Nullable<std::string>()
        : Nullable<std::string>(data.keyset.get_value().handle);

    for (std::vector<Fred::ObjectStateData>::const_iterator object_state_it = object_states_data.begin();
         object_state_it != object_states_data.end();
         ++object_state_it)
    {
        if (object_state_it->is_external)
        {
            ret.states.insert(Conversion::Enums::from_status_value_name<Epp::Domain::StatusValue>(object_state_it->state_name));
        }
    }

    ret.sponsoring_registrar_handle = data.sponsoring_registrar_handle;
    ret.creating_registrar_handle = data.create_registrar_handle;
    ret.last_update_registrar_handle = data.update_registrar_handle;
    ret.crdate = data.creation_time;
    ret.last_update = data.update_time;
    ret.last_transfer = data.transfer_time;
    ret.exdate = data.expiration_date;

    ret.authinfopw = authinfopw_has_to_be_hidden ? boost::optional<std::string>() : data.authinfopw;

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator object_id_handle_pair = data.admin_contacts.begin();
         object_id_handle_pair != data.admin_contacts.end();
         ++object_id_handle_pair)
    {
        ret.admin.insert(object_id_handle_pair->handle);
    }

    ret.ext_enum_domain_validation = data.enum_domain_validation.isnull()
        ? Nullable<EnumValidationExtension>()
        : EnumValidationExtension(
            data.enum_domain_validation.get_value().validation_expiration,
            data.enum_domain_validation.get_value().publish);

    return ret;
}

} // namespace Epp::Domain
} // namespace Epp
