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
 *  @file domain_delete_impl.h
 *  <++>
 */

#ifndef DOMAIN_DELETE_IMPL_H_D27C19B09A0148CA9626216CD4ABC86C
#define DOMAIN_DELETE_IMPL_H_D27C19B09A0148CA9626216CD4ABC86C

#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {

namespace Domain {

/**
 * If successful (no exception thrown), state requests of domain are performed.
 * In case of exception, behaviour is undefined and transaction should bo rolled back.
 *
 * \returns last domain history id before delete
 *
 * \throws AuthErrorServerClosingConnection in case registrar_id is zero (legacy reasons)
 * \throws NonexistentHandle
 * \throws AuthorizationError
 * \throws ObjectStatusProhibitsOperation  if domain has serverDeleteProhibited, serverUpdateProhibited or deleteCandidate status
 *
 * \throws Fred::DeleteDomainByHandle::Exception
 */
unsigned long long domain_delete_impl(
    Fred::OperationContext& _ctx,
    const std::string& _domain_fqdn,
    unsigned long long _registrar_id
);

}

}

#endif