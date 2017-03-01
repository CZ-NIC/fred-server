/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef CHECK_DOMAIN_LOCALIZED_RESPONSE_H_EA8DA66B6D34477D9D35E925F78BE8FB
#define CHECK_DOMAIN_LOCALIZED_RESPONSE_H_EA8DA66B6D34477D9D35E925F78BE8FB

#include "src/epp/domain/impl/domain_registration_obstruction.h"
#include "src/epp/impl/epp_response_success_localized.h"

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
            domain_fqdn_to_domain_localized_registration_obstruction;


    CheckDomainLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const std::map<std::string,
                    boost::optional<DomainLocalizedRegistrationObstruction> >& _domain_fqdn_to_domain_localized_registration_obstruction)
        : epp_response_success_localized(_epp_response_success_localized),
          domain_fqdn_to_domain_localized_registration_obstruction(
                  _domain_fqdn_to_domain_localized_registration_obstruction)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
