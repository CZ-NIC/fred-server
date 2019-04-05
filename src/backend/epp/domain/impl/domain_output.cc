/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/epp/domain/impl/domain_output.hh"

#include "util/db/nullable.hh"
#include "libfred/object/object_state.hh"
#include "src/backend/epp/domain/status_value.hh"
#include "util/enum_conversion.hh"

#include <string>

namespace Epp {
namespace Domain {

InfoDomainOutputData get_info_domain_output(
    const LibFred::InfoDomainData& _data,
    const std::vector<LibFred::ObjectStateData>& _object_state_data,
    bool _info_is_for_sponsoring_registrar)
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
        for (std::vector<LibFred::ObjectStateData>::const_iterator object_state_ptr = _object_state_data.begin();
             object_state_ptr != _object_state_data.end();
             ++object_state_ptr)
        {
            if (object_state_ptr->is_external)
            {
                ret.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(
                                      Conversion::Enums::from_db_handle<LibFred::Object_State>(
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

    const bool authinfopw_has_to_be_hidden = !_info_is_for_sponsoring_registrar;
    ret.authinfopw = authinfopw_has_to_be_hidden ? boost::optional<std::string>() : _data.authinfopw;

    for (const auto& admin_contact : _data.admin_contacts)
    {
        ret.admin.insert(admin_contact.handle);
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
