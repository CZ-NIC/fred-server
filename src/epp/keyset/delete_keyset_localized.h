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

#ifndef DELETE_KEYSET_LOCALIZED_H_8DB04AC4562642F192AACBA17210E895
#define DELETE_KEYSET_LOCALIZED_H_8DB04AC4562642F192AACBA17210E895

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"

#include <string>

namespace Epp {
namespace Keyset {

LocalizedSuccessResponse delete_keyset_localized(
        const std::string& _keyset_handle,
        unsigned long long _registrar_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify);

} // namespace Epp::KeySet
} // namespace Epp

#endif
