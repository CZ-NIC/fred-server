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

#include "src/backend/epp/contact/transfer_contact_config_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "src/util/optional_value.hh"

#include <string>

namespace Epp {
namespace Contact {

EppResponseSuccessLocalized transfer_contact_localized(
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const TransferContactConfigData& _transfer_contact_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
