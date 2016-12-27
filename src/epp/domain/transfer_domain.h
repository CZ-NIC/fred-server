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

#ifndef TRANSFER_DOMAIN_H_5EA6B0EA56704EF5ACCB8A6A2B76D1DD
#define TRANSFER_DOMAIN_H_5EA6B0EA56704EF5ACCB8A6A2B76D1DD

#include "src/epp/domain/transfer_domain_localized.h"
#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {
namespace Domain {

/**
 * If successful (no exception thrown), state requests of domain are performed.
 * In case of exception, behaviour is undefined and transaction should bo rolled back.
 *
 * \returns new history id
 *
 * \throws AuthErrorServerClosingConnection in case _registrar_id is zero (legacy reasons)
 * \throws NonexistentHandle
 * \throws ObjectNotEligibleForTransfer in case _registrar_id is of currently sponsoring registrar
 * \throws ObjectStatusProhibitsOperation
 * \throws AuthorizationInformationError in case invalid _authinfopw is given
 */
unsigned long long transfer_domain(
        Fred::OperationContext& _ctx,
        const std::string& _domain_fqdn,
        const std::string& _authinfopw,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id);

} // namespace Epp::Domain
} // namespace Epp

#endif
