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
 *  @file domain_check_impl.h
 *  <++>
 */

#ifndef DOMAIN_CHECK_IMPL_H_E841B122C54D49D28251BBFEB3F69769
#define DOMAIN_CHECK_IMPL_H_E841B122C54D49D28251BBFEB3F69769

#include "src/epp/domain/domain_check.h"
#include "src/epp/domain/domain_check_localization.h"
#include "src/epp/domain/domain_registration_obstruction.h"
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
DomainFqdnToDomainRegistrationObstruction domain_check_impl(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _domain_fqdns,
    unsigned long long _registrar_id
);

}

}

#endif