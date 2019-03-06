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
#ifndef TRANSFER_CONTACT_HH_89A0361AC01E4724B9631086D19BCC5E
#define TRANSFER_CONTACT_HH_89A0361AC01E4724B9631086D19BCC5E

#include "src/backend/epp/contact/transfer_contact_localized.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Contact {

/**
 * If successful (no exception thrown) state requests of conact are performed.
 * In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns new history id
 */
unsigned long long transfer_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const TransferContactConfigData& _transfer_contact_config_data,
        const SessionData& _session_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
