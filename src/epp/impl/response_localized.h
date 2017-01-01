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

#ifndef RESPONSE_LOCALIZED_H_A5D5FF63BD4A4ECFADECA24EE8BDA0DC
#define RESPONSE_LOCALIZED_H_A5D5FF63BD4A4ECFADECA24EE8BDA0DC

#include "src/epp/impl/param.h"
#include "src/epp/impl/epp_result_code.h"

#include <set>
#include <string>

namespace Epp {

struct LocalizedSuccessResponse
{
    const EppResultCode::Success epp_result_code;
    const std::string localized_response_description;

    LocalizedSuccessResponse(
        const EppResultCode::Success& _epp_result_code,
        const std::string& _localized_response_description)
    :
        epp_result_code(_epp_result_code),
        localized_response_description(_localized_response_description)
    { }
};

} // namespace Epp

#endif
