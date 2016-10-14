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
 *  @file domain_update_impl.h
 *  <++>
 */

#ifndef SRC_EPP_DOMAIN_DOMAIN_UPDATE_IMPL_H
#define SRC_EPP_DOMAIN_DOMAIN_UPDATE_IMPL_H

#include "src/epp/contact/contact_update.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {

namespace Domain {

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
unsigned long long contact_update_impl(
    Fred::OperationContext &_ctx,
    const std::string& _domain_fqdn,
    const std::string& _registrant_change,
    const Optional<std::string>& _auth_info_pw_change,
    const std::string& _nsset_change,
    const std::string& _keyset_change,
    const std::vector<std::string>& _admin_contacts_add,
    const std::vector<std::string>& _admin_contacts_rem,
    const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id);

}

}

#endif
