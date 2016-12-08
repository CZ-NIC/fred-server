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

#ifndef CREATE_KEYSET_LOCALIZED_H_7AD651C69E1247888C7D924FE3B981F5
#define CREATE_KEYSET_LOCALIZED_H_7AD651C69E1247888C7D924FE3B981F5

#include "src/epp/keyset/ds_record.h"
#include "src/epp/keyset/impl/dns_key.h"
#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Keyset {
namespace Localized {

struct ResponseOfCreate
{
    ResponseOfCreate(const LocalizedSuccessResponse &_response,
                     const boost::posix_time::ptime &_crdate)
    :   ok_response(_response),
        crdate(_crdate)
    { }
    LocalizedSuccessResponse ok_response;
    boost::posix_time::ptime crdate;
};

ResponseOfCreate create_keyset_localized(
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts,
    const std::vector< Keyset::DsRecord > &_ds_records,
    const std::vector< Keyset::DnsKey > &_dns_keys,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    bool _epp_notification_disabled,
    const std::string &_dont_notify_client_transaction_handles_with_this_prefix);

} // namespace Epp::Keyset::Localized
} // namespace Epp::Keyset
} // namespace Epp

#endif
