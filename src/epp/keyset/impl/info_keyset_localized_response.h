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

#ifndef INFO_KEYSET_LOCALIZED_RESPONSE_H_A13CF66F8D7442B98DCD1BF959DEFC6C
#define INFO_KEYSET_LOCALIZED_RESPONSE_H_A13CF66F8D7442B98DCD1BF959DEFC6C

#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/keyset/impl/info_keyset_localized_output_data.h"

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
