/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef EPP_RESPONSE_SUCCESS_HH_C214DF4F95BD4112BFE1C84E5F5D619F
#define EPP_RESPONSE_SUCCESS_HH_C214DF4F95BD4112BFE1C84E5F5D619F

#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <exception>
#include <set>
#include <vector>

namespace Epp {

/**
 * EppResponseSuccess represents STD 69 EPP response
 * of positive completion reply.
 *
 * According to STD, it must include one results with
 * EPP error code representing successfull result (1xxx).
 */
class EppResponseSuccess : std::exception
{

public:

    /** Every EppResponse needs a valid EppResultSuccess */
    explicit EppResponseSuccess(const EppResultSuccess& _epp_result)
        : epp_result_(_epp_result)
    { }

    virtual ~EppResponseSuccess()
    { }

    const EppResultSuccess& epp_result() const {
        return epp_result_;
    }

    const char* what() const noexcept {
        return epp_result().c_str();
    }

private:

    EppResultSuccess epp_result_; ///< Every EppResponseSuccess instance must have one result.

};

} // namespace Epp

#endif
