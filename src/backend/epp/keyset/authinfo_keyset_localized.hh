/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef AUTHINFO_KEYSET_LOCALIZED_HH_2D313922ED104F2994DD14C4311C954C
#define AUTHINFO_KEYSET_LOCALIZED_HH_2D313922ED104F2994DD14C4311C954C

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Keyset {

EppResponseSuccessLocalized authinfo_keyset_localized(
        const std::string& _keyset_handle,
        const SessionData& _session_data);


} // namespace Epp::Keyset
} // namespace Epp

#endif
