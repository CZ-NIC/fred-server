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
