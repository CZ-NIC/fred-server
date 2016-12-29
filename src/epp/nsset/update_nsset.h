/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_NSSET_H_A39C4BFF30BD43699F6D06CC93A6B4EC
#define UPDATE_NSSET_H_A39C4BFF30BD43699F6D06CC93A6B4EC

#include "src/fredlib/opcontext.h"
#include "src/epp/nsset/update_nsset_localized.h"

namespace Epp {
namespace Nsset {

/**
 * If successful, no exception thrown. In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns new nsset history id
 *
 * @throws AuthErrorServerClosingConnection
 * @throws NonexistentHandle
 * @throws AuthorizationError
 * @throws ObjectStatusProhibitsOperation in case nsset has serverUpdateProhibited or deleteCandidate status (or request)
 * @throws AggregatedParamErrors
 */
unsigned long long update_nsset(
        Fred::OperationContext& _ctx,
        const UpdateNssetInputData& _data,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id);

} // namespace Epp::Nsset
} // namespace Epp

#endif
