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

#ifndef AUTHINFO_DOMAIN_HH_F6ACE0BDD41845D0A3246E9D50E75DEB
#define AUTHINFO_DOMAIN_HH_F6ACE0BDD41845D0A3246E9D50E75DEB

#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Domain {

void authinfo_domain(
        LibFred::OperationContext& _ctx,
        const std::string& _fqdn,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
