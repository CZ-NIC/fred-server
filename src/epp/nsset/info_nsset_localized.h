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

#ifndef INFO_NSSET_LOCALIZED_H_47356271F650459CBC0999B992C5C6DD
#define INFO_NSSET_LOCALIZED_H_47356271F650459CBC0999B992C5C6DD

#include "src/epp/impl/session_data.h"
#include "src/epp/nsset/impl/info_nsset_localized_response.h"

#include <string>

namespace Epp {
namespace Nsset {

InfoNssetLocalizedResponse info_nsset_localized(
        const std::string& _handle,
        const SessionData& _session_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
