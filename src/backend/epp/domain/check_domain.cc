/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/epp/domain/check_domain.hh"

#include "src/backend/epp/domain/domain_registration_obstruction.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/domain.hh"
#include "libfred/registrable_object/domain/domain_name.hh"
#include "libfred/object/object_type.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/idn_utils.hh"

#include <boost/foreach.hpp>

#include <map>

namespace Epp {
namespace Domain {

namespace {

Nullable<DomainRegistrationObstruction::Enum> domain_get_registration_obstruction_by_fqdn(
        LibFred::OperationContext& _ctx,
        const std::string& _domain_fqdn)
{
    switch (LibFred::Domain::get_domain_registrability_by_domain_fqdn(_ctx, _domain_fqdn))
    {

        case LibFred::Domain::DomainRegistrability::registered:
            return DomainRegistrationObstruction::registered;

        case LibFred::Domain::DomainRegistrability::blacklisted:
            return DomainRegistrationObstruction::blacklisted;

        case LibFred::Domain::DomainRegistrability::zone_not_in_registry:
            return DomainRegistrationObstruction::zone_not_in_registry;

        case LibFred::Domain::DomainRegistrability::available:

            switch (LibFred::Domain::get_domain_fqdn_syntax_validity(_ctx, _domain_fqdn))
            {
                case LibFred::Domain::DomainFqdnSyntaxValidity::invalid:
                    return DomainRegistrationObstruction::invalid_fqdn;

                case LibFred::Domain::DomainFqdnSyntaxValidity::valid:
                    return Nullable<DomainRegistrationObstruction::Enum>();
            }
            throw std::logic_error("Unexpected LibFred::Domain::DomainFqdnSyntaxValidity::Enum value.");

    }
    throw std::logic_error("Unexpected LibFred::Domain::DomainRegistrability::Enum value.");
}


} // namespace Epp::Domain::{anonymous}

std::map<std::string, Nullable<DomainRegistrationObstruction::Enum> > check_domain(
        LibFred::OperationContext& _ctx,
        const std::set<std::string>& _fqdn,
        const CheckDomainConfigData& _check_domain_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    std::map<std::string, Nullable<DomainRegistrationObstruction::Enum> >
            fqdn_to_domain_registration_obstruction;

    BOOST_FOREACH (const std::string& fqdn, _fqdn)
    {
        fqdn_to_domain_registration_obstruction[fqdn] =
                domain_get_registration_obstruction_by_fqdn(_ctx, fqdn);
    }

    return fqdn_to_domain_registration_obstruction;
}


} // namespace Epp::Domain
} // namespace Epp
