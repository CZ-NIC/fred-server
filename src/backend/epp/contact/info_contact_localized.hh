/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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
#ifndef INFO_CONTACT_LOCALIZED_HH_EF41DA3A6AD7403CBBD9DB1371A55FC0
#define INFO_CONTACT_LOCALIZED_HH_EF41DA3A6AD7403CBBD9DB1371A55FC0

#include "src/backend/epp/contact/info_contact_config_data.hh"
#include "src/backend/epp/contact/info_contact_localized_response.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/password.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Contact {

InfoContactLocalizedResponse info_contact_localized(
        const std::string& _contact_handle,
        const InfoContactConfigData& _info_contact_config_data,
        const Password& _authinfopw,
        const SessionData& _session_data);

} // namespace Epp::Contact
} // namespace Epp

#endif
