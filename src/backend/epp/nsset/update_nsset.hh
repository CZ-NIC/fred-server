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

#ifndef UPDATE_NSSET_HH_31D2F031F0814B98BDD24E67C93CDD12
#define UPDATE_NSSET_HH_31D2F031F0814B98BDD24E67C93CDD12

#include "src/backend/epp/nsset/update_nsset_config_data.hh"
#include "src/backend/epp/nsset/update_nsset_input_data.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

namespace Epp {
namespace Nsset {

/**
 * If successful, no exception thrown. In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns new nsset history id
 */
unsigned long long update_nsset(
        LibFred::OperationContext& _ctx,
        const UpdateNssetInputData& _update_nsset_input_data,
        const UpdateNssetConfigData& _update_nsset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Nsset
} // namespace Epp

#endif
