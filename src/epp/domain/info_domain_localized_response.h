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

#ifndef INFO_DOMAIN_LOCALIZED_RESPONSE_H_63645EF9A6324C078BE9A40618EF88BA
#define INFO_DOMAIN_LOCALIZED_RESPONSE_H_63645EF9A6324C078BE9A40618EF88BA

#include "src/epp/domain/impl/info_domain_localized_output_data.h"
#include "src/epp/epp_response_success_localized.h"

namespace Epp {
namespace Domain {

struct InfoDomainLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const InfoDomainLocalizedOutputData info_domain_localized_output_data;


    InfoDomainLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const InfoDomainLocalizedOutputData& _info_domain_localized_output_data)
        : epp_response_success_localized(_epp_response_success_localized),
          info_domain_localized_output_data(_info_domain_localized_output_data)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
