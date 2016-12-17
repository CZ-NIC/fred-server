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

/**
 *  @file
 */

#ifndef SRC_EPP_LOCALIZED_RESPONSE_988764121
#define SRC_EPP_LOCALIZED_RESPONSE_988764121

#include "src/epp/impl/param.h"
#include "src/epp/impl/response.h"

#include <set>
#include <string>

namespace Epp {

    struct LocalizedSuccessResponse {
        const Response::Enum response;
        const std::string localized_response_description;

        LocalizedSuccessResponse(
            const Response::Enum& _response,
            const std::string& _localized_response_description
        ) :
            response(_response),
            localized_response_description(_localized_response_description)
        { }
    };

    struct LocalizedError {
        const Param::Enum param;
        const unsigned short position;
        const std::string localized_reason_description;

        LocalizedError(
            Param::Enum _param,
            unsigned short _position,
            const std::string& _localized_reason_description
        ) :
            param(_param),
            position(_position),
            localized_reason_description(_localized_reason_description)
        { }
    };
}

/* Only intended for std::set usage - ordering definition is irrelevant. */

namespace std {
    template<> struct less<Epp::LocalizedError> {
        bool operator()(const Epp::LocalizedError& lhs, const Epp::LocalizedError& rhs) const {
            /* order by param, position, reason */

            if(      static_cast<int>(lhs.param) < static_cast<int>(rhs.param) ) { return true; }
            else if( static_cast<int>(lhs.param) > static_cast<int>(rhs.param) ) { return false; }
            else {

                if(      lhs.position < rhs.position ) { return true; }
                else if( lhs.position > rhs.position ) { return false; }
                else {

                    if( lhs.localized_reason_description < rhs.localized_reason_description ) { return true; }
                    else { return false; }
                }
            }
        }
    };
};

namespace Epp {
    struct LocalizedFailResponse {
        const Response::Enum response;
        const std::string localized_response_description;
        const std::set<LocalizedError> localized_errors;

        LocalizedFailResponse(
            const Response::Enum& _response,
            const std::string& _localized_response_description,
            const std::set<LocalizedError>& _localized_errors
        ) :
            response(_response),
            localized_response_description(_localized_response_description),
            localized_errors(_localized_errors)
        { }
    };
}

#endif
