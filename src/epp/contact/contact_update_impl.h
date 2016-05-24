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

/**
 *  @file
 */

#ifndef EPP_CONTACT_UPDATE_IMPL_H_004471163327
#define EPP_CONTACT_UPDATE_IMPL_H_004471163327

#include "src/fredlib/opcontext.h"
#include "src/epp/contact/contact_update.h"

namespace Epp {

/**
 * If successful (no exception thrown) state requests of conact are performed. In case of exception behaviour is undefined and transaction should bo rolled back.
 *
 * @returns new contact history id
 *
 * @throws AuthErrorServerClosingConnection
 * @throws NonexistentHandle
 * @throws AutorError
 * @throws ObjectStatusProhibitingOperation in case conatct has serverUpdateProhibited or deleteCandidate status (or request)
 * @throws AggregatedParamErrors
 */
unsigned long long contact_update_impl(
    Fred::OperationContext& _ctx,
    const ContactUpdateInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
);

}

#endif
