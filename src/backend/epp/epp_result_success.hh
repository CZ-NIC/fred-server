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
#ifndef EPP_RESULT_SUCCESS_HH_73487D8EB53943A9BC8B31CD2BA1949E
#define EPP_RESULT_SUCCESS_HH_73487D8EB53943A9BC8B31CD2BA1949E

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
