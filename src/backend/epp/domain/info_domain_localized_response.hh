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
#ifndef INFO_DOMAIN_LOCALIZED_RESPONSE_HH_9FD2467249424EEFA56FD7EBBB0803BB
#define INFO_DOMAIN_LOCALIZED_RESPONSE_HH_9FD2467249424EEFA56FD7EBBB0803BB

#include "src/backend/epp/domain/info_domain_localized_output_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"

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
