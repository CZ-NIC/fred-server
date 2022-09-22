/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INFO_DOMAIN_LOCALIZED_HH_8658F949C17A4B14951DA835B8B614D0
#define INFO_DOMAIN_LOCALIZED_HH_8658F949C17A4B14951DA835B8B614D0

#include "src/backend/epp/domain/info_domain_config_data.hh"
#include "src/backend/epp/domain/info_domain_localized_response.hh"
#include "src/backend/epp/password.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Domain {

InfoDomainLocalizedResponse info_domain_localized(
        const std::string& _fqdn,
        const InfoDomainConfigData& _info_domain_config_data,
        const Password& _authinfopw,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
