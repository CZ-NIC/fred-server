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

#ifndef TRANSFER_NSSET_LOCALIZED_H_B9B03599028B45F59A9D714CD01E1696
#define TRANSFER_NSSET_LOCALIZED_H_B9B03599028B45F59A9D714CD01E1696

#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/session_lang.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {
namespace Nsset {

EppResponseSuccessLocalized transfer_nsset_localized(
        const std::string& _nsset_handle,
        const std::string& _authinfopw,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify);

} // namespace Epp::Nsset
} // namespace Epp

#endif
