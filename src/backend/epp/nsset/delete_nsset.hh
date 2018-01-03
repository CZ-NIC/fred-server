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

#ifndef DELETE_NSSET_HH_B737D56D3518457CBD931DFB2B12C3BB
#define DELETE_NSSET_HH_B737D56D3518457CBD931DFB2B12C3BB

#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/nsset/delete_nsset_config_data.hh"
#include "src/backend/epp/session_data.hh"
#include "src/libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Nsset {

/**
 * If successful (no exception thrown) state requests of nsset are performed.
 * In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns last nsset history id before delete
 */
unsigned long long delete_nsset(
        LibFred::OperationContext& _ctx,
        const std::string& _handle,
        const DeleteNssetConfigData& _delete_nsset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Nsset
} // namespace Epp

#endif
