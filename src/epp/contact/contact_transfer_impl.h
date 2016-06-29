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

#ifndef EPP_CONTACT_TRANSFER_IMPL_H_63598781301
#define EPP_CONTACT_TRANSFER_IMPL_H_63598781301

#include "src/epp/contact/ident_type.h"
#include "src/fredlib/opcontext.h"

#include "src/epp/contact/contact_transfer.h"

namespace Epp {

/**
 * If successful (no exception thrown) state requests of conact are performed. In case of exception behaviour is undefined and transaction should bo rolled back.
 *
 * @returns new history id
 *
 * @throws AuthErrorServerClosingConnection in case _registrar_id is zero (legacy reasons)
 * @throws NonexistentHandle
 * @throws ObjectNotEligibleForTransfer in case _registrar_id is of currently sponsoring registrar
 * @throws ObjectStatusProhibitingOperation
 * @throws AuthorizationError in case invalid _authinfopw is given
 */
unsigned long long contact_transfer_impl(
    Fred::OperationContext& _ctx,
    const std::string& _contact_handle,
    const std::string& _authinfopw,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
);

}

#endif
