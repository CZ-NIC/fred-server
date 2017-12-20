/*
 * Copyright (C) 2016 CZ.NIC, z.s.p.o.
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

#include "src/libfred/registrable_object/domain/check_domain.hh"

#include "src/libfred/registrable_object/domain/domain.hh"
#include "src/libfred/registrable_object/domain/domain_name.hh"
#include "src/libfred/object/check_handle.hh"

namespace LibFred {

namespace Domain {

DomainFqdnSyntaxValidity::Enum get_domain_fqdn_syntax_validity(
       OperationContext& ctx,
       const std::string& domain_fqdn,
       const bool is_system_registrar)
{
    const LibFred::CheckDomain domain = LibFred::CheckDomain(domain_fqdn, is_system_registrar);

    if (domain.is_bad_length(ctx)
    || domain.is_invalid_syntax(ctx)) {
        return DomainFqdnSyntaxValidity::invalid;
    }
    return DomainFqdnSyntaxValidity::valid;
}

/**
* \throws ExceptionInvalidFqdn
*/
DomainRegistrability::Enum get_domain_registrability_by_domain_fqdn(
       OperationContext& ctx,
       const std::string& domain_fqdn,
       const bool is_system_registrar)
{
    const LibFred::CheckDomain domain = LibFred::CheckDomain(domain_fqdn, is_system_registrar);

    if (domain.is_bad_zone(ctx)) {
        return DomainRegistrability::zone_not_in_registry;
    }

    if (domain.is_registered(ctx)) {
        return DomainRegistrability::registered;
    }

    if (domain.is_blacklisted(ctx)) {
        return DomainRegistrability::blacklisted;
    }

    return DomainRegistrability::available;
}

}

}
