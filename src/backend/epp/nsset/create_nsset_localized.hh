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

#ifndef CREATE_NSSET_LOCALIZED_HH_AB2E8CDC515043D9882B554DA8295FEA
#define CREATE_NSSET_LOCALIZED_HH_AB2E8CDC515043D9882B554DA8295FEA

#include "src/backend/epp/nsset/create_nsset_config_data.hh"
#include "src/backend/epp/nsset/create_nsset_input_data.hh"
#include "src/backend/epp/nsset/create_nsset_localized_response.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "util/optional_value.hh"

namespace Epp {
namespace Nsset {

CreateNssetLocalizedResponse create_nsset_localized(
        const CreateNssetInputData& _create_nsset_input_data,
        const CreateNssetConfigData& _create_nsset_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
