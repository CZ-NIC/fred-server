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

#ifndef CHECK_KEYSET_LOCALIZED_RESPONSE_H_78FE1075D188416A874A5A8AF98971CD
#define CHECK_KEYSET_LOCALIZED_RESPONSE_H_78FE1075D188416A874A5A8AF98971CD

#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/keyset/impl/keyset_handle_registration_obstruction.h"
#include "util/db/nullable.h"

#include <map>
#include <string>

namespace Epp {
namespace Keyset {

struct CheckKeysetLocalizedResponse
{
    struct Result
    {
        KeysetHandleRegistrationObstruction::Enum state;
        std::string description;

        Result()
        { }

        Result(
                const KeysetHandleRegistrationObstruction::Enum _state,
                const std::string& _description)
            : state(_state),
              description(_description)
        { }
    };

    typedef std::map<std::string, Nullable<Result> > Results;

    CheckKeysetLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const Results& _results)
        : epp_response_success_localized(_epp_response_success_localized),
          results(_results)
    { }

    EppResponseSuccessLocalized epp_response_success_localized;
    Results results;
};

} // namespace Epp::Keyset
} // namespace Epp

#endif
