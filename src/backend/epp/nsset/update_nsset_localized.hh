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
#ifndef UPDATE_NSSET_LOCALIZED_HH_590E7F1557224C248AAF931E4C7F5DDF
#define UPDATE_NSSET_LOCALIZED_HH_590E7F1557224C248AAF931E4C7F5DDF

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/nsset/update_nsset_config_data.hh"
#include "src/backend/epp/nsset/update_nsset_input_data.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Nsset {

EppResponseSuccessLocalized update_nsset_localized(
        const UpdateNssetInputData& _update_nsset_input_data,
        const UpdateNssetConfigData& _update_nsset_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
