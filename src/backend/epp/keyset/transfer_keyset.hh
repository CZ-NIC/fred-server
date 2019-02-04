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

#ifndef TRANSFER_KEYSET_HH_B1C90D1301214376BF412BFFACC601CB
#define TRANSFER_KEYSET_HH_B1C90D1301214376BF412BFFACC601CB

#include "src/backend/epp/keyset/transfer_keyset_config_data.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"
#include "util/optional_value.hh"

#include <string>

namespace Epp {
namespace Keyset {

/**
 * If successful (no exception thrown) state requests of keyset are performed. In case of exception
 * behaviour is undefined and transaction should be rolled back.
 *
 * @returns new history id
 */
unsigned long long transfer_keyset(
        LibFred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const std::string& _authinfopw,
        const TransferKeysetConfigData& _transfer_keyset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Keyset
} // namespace Epp

#endif
