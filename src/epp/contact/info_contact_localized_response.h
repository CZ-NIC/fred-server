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

#ifndef INFO_CONTACT_LOCALIZED_RESPONSE_H_B1A92D8D6FC04366A0327A5E73AB558C
#define INFO_CONTACT_LOCALIZED_RESPONSE_H_B1A92D8D6FC04366A0327A5E73AB558C

#include "src/epp/contact/info_contact_localized_output_data.h"
#include "src/epp/epp_response_success_localized.h"

namespace Epp {
namespace Contact {

struct InfoContactLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const InfoContactLocalizedOutputData data;


    InfoContactLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const InfoContactLocalizedOutputData& _data)
        : epp_response_success_localized(_epp_response_success_localized),
          data(_data)
    {
    }

};

} // namespace Epp::Contact
} // namespace Epp

#endif
