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

#ifndef UPDATE_KEYSET_LOCALIZED_H_D8C319D4188D49BBB144814D6A67970C
#define UPDATE_KEYSET_LOCALIZED_H_D8C319D4188D49BBB144814D6A67970C

#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/keyset/ds_record.h"
#include "src/epp/keyset/impl/dns_key.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

EppResponseSuccessLocalized update_keyset_localized(
        const std::string& _keyset_handle,
        const Optional<std::string>& _auth_info_pw,
        const std::vector<std::string>& _tech_contacts_add,
        const std::vector<std::string>& _tech_contacts_rem,
        const std::vector<Keyset::DsRecord>& _ds_records_add,
        const std::vector<Keyset::DsRecord>& _ds_records_rem,
        const std::vector<Keyset::DnsKey>& _dns_keys_add,
        const std::vector<Keyset::DnsKey>& _dns_keys_rem,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix);

} // namespace Epp::Keyset
} // namespace Epp

#endif
