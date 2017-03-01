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

#ifndef TRANSFER_CONTACT_LOCALIZED_H_24DC8E8F98A248C4B25980228AD6AB69
#define TRANSFER_CONTACT_LOCALIZED_H_24DC8E8F98A248C4B25980228AD6AB69

#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/notification_data.h"
#include "src/epp/impl/session_data.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {
namespace Contact {

EppResponseSuccessLocalized transfer_contact_localized(
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const Optional<unsigned long long>& _logd_request_id);


} // namespace Epp::Contact
} // namespace Epp

#endif
