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

#ifndef CREATE_KEYSET_LOCALIZED_H_7AD651C69E1247888C7D924FE3B981F5
#define CREATE_KEYSET_LOCALIZED_H_7AD651C69E1247888C7D924FE3B981F5

#include "src/epp/keyset/impl/create_keyset_localized_response.h"
#include "src/epp/keyset/impl/create_keyset_input_data.h"
#include "src/epp/impl/session_data.h"
#include "src/epp/impl/notification_data.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

CreateKeysetLocalizedResponse create_keyset_localized(
        const CreateKeysetInputData& _create_keyset_input_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const Optional<unsigned long long>& _logd_request_id);

} // namespace Epp::Keyset
} // namespace Epp

#endif
