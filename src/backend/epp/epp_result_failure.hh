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

#ifndef EPP_RESULT_FAILURE_HH_0578C488286448C59576AB85243B21D8
#define EPP_RESULT_FAILURE_HH_0578C488286448C59576AB85243B21D8

#include "src/backend/epp/epp_extended_error.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <set>

namespace Epp {

/**
 * EppResultFailure represents STD 69 EPP error result of response
 * of negative completion reply (result with error code 2xxx).
 */
class EppResultFailure
{

public:

    /** Every EppResultFailure instance must have a valid EppResultCode::Failure */
    explicit EppResultFailure(EppResultCode::Failure _epp_result_code)
        : epp_result_code_(_epp_result_code)
    { }

    EppResultFailure& add_extended_error(const EppExtendedError& _error)
    {
        if (!extended_errors_) {
            extended_errors_ = std::set<EppExtendedError>();
        }
        extended_errors_->insert(_error);
        return *this;
    }

    EppResultFailure& add_extended_errors(const std::set<EppExtendedError>& _errors)
    {
        if (!extended_errors_) {
            extended_errors_ = std::set<EppExtendedError>();
        }
        extended_errors_->insert(_errors.begin(), _errors.end());
        return *this;
    }

    const boost::optional<std::set<EppExtendedError> >& extended_errors() const
    {
        return extended_errors_;
    }

    const bool empty() const
    {
        return !extended_errors_ || extended_errors_->empty();
    }

    virtual const char* c_str() const noexcept {
        return EppResultCode::c_str(epp_result_code_);
    }

    EppResultCode::Failure epp_result_code() const {
        return epp_result_code_;
    }

    // only intended for std::set usage - ordering definition is irrelevant
    friend bool operator < (const EppResultFailure& lhs, const EppResultFailure& rhs);

protected:

    EppResultCode::Failure epp_result_code_;

private:

    boost::optional<std::set<EppExtendedError> > extended_errors_; ///< represents STD 69 response result's extValue

};

bool has_extended_error(
        const EppResultFailure& _epp_result_failure,
        const EppExtendedError& _epp_extended_error);

bool has_extended_error_with_param_reason(
        const EppResultFailure& _epp_result_failure,
        Param::Enum _param,
        Reason::Enum _reason);

bool has_extended_error_with_param_index_reason(
        const EppResultFailure& _epp_result_failure,
        Param::Enum _param,
        unsigned short _index,
        Reason::Enum _reason);

std::set<EppExtendedError> extended_errors_with_param_reason(
        const EppResultFailure& _epp_result_failure,
        Param::Enum _param,
        Reason::Enum _reason);

} // namespace Epp

#endif
