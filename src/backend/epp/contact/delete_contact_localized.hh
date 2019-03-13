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
#ifndef DELETE_CONTACT_LOCALIZED_HH_DE2A45072B1746CC8952BEAAD13CC456
#define DELETE_CONTACT_LOCALIZED_HH_DE2A45072B1746CC8952BEAAD13CC456

#include "src/backend/epp/contact/delete_contact_config_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Contact {

EppResponseSuccessLocalized delete_contact_localized(
        const std::string& _contact_handle,
        const DeleteContactConfigData& _delete_contact_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
