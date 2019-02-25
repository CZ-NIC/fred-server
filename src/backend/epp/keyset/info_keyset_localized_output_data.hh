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
#ifndef INFO_KEYSET_LOCALIZED_OUTPUT_DATA_HH_34C5C535B7A24C7EBA8375205F1326E8
#define INFO_KEYSET_LOCALIZED_OUTPUT_DATA_HH_34C5C535B7A24C7EBA8375205F1326E8

#include "src/backend/epp/keyset/info_keyset_output_data.hh"
#include "src/backend/epp/keyset/status_value.hh"
#include "src/backend/epp/object_states_localized.hh"
#include "libfred/object/object_state.hh"
#include "util/db/nullable.hh"

#include <boost/optional.hpp>

#include <string>

namespace Epp {
namespace Keyset {

struct InfoKeysetLocalizedOutputData
{
    std::string handle; ///< Keyset handle
    std::string roid; ///< Keyset identifier - repository ID
    std::string sponsoring_registrar_handle; ///< registrar identifier, which has to right for change
    std::string creating_registrar_handle; ///< Registrar identifier, which created contact
    Nullable<std::string> last_update_registrar_handle; ///< Registrar identifier, which realized changes
    ObjectStatesLocalized<StatusValue> localized_external_states; ///< Keyset states list
    boost::posix_time::ptime crdate; ///< Creation date and time
    Nullable<boost::posix_time::ptime> last_update; ///< Date and time of last change
    Nullable<boost::posix_time::ptime> last_transfer; ///< Date and time of last transfer
    boost::optional<std::string> authinfopw; ///< Password for keyset transfer
    InfoKeysetOutputData::DsRecords ds_records; ///< List of ds records
    InfoKeysetOutputData::DnsKeys dns_keys; ///< List of dnskeys
    InfoKeysetOutputData::TechContacts tech_contacts; ///< List of technical contacts identifier


    InfoKeysetLocalizedOutputData(
            const std::string& _handle,
            const std::string& _roid,
            const std::string& _sponsoring_registrar_handle,
            const std::string& _creating_registrar_handle,
            const Nullable<std::string>& _last_update_registrar_handle,
            const ObjectStatesLocalized<Epp::Keyset::StatusValue> _localized_external_states,
            const boost::posix_time::ptime& _crdate,
            const Nullable<boost::posix_time::ptime>& _last_update,
            const Nullable<boost::posix_time::ptime>& _last_transfer,
            const boost::optional<std::string>& _authinfopw,
            const InfoKeysetOutputData::DsRecords& _ds_records,
            const InfoKeysetOutputData::DnsKeys& _dns_keys,
            const InfoKeysetOutputData::TechContacts& _tech_contacts)
        : handle(_handle),
          roid(_roid),
          sponsoring_registrar_handle(_sponsoring_registrar_handle),
          creating_registrar_handle(_creating_registrar_handle),
          last_update_registrar_handle(_last_update_registrar_handle),
          localized_external_states(_localized_external_states),
          crdate(_crdate),
          last_update(_last_update),
          last_transfer(_last_transfer),
          authinfopw(_authinfopw),
          ds_records(_ds_records),
          dns_keys(_dns_keys),
          tech_contacts(_tech_contacts)
    {
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
