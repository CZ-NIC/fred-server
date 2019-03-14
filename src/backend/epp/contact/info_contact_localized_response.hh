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
#ifndef INFO_CONTACT_LOCALIZED_RESPONSE_HH_C899981C505F446CB61D626044045C11
#define INFO_CONTACT_LOCALIZED_RESPONSE_HH_C899981C505F446CB61D626044045C11

#include "src/backend/epp/contact/info_contact_localized_output_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"

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
