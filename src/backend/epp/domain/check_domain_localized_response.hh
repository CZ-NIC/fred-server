/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef CHECK_DOMAIN_LOCALIZED_RESPONSE_HH_AD34C8B010364210B98850054DBD69BF
#define CHECK_DOMAIN_LOCALIZED_RESPONSE_HH_AD34C8B010364210B98850054DBD69BF

#include "src/backend/epp/domain/domain_registration_obstruction.hh"
#include "src/backend/epp/epp_response_success_localized.hh"

#include <boost/optional.hpp>

#include <map>
#include <string>

namespace Epp {
namespace Domain {

struct CheckDomainLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const std::map<std::string,
            boost::optional<DomainLocalizedRegistrationObstruction> >
            fqdn_to_domain_localized_registration_obstruction;


    CheckDomainLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const std::map<std::string,
                    boost::optional<DomainLocalizedRegistrationObstruction> >& _fqdn_to_domain_localized_registration_obstruction)
        : epp_response_success_localized(_epp_response_success_localized),
          fqdn_to_domain_localized_registration_obstruction(
                  _fqdn_to_domain_localized_registration_obstruction)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
