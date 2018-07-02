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

#ifndef AUTHINFO_NSSET_LOCALIZED_HH_EA3EA38DA664446E9D1B153F165B366C
#define AUTHINFO_NSSET_LOCALIZED_HH_EA3EA38DA664446E9D1B153F165B366C

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Nsset {

EppResponseSuccessLocalized authinfo_nsset_localized(
        const std::string& _nsset_handle,
        const SessionData& _session_data);


} // namespace Epp::Nsset
} // namespace Epp

#endif
