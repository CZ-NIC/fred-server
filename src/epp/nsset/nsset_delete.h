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

#ifndef EPP_NSSET_DELETE_H_c87679541493459eadae29ef4953902d
#define EPP_NSSET_DELETE_H_c87679541493459eadae29ef4953902d

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"

#include <string>

namespace Epp {

LocalizedSuccessResponse nsset_delete(
    const std::string& _handle,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string& _client_transaction_handles_prefix_not_to_nofify
);

}

#endif
