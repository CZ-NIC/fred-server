/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#ifndef INFO_NSSET_LOCALIZED_RESPONSE_HH_4E047040FDAF4E57874FF25023005363
#define INFO_NSSET_LOCALIZED_RESPONSE_HH_4E047040FDAF4E57874FF25023005363

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/nsset/info_nsset_localized_output_data.hh"

namespace Epp {
namespace Nsset {

struct InfoNssetLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const InfoNssetLocalizedOutputData data;


    InfoNssetLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const InfoNssetLocalizedOutputData& _data)
        : epp_response_success_localized(_epp_response_success_localized),
          data(_data)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
