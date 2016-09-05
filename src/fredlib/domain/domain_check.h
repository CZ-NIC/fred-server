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

#ifndef SRC_FREDLIB_DOMAIN_DOMAIN_CHECK_H
#define SRC_FREDLIB_DOMAIN_DOMAIN_CHECK_H

#include "src/fredlib/domain/domain.h"

#include <string>

#include "src/fredlib/opcontext.h"

namespace Fred {

namespace Domain {

DomainFqdnSyntaxValidity::Enum get_domain_fqdn_syntax_validity(OperationContext& ctx, const std::string& domain_fqdn);
DomainRegistrability::Enum get_domain_registrability_by_domain_fqdn(OperationContext& ctx, const std::string& domain_fqdn);

}

}

#endif
