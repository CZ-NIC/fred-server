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

#ifndef DELETE_KEYSET_H_B1F9FD75689C471A8C4FAB8A30DB2B80
#define DELETE_KEYSET_H_B1F9FD75689C471A8C4FAB8A30DB2B80

#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {

/**
 * If successful (no exception thrown) state requests of keyset are performed. In case of exception
 * behaviour is undefined and transaction should be rolled back.
 *
 * @returns last keyset history id before delete
 *
 * @throws AuthErrorServerClosingConnection
 * @throws NonexistentHandle
 * @throws AutorError
 * @throws ObjectStatusProhibitsOperation in case contact has serverDeleteProhibited, serverUpdateProhibited, deleteCandidate or linked status (or request)
 */
unsigned long long keyset_delete(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    unsigned long long _registrar_id);

}//namespace Epp

#endif
