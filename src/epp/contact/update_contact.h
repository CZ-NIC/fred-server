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

#ifndef UPDATE_CONTACT_H_AC0FEA7E8B954D94BA589B65399518D9
#define UPDATE_CONTACT_H_AC0FEA7E8B954D94BA589B65399518D9

#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/update_contact_localized.h"
#include "src/fredlib/opcontext.h"

namespace Epp {
namespace Contact {

/**
 * If successful (no exception thrown) state requests of conact are performed. In case of exception behaviour is undefined and transaction should bo rolled back.
 *
 * @returns new contact history id
 *
 * @throws AuthErrorServerClosingConnection
 * @throws NonexistentHandle
 * @throws AuthorizationError
 * @throws ObjectStatusProhibitsOperation in case conatct has serverUpdateProhibited or deleteCandidate status (or request)
 * @throws AggregatedParamErrors
 */
unsigned long long update_contact(
    Fred::OperationContext &_ctx,
    const std::string &_contact_handle,
    const ContactChange &_data,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id);

} // namespace Epp::Contact
} // namespace Epp

#endif
