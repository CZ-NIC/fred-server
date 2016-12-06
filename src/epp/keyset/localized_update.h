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

#ifndef LOCALIZED_UPDATE_H_B5ACACE3D6594CA7BA517C2C333CD9D3
#define LOCALIZED_UPDATE_H_B5ACACE3D6594CA7BA517C2C333CD9D3

#include "src/epp/keyset/ds_record.h"
#include "src/epp/keyset/dns_key.h"
#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Keyset {
namespace Localized {

LocalizedSuccessResponse update(
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts_add,
    const std::vector< std::string > &_tech_contacts_rem,
    const std::vector< Keyset::DsRecord > &_ds_records_add,
    const std::vector< Keyset::DsRecord > &_ds_records_rem,
    const std::vector< Keyset::DnsKey > &_dns_keys_add,
    const std::vector< Keyset::DnsKey > &_dns_keys_rem,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    bool _epp_notification_disabled,
    const std::string &_dont_notify_client_transaction_handles_with_this_prefix);

}//namespace Epp::Keyset::Localized
}//namespace Epp::Keyset
}//namespace Epp

#endif
