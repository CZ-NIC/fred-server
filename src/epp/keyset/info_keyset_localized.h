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

/**
 *  @file
 */

#ifndef INFO_KEYSET_LOCALIZED_H_FC17377A92AD4B6287BF8BCCBC3E1F93
#define INFO_KEYSET_LOCALIZED_H_FC17377A92AD4B6287BF8BCCBC3E1F93

#include "src/epp/keyset/impl/info_keyset_data.h"
#include "src/epp/impl/object_states_localized.h"
#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/object/object_state.h"
#include "util/db/nullable.h"

#include <boost/optional.hpp>

namespace Epp {
namespace Keyset {
namespace Localized {

struct InfoKeysetLocalizedOutputData
{
    std::string handle; ///< Keyset handle
    std::string roid; ///< Keyset identifier - repository ID
    std::string sponsoring_registrar_handle; ///< registrar identifier, which has to right for change
    std::string creating_registrar_handle; ///< Registrar identifier, which created contact
    Nullable<std::string> last_update_registrar_handle; ///< Registrar identifier, which realized changes
    ObjectStatesLocalized localized_states; ///< Keyset states list
    boost::posix_time::ptime crdate; ///< Creation date and time
    Nullable<boost::posix_time::ptime> last_update; ///< Date and time of last change
    Nullable<boost::posix_time::ptime> last_transfer; ///< Date and time of last transfer
    boost::optional<std::string> auth_info_pw; ///< Password for keyset transfer
    InfoKeysetOutputData::DsRecords ds_records; ///< List of ds records
    InfoKeysetOutputData::DnsKeys dns_keys; ///< List of dnskeys
    InfoKeysetOutputData::TechContacts tech_contacts; ///< List of technical contacts identifier

    InfoKeysetLocalizedOutputData(
        const std::string& _handle,
        const std::string& _roid,
        const std::string& _sponsoring_registrar_handle,
        const std::string& _creating_registrar_handle,
        const Nullable<std::string>& _last_update_registrar_handle,
        const ObjectStatesLocalized& _localized_states,
        const boost::posix_time::ptime& _crdate,
        const Nullable<boost::posix_time::ptime>& _last_update,
        const Nullable<boost::posix_time::ptime>& _last_transfer,
        const boost::optional<std::string>& _auth_info_pw,
        const InfoKeysetOutputData::DsRecords& _ds_records,
        const InfoKeysetOutputData::DnsKeys& _dns_keys,
        const InfoKeysetOutputData::TechContacts& _tech_contacts)
    :
        handle(_handle),
        roid(_roid),
        sponsoring_registrar_handle(_sponsoring_registrar_handle),
        creating_registrar_handle(_creating_registrar_handle),
        last_update_registrar_handle(_last_update_registrar_handle),
        localized_states(_localized_states),
        crdate(_crdate),
        last_update(_last_update),
        last_transfer(_last_transfer),
        auth_info_pw(_auth_info_pw),
        ds_records(_ds_records),
        dns_keys(_dns_keys),
        tech_contacts(_tech_contacts)
    { }

};

struct InfoKeysetLocalizedResponse
{
    LocalizedSuccessResponse ok_response;
    InfoKeysetLocalizedOutputData data;

    InfoKeysetLocalizedResponse(
        const LocalizedSuccessResponse& _ok_response,
        const InfoKeysetLocalizedOutputData& _data)
    :   ok_response(_ok_response),
        data(_data)
    { }
};

InfoKeysetLocalizedResponse info_keyset_localized(
        const std::string& _keyset_handle,
        unsigned long long _registrar_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle);

} // namespace Epp::Keyset::Localized
} // namespace Epp::Keyset
} // namespace Epp

#endif
