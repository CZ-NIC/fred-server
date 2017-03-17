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

#ifndef TRANSFER_CONTACT_H_A01B6F5659C348CB9F8372FEBD555CDB
#define TRANSFER_CONTACT_H_A01B6F5659C348CB9F8372FEBD555CDB

#include "src/epp/contact/transfer_contact_localized.h"
#include "src/fredlib/opcontext.h"

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
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const TransferContactConfigData& _transfer_contact_config_data,
        const SessionData& _session_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
