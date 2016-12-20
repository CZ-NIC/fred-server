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

#ifndef TRANSFER_KEYSET_LOCALIZED_H_49BA69E2FC3645A09CA592D1E3FDD10A
#define TRANSFER_KEYSET_LOCALIZED_H_49BA69E2FC3645A09CA592D1E3FDD10A

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {
namespace Keyset {

LocalizedSuccessResponse transfer_keyset_localized(
        const std::string& _keyset_handle,
        const std::string& _authinfopw,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify);

} // namespace Epp::Keyset
} // namespace Epp

#endif
