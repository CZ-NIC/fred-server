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

#ifndef EPP_RESULT_SUCCESS_H_0ACB22B7BCF44B7B9061D94364E51C2E
#define EPP_RESULT_SUCCESS_H_0ACB22B7BCF44B7B9061D94364E51C2E

#include "src/backend/epp/epp_result_code.hh"

namespace Epp {

/**
 * EppResultSuccess represents STD 69 EPP result of response
 * of positive completion reply (result with error code 1xxx).
 */
class EppResultSuccess
{

public:

    /** Every EppResultSuccess instance must have a valid EppResultCode::Success */
    explicit EppResultSuccess(EppResultCode::Success _epp_result_code)
        : epp_result_code_(_epp_result_code)
    { }

    virtual const char* c_str() const noexcept {
        return EppResultCode::c_str(epp_result_code_);
    }

    EppResultCode::Success epp_result_code() const {
        return epp_result_code_;
    }

protected:

    EppResultCode::Success epp_result_code_;

};

} // namespace Epp

#endif
