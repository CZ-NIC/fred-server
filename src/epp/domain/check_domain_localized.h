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

#ifndef CHECK_DOMAIN_LOCALIZED_H_2EB8975220034A3F92F6BF3D499C6EF5
#define CHECK_DOMAIN_LOCALIZED_H_2EB8975220034A3F92F6BF3D499C6EF5

#include "src/epp/impl/action.h"
#include "src/epp/domain/impl/domain_registration_obstruction.h"
#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"

#include <boost/optional.hpp>

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Domain {

struct CheckDomainLocalizedResponse
{
    const LocalizedSuccessResponse localized_success_response;
    const std::map<std::string, boost::optional<DomainLocalizedRegistrationObstruction> > domain_fqdn_to_domain_localized_registration_obstruction;

    CheckDomainLocalizedResponse(
        const LocalizedSuccessResponse& localized_success_response,
        const std::map<std::string, boost::optional<DomainLocalizedRegistrationObstruction> >& domain_fqdn_to_domain_localized_registration_obstruction)
    :
        localized_success_response(localized_success_response),
        domain_fqdn_to_domain_localized_registration_obstruction(domain_fqdn_to_domain_localized_registration_obstruction)
    { }
};

CheckDomainLocalizedResponse check_domain_localized(
        const std::set<std::string>& _domain_fqdns,
        unsigned long long _registrar_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle);

} // namespace Epp::Domain
} // namespace Epp

#endif
