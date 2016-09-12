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
 *  @file domain_delete.h
 *  <++>
 */

#ifndef SRC_EPP_DOMAIN_DOMAIN_DELETE_H
#define SRC_EPP_DOMAIN_DOMAIN_DELETE_H

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"

#include <string>

namespace Epp {

namespace Domain {

LocalizedSuccessResponse domain_delete(
    const std::string& fqdn,
    const unsigned long long registrar_id,
    const SessionLang::Enum lang,
    const std::string& server_transaction_handle,
    const std::string& client_transaction_handle,
    const bool epp_notification_disabled,
    const std::string& client_transaction_handles_prefix_not_to_notify
);

}

}

#endif
