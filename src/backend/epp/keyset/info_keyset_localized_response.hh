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
#ifndef INFO_KEYSET_LOCALIZED_RESPONSE_HH_2BC90DE5F2EC41D4BD5A72512AE17A27
#define INFO_KEYSET_LOCALIZED_RESPONSE_HH_2BC90DE5F2EC41D4BD5A72512AE17A27

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/keyset/info_keyset_localized_output_data.hh"

namespace Epp {
namespace Keyset {

struct InfoKeysetLocalizedResponse
{
    EppResponseSuccessLocalized epp_response_success_localized;
    InfoKeysetLocalizedOutputData data;


    InfoKeysetLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const InfoKeysetLocalizedOutputData& _data)
        : epp_response_success_localized(_epp_response_success_localized),
          data(_data)
    {
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
