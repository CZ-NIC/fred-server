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

#ifndef EPP_RESULT_FAILURE_H_05F2D2D2FCCD46019CD7B7E1BBF33E09
#define EPP_RESULT_FAILURE_H_05F2D2D2FCCD46019CD7B7E1BBF33E09

#include "src/epp/impl/epp_extended_error.h"
#include "src/epp/impl/epp_result_code.h"

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
    EppResultFailure(EppResultCode::Failure _epp_result_code)
        : epp_result_code_(_epp_result_code)
    { }

    //EppResultFailure& add_error(const EppExtendedError& _error)
    //{
    //    if(!errors_) {
    //        errors_ = std::set<EppError>();
    //    }
    //    errors_->insert(_error);
    //    return *this;
    //}

    //EppResultFailure& add_errors_(const std::set<EppExtendedError>& _errors)
    //{
    //    if(!errors_) {
    //        errors_ = std::set<EppError>();
    //    }
    //    errors_->insert(_errors.begin(), _errors.end());
    //    return *this;
    //}

    EppResultFailure& add_extended_error(const EppExtendedError& _error)
    {
        if(!extended_errors_) {
            extended_errors_ = std::set<EppExtendedError>();
        }
        extended_errors_->insert(_error);
        return *this;
    }

    EppResultFailure& add_extended_errors(const std::set<EppExtendedError>& _errors)
    {
        if(!extended_errors_) {
            extended_errors_ = std::set<EppExtendedError>();
        }
        extended_errors_->insert(_errors.begin(), _errors.end());
        return *this;
    }

    //const boost::optional<std::set<EppExtendedError> >& errors() const
    //{
    //    return errors_;
    //}

    const boost::optional<std::set<EppExtendedError> >& extended_errors() const
    {
        return extended_errors_;
    }

    virtual const char* c_str() const throw() {
        return EppResultCode::c_str(epp_result_code_);
    }

    EppResultCode::Failure epp_result_code() const {
        return epp_result_code_;
    }

    // only intended for std::set usage - ordering definition is irrelevant
    friend bool operator < (const Epp::EppResultFailure& lhs, const Epp::EppResultFailure& rhs);

protected:

    EppResultCode::Failure epp_result_code_;

private:

    //boost::optional<std::set<EppError> > errors_; ///< represents STD 69 response result's value
    boost::optional<std::set<EppExtendedError> > extended_errors_; ///< represents STD 69 response result's extValue

};

} // namespace Epp

#endif
