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

#ifndef AUTHINFO_DOMAIN_LOCALIZED_HH_3C61552E46284C548154E4331CE41B67
#define AUTHINFO_DOMAIN_LOCALIZED_HH_3C61552E46284C548154E4331CE41B67

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Domain {

EppResponseSuccessLocalized authinfo_domain_localized(
        const std::string& _fqdn,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
