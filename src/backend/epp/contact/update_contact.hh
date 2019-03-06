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
#ifndef UPDATE_CONTACT_HH_2117202ACF1A4A4E99F44F43EA046C6C
#define UPDATE_CONTACT_HH_2117202ACF1A4A4E99F44F43EA046C6C

#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/update_contact_config_data.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Contact {

/**
 * If successful (no exception thrown) state requests of contact are performed.
 * In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns new contact history id
 */
unsigned long long update_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const ContactChange& _data,
        const UpdateContactConfigData& _update_contact_config_data,
        const SessionData& _session_data);

} // namespace Epp::Contact
} // namespace Epp

#endif
