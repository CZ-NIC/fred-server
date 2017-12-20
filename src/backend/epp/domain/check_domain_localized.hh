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
 * along with FRED.  If not, see <http://www.gnu.or/licenses/>.
 */

#ifndef CHECK_DOMAIN_LOCALIZED_H_2EB8975220034A3F92F6BF3D499C6EF5
#define CHECK_DOMAIN_LOCALIZED_H_2EB8975220034A3F92F6BF3D499C6EF5

#include "src/backend/epp/domain/check_domain_config_data.hh"
#include "src/backend/epp/domain/check_domain_localized_response.hh"
#include "src/backend/epp/session_data.hh"

#include <set>
#include <string>

namespace Epp {
namespace Domain {

CheckDomainLocalizedResponse check_domain_localized(
        const std::set<std::string>& _domain_names,
        const CheckDomainConfigData& _check_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
