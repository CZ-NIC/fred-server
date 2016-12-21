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

#ifndef EPP_EXCEPTION_H_31397E48DE5D4189A8C4A2E84F180DA9
#define EPP_EXCEPTION_H_31397E48DE5D4189A8C4A2E84F180DA9

#include "src/epp/impl/epp_result_code.h"

#include <exception>

namespace Epp {

class EppException : std::exception
{

public:

    /** Every EppException needs valid epp_result_code_ */
    EppException(EppResultCode::Enum _epp_response_result_code)
        : epp_response_result_code_(_epp_response_result_code)
    { }

    const char* what() const throw() {
        // for now, we have only one epp_response_result_code,
        // so its description is used to describe this exception
        return EppResultCode::to_string(epp_response_result_code_);
    }

    EppResultCode::Enum epp_response_result_code() const throw() {
        return epp_response_result_code_;
    }

protected:

    EppResultCode::Enum epp_response_result_code_;

};

} // namespace Epp

#endif
