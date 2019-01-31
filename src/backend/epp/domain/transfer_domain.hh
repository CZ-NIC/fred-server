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

#ifndef TRANSFER_DOMAIN_HH_3D3B7D9BD9F142AC8347A049ED72C916
#define TRANSFER_DOMAIN_HH_3D3B7D9BD9F142AC8347A049ED72C916

#include "src/backend/epp/domain/transfer_domain_localized.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Domain {

/**
 * If successful (no exception thrown), state requests of domain are performed.
 * In case of exception, behaviour is undefined and transaction should bo rolled back.
 *
 * \returns new history id
 */
unsigned long long transfer_domain(
        LibFred::OperationContext& _ctx,
        const std::string& _fqdn,
        const std::string& _authinfopw,
        const TransferDomainConfigData& _transfer_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
