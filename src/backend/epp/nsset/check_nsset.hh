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

#ifndef CHECK_NSSET_HH_7A08F85D49C64258BBD4FE4415F42501
#define CHECK_NSSET_HH_7A08F85D49C64258BBD4FE4415F42501

#include "src/backend/epp/nsset/check_nsset_config_data.hh"
#include "src/backend/epp/nsset/nsset_handle_registration_obstruction.hh"
#include "src/backend/epp/session_data.hh"
#include "src/libfred/opcontext.hh"

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Nsset {

/**
 * @returns check results for given nsset handles
 */
std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > check_nsset(
        LibFred::OperationContext& _ctx,
        const std::set<std::string>& _nsset_handles,
        const CheckNssetConfigData& _check_nsset_config_data,
        const SessionData& _session_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
