/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/domain/check_domain.h"

#include "src/epp/domain/domain_registration_obstruction.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/idn_utils.h"

#include <boost/foreach.hpp>

#include <map>

namespace Epp {
namespace Domain {

namespace {

Nullable<DomainRegistrationObstruction::Enum> domain_get_registration_obstruction_by_fqdn(
        Fred::OperationContext& _ctx,
        const std::string& _domain_fqdn)
{
    switch (Fred::Domain::get_domain_registrability_by_domain_fqdn(_ctx, _domain_fqdn))
    {

        case Fred::Domain::DomainRegistrability::registered:
            return DomainRegistrationObstruction::registered;

        case Fred::Domain::DomainRegistrability::blacklisted:
            return DomainRegistrationObstruction::blacklisted;

        case Fred::Domain::DomainRegistrability::zone_not_in_registry:
            return DomainRegistrationObstruction::zone_not_in_registry;

        case Fred::Domain::DomainRegistrability::available:

            switch (Fred::Domain::get_domain_fqdn_syntax_validity(_ctx, _domain_fqdn))
            {
                case Fred::Domain::DomainFqdnSyntaxValidity::invalid:
                    return DomainRegistrationObstruction::invalid_fqdn;

                case Fred::Domain::DomainFqdnSyntaxValidity::valid:
                    return Nullable<DomainRegistrationObstruction::Enum>();
            }
            throw std::logic_error("Unexpected Fred::Domain::DomainFqdnSyntaxValidity::Enum value.");

    }
    throw std::logic_error("Unexpected Fred::Domain::DomainRegistrability::Enum value.");
}


} // namespace Epp::Domain::{anonymous}

std::map<std::string, Nullable<DomainRegistrationObstruction::Enum> > check_domain(
        Fred::OperationContext& _ctx,
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
