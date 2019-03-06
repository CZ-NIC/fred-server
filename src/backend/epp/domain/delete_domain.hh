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
#ifndef DELETE_DOMAIN_HH_773335E44C874F31866B775F01912D84
#define DELETE_DOMAIN_HH_773335E44C874F31866B775F01912D84

#include "src/backend/epp/domain/delete_domain_config_data.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Domain {

/**
 * If successful (no exception thrown), state requests of domain are performed.
 * In case of exception, behaviour is undefined and transaction should bo rolled back.
 *
 * \returns last domain history id before delete
 */
unsigned long long delete_domain(
        LibFred::OperationContext& _ctx,
        const std::string& _domain_fqdn,
        const DeleteDomainConfigData& _delete_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
