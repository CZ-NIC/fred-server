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

#ifndef EPP_RESPONSE_FAILURE_HH_E7B1384E3ABF48A4AA514AF77FD6F841
#define EPP_RESPONSE_FAILURE_HH_E7B1384E3ABF48A4AA514AF77FD6F841

#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <exception>
#include <set>
#include <vector>

namespace Epp {

/**
 * EppResponseFailure represents STD 69 EPP error response
 * of negative completion reply.
 *
 * According to STD, it may include multiple results with
 * EPP error code representing unsuccessful result (2xxx).
 */
class EppResponseFailure : public std::exception
{

public:

    /** Every EppResponse needs a valid EppResultFailure */
    explicit EppResponseFailure(const EppResultFailure& _epp_result)
    {
        epp_results_.push_back(_epp_result);
    }

    virtual ~EppResponseFailure()
    { }

    EppResponseFailure& add(const EppResultFailure& _epp_result)
    {
        epp_results_.push_back(_epp_result);
        return *this;
    }

    EppResponseFailure& add(const std::set<EppResultFailure>& _epp_results)
    {
        epp_results_.insert(epp_results_.begin(), _epp_results.begin(), _epp_results.end());
        return *this;
    }

    const std::vector<EppResultFailure>& epp_results() const {
        return epp_results_;
    }

    const EppResultFailure& epp_result() const {
        return epp_results_.front();
    }

    /**
     * As for now, we have only one epp_result, so its
     * description is used to describe this exception.
     */
    const char* what()const noexcept override
    {
        return epp_result().c_str();
    }

private:

    std::vector<EppResultFailure> epp_results_; ///< Every EppResponseFailure instance must have at least one result.

};

} // namespace Epp

#endif
