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
#ifndef DELETE_CONTACT_HH_F686D915FAF44E22A9DDBBE589ACD111
#define DELETE_CONTACT_HH_F686D915FAF44E22A9DDBBE589ACD111

#include "src/backend/epp/contact/delete_contact_config_data.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Contact {

/**
 * If successful (no exception thrown) state requests of conact are performed.
 * In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns last contact history id before delete
 */
unsigned long long delete_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _handle,
        const DeleteContactConfigData& _delete_contact_config_data,
        const SessionData& _session_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
