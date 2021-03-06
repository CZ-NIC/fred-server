/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DELETE_KEYSET_LOCALIZED_HH_E1D3526EA7414AADB2E2AB103F8E83B1
#define DELETE_KEYSET_LOCALIZED_HH_E1D3526EA7414AADB2E2AB103F8E83B1

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/keyset/delete_keyset_config_data.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Keyset {

EppResponseSuccessLocalized delete_keyset_localized(
        const std::string& _keyset_handle,
        const DeleteKeysetConfigData& _delete_keyset_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);


} // namespace Epp::KeySet
} // namespace Epp

#endif
