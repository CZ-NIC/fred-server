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

/**
 *  @file
 */

#ifndef EPP_NSSET_DELETE_IMPL_H_2a716b63190a4c0fbb93bd7a913c0266
#define EPP_NSSET_DELETE_IMPL_H_2a716b63190a4c0fbb93bd7a913c0266

#include <string>

#include "src/fredlib/opcontext.h"

namespace Epp {

/**
 * If successful (no exception thrown) state requests of nsset are performed. In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns last nsset history id before delete
 *
 * @throws AuthErrorServerClosingConnection
 * @throws NonexistentHandle
 * @throws AuthorizationError
 * @throws ObjectStatusProhibitsOperation in case nsset has serverDeleteProhibited, serverUpdateProhibited, deleteCandidate or linked status (or request)
 */
unsigned long long nsset_delete_impl(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    unsigned long long _registrar_id
);

}

#endif
