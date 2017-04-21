/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_CONTACT_LOCALIZED_H_8134301928FF41759B9B4E7061469BE4
#define CREATE_CONTACT_LOCALIZED_H_8134301928FF41759B9B4E7061469BE4

#include "src/epp/contact/create_contact_config_data.h"
#include "src/epp/contact/create_contact_input_data.h"
#include "src/epp/contact/create_contact_localized_response.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "util/optional_value.h"

namespace Epp {
namespace Contact {

CreateContactLocalizedResponse create_contact_localized(
        const std::string& _contact_handle,
        const CreateContactInputData& _create_contact_input_data,
        const CreateContactConfigData& _create_contact_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
