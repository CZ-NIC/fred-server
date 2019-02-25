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
#ifndef CREATE_KEYSET_LOCALIZED_HH_DC145E03C2654D648B881CA0D8D74454
#define CREATE_KEYSET_LOCALIZED_HH_DC145E03C2654D648B881CA0D8D74454

#include "src/backend/epp/keyset/create_keyset_config_data.hh"
#include "src/backend/epp/keyset/create_keyset_input_data.hh"
#include "src/backend/epp/keyset/create_keyset_localized_response.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/notification_data.hh"

namespace Epp {
namespace Keyset {

CreateKeysetLocalizedResponse create_keyset_localized(
        const CreateKeysetInputData& _create_keyset_input_data,
        const CreateKeysetConfigData& _create_keyset_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);

} // namespace Epp::Keyset
} // namespace Epp

#endif
