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

#ifndef INFO_NSSET_LOCALIZED_RESPONSE_H_D099C8B7E81A43C2BEB4E7193E57E8FC
#define INFO_NSSET_LOCALIZED_RESPONSE_H_D099C8B7E81A43C2BEB4E7193E57E8FC

#include "src/epp/nsset/impl/info_nsset_localized_output_data.h"
#include "src/epp/impl/epp_response_success_localized.h"

namespace Epp {
namespace Nsset {

struct InfoNssetLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const InfoNssetLocalizedOutputData data;

    InfoNssetLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response_success_localized,
        const InfoNssetLocalizedOutputData& _data)
    :
        epp_response_success_localized(_epp_response_success_localized),
        data(_data)
    { }
};

} // namespace Epp::Nsset
} // namespace Epp

#endif
