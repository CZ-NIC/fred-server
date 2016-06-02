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

#ifndef DELETE_H_5C740B7ECE9319D5BBB37EC46383588B//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define DELETE_H_5C740B7ECE9319D5BBB37EC46383588B

#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {

/**
 * If successful (no exception thrown) state requests of conact are performed. In case of exception behaviour is undefined and transaction should bo rolled back.
 *
 * @returns last contact history id before delete
 *
 * @throws AuthErrorServerClosingConnection
 * @throws NonexistentHandle
 * @throws AutorError
 * @throws ObjectStatusProhibitingOperation in case contact has serverDeleteProhibited, serverUpdateProhibited, deleteCandidate or linked status (or request)
 */
unsigned long long keyset_delete(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    unsigned long long _registrar_id);

}

#endif//DELETE_H_5C740B7ECE9319D5BBB37EC46383588B
