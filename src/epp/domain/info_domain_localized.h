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

#ifndef INFO_DOMAIN_LOCALIZED_H_23BA4FC5083B45F2AB3100C54E6AF3F6
#define INFO_DOMAIN_LOCALIZED_H_23BA4FC5083B45F2AB3100C54E6AF3F6

#include "src/epp/domain/info_domain_localized_response.h"
#include "src/epp/session_data.h"

namespace Epp {
namespace Domain {

InfoDomainLocalizedResponse info_domain_localized(
        const std::string& _domain_fqdn,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
