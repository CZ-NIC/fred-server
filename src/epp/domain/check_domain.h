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
 *  @file check_domain.h
 *  <++>
 */

#ifndef CHECK_DOMAIN_H_29361249D9254622AD544B34701D9E63
#define CHECK_DOMAIN_H_29361249D9254622AD544B34701D9E63

#include "src/epp/domain/check_domain_localized.h"
#include "src/epp/domain/impl/domain_check_localization.h"
#include "src/epp/domain/impl/domain_registration_obstruction.h"
#include "src/fredlib/opcontext.h"

#include <set>
#include <string>

namespace Epp {

namespace Domain {

/**
 * \returns check results for given domain FQDNs
 *
 * \throws  AuthErrorServerClosingConnection
 */
DomainFqdnToDomainRegistrationObstruction check_domain(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _domain_fqdns,
    unsigned long long _registrar_id
);

}

}

#endif
