/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef CHECK_DOMAIN_HH_1BDF5A43C48645919D20881B6D63C254
#define CHECK_DOMAIN_HH_1BDF5A43C48645919D20881B6D63C254

#include "src/backend/epp/domain/check_domain_config_data.hh"
#include "src/backend/epp/domain/check_domain_localized.hh"
#include "src/backend/epp/domain/domain_registration_obstruction.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Domain {

/**
 * \returns check results for given domain names
 *
 * \throws  AuthErrorServerClosingConnection
 */
std::map<std::string, Nullable<DomainRegistrationObstruction::Enum> > check_domain(
        LibFred::OperationContext& _ctx,
        const std::set<std::string>& _fqdns,
        const CheckDomainConfigData& _check_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
