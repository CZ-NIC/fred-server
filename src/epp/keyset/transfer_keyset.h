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

#ifndef TRANSFER_KEYSET_H_14553F4B5766411C9489766C4B95E283
#define TRANSFER_KEYSET_H_14553F4B5766411C9489766C4B95E283

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Epp {
namespace Keyset {

/**
 * If successful (no exception thrown) state requests of keyset are performed. In case of exception
 * behaviour is undefined and transaction should be rolled back.
 *
 * @returns new history id
 *
 * @throws AuthErrorServerClosingConnection in case _registrar_id is zero (legacy reasons)
 * @throws NonexistentHandle
 * @throws ObjectNotEligibleForTransfer in case _registrar_id is of currently sponsoring registrar
 * @throws ObjectStatusProhibitsOperation
 * @throws AutorError in case invalid _authinfopw is given
 */
unsigned long long transfer_keyset(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const std::string &_authinfopw,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id);

} // namespace Epp::Keyset
} // namespace Epp

#endif
