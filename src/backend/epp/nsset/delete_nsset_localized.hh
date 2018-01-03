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

#ifndef DELETE_NSSET_LOCALIZED_HH_9BB0774F653D4769AEA8CEFE461A6352
#define DELETE_NSSET_LOCALIZED_HH_9BB0774F653D4769AEA8CEFE461A6352

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/nsset/delete_nsset_config_data.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Nsset {

EppResponseSuccessLocalized delete_nsset_localized(
        const std::string& _nsset_handle,
        const DeleteNssetConfigData& _delete_nsset_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);


} // namespace Epp::Nsset
} // namespace Epp

#endif
