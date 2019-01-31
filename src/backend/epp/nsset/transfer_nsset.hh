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

#ifndef TRANSFER_NSSET_HH_F0CF5B6652AA421AA0747B2638CF7B60
#define TRANSFER_NSSET_HH_F0CF5B6652AA421AA0747B2638CF7B60

#include "src/backend/epp/nsset/transfer_nsset_config_data.hh"
#include "src/backend/epp/nsset/transfer_nsset_localized.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Nsset {

/**
 * If successful (no exception thrown) state requests of nsset are performed.
 * In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns new history id
 */
unsigned long long transfer_nsset(
        LibFred::OperationContext& _ctx,
        const std::string& _nsset_handle,
        const std::string& _authinfopw,
        const TransferNssetConfigData& _transfer_nsset_config_data,
        const SessionData& _session_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
