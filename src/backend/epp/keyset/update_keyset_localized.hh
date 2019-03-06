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
#ifndef UPDATE_KEYSET_LOCALIZED_HH_83D7A3C739EF4D788C053F23ED7301C2
#define UPDATE_KEYSET_LOCALIZED_HH_83D7A3C739EF4D788C053F23ED7301C2

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/keyset/update_keyset_config_data.hh"
#include "src/backend/epp/keyset/update_keyset_input_data.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Keyset {

EppResponseSuccessLocalized update_keyset_localized(
        const UpdateKeysetInputData& _update_keyset_input_data,
        const UpdateKeysetConfigData& _update_keyset_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);

} // namespace Epp::Keyset
} // namespace Epp

#endif
