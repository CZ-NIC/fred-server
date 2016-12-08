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
#include "src/epp/impl/states_localized.h"
#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/object/object_state.h"

namespace Epp {
namespace Keyset {
namespace Localized {

struct InfoData
{
    std::string handle; ///< Keyset handle
    std::string roid; ///< Keyset identifier - repository ID
    std::string sponsoring_registrar_handle; ///< registrar identifier, which has to right for change
    std::string creating_registrar_handle; ///< Registrar identifier, which created contact
    Nullable< std::string > last_update_registrar_handle; ///< Registrar identifier, which realized changes
    LocalizedStates states; ///< Keyset states list
    boost::posix_time::ptime crdate; ///< Creation date and time
    Nullable< boost::posix_time::ptime > last_update; ///< Date and time of last change
    Nullable< boost::posix_time::ptime > last_transfer; ///< Date and time of last transfer
    Nullable< std::string > auth_info_pw; ///< Password for keyset transfer
    InfoKeysetData::DsRecords ds_records; ///< List of ds records
    InfoKeysetData::DnsKeys dns_keys; ///< List of dnskeys
    InfoKeysetData::TechContacts tech_contacts; ///< List of technical contacts identifier
};

struct InfoResult
{
    InfoResult(const InfoData &_data, const LocalizedSuccessResponse &_response)
    :   data(_data),
        response(_response)
    { }
    InfoData data;
    LocalizedSuccessResponse response;
};

InfoResult info_keyset_localized(
    const std::string &_keyset_handle,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle);

} // namespace Epp::Keyset::Localized
} // namespace Epp::Keyset
} // namespace Epp

#endif
