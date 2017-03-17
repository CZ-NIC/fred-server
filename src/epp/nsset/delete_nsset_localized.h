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

#ifndef DELETE_NSSET_LOCALIZED_H_9D46AC55B43743898CF9E6A41641A1BC
#define DELETE_NSSET_LOCALIZED_H_9D46AC55B43743898CF9E6A41641A1BC

#include "src/epp/epp_response_success_localized.h"
#include "src/epp/notification_data.h"
#include "src/epp/nsset/delete_nsset_config_data.h"
#include "src/epp/session_data.h"

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
