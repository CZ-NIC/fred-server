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

#ifndef CHECK_NSSET_LOCALIZED_H_E0D532AC33D449DFB84FB3F0E2204B1F
#define CHECK_NSSET_LOCALIZED_H_E0D532AC33D449DFB84FB3F0E2204B1F

#include "src/epp/nsset/impl/check_nsset_localized_response.h"
#include "src/epp/impl/session_data.h"

#include <set>
#include <string>

namespace Epp {
namespace Nsset {

CheckNssetLocalizedResponse check_nsset_localized(
        const std::set<std::string>& _nsset_handles,
        const SessionData& _session_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
