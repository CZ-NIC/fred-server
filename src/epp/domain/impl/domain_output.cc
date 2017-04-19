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

#include "src/epp/domain/impl/domain_output.h"

#include "util/db/nullable.h"
#include "src/fredlib/object/object_id_handle_pair.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/epp/domain/status_value.h"
#include "util/enum_conversion.h"

#include <string>
#include <vector>

namespace Epp {
namespace Domain {

InfoDomainOutputData get_info_domain_output(
    Fred::OperationContext& _ctx,
    const Fred::InfoDomainData& _data,
    unsigned long long _registrar_id)
{
    InfoDomainOutputData ret;

    ret.roid = _data.roid;
    ret.fqdn = _data.fqdn;
    ret.registrant = _data.registrant.handle;
    ret.nsset = _data.nsset.isnull()
        ? Nullable<std::string>()
        : Nullable<std::string>(_data.nsset.get_value().handle);
    ret.keyset = _data.keyset.isnull()
        ? Nullable<std::string>()
        : Nullable<std::string>(_data.keyset.get_value().handle);

    {
        typedef std::vector<Fred::ObjectStateData> ObjectStatesData;

        ObjectStatesData domain_states_data = Fred::GetObjectStates(_data.id).exec(_ctx);
        for (ObjectStatesData::const_iterator object_state_ptr = domain_states_data.begin();
             object_state_ptr != domain_states_data.end();
             ++object_state_ptr)
        {
            if (object_state_ptr->is_external)
            {
                ret.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(
                                      Conversion::Enums::from_db_handle<Fred::Object_State>(
                                          object_state_ptr->state_name)));
            }
        }
    }

    ret.sponsoring_registrar_handle = _data.sponsoring_registrar_handle;
    ret.creating_registrar_handle = _data.create_registrar_handle;
    ret.last_update_registrar_handle = _data.update_registrar_handle;

    ret.crdate = _data.creation_time;
    ret.last_update = _data.update_time;
    ret.last_transfer = _data.transfer_time;
    ret.exdate = _data.expiration_date;

    // show object authinfopw only to sponsoring registrar
    const std::string session_registrar_handle =
        Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle;
    const bool authinfopw_has_to_be_hidden = _data.sponsoring_registrar_handle != session_registrar_handle;
    ret.authinfopw = authinfopw_has_to_be_hidden ? boost::optional<std::string>() : _data.authinfopw;

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator object_id_handle_pair =
             _data.admin_contacts.begin();
         object_id_handle_pair != _data.admin_contacts.end();
         ++object_id_handle_pair)
    {
        ret.admin.insert(object_id_handle_pair->handle);
    }

    ret.ext_enum_domain_validation =
        _data.enum_domain_validation.isnull()
        ? Nullable<EnumValidationExtension>()
        : EnumValidationExtension(
            _data.enum_domain_validation.get_value().validation_expiration,
            _data.enum_domain_validation.get_value().publish);

    return ret;
}

} // namespace Epp::Domain
} // namespace Epp
