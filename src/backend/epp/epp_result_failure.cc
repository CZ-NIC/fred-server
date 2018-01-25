/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/epp_result_failure.hh"

namespace Epp {

bool operator < (const EppResultFailure& _lhs, const EppResultFailure& _rhs)
{
    return static_cast<int>(_lhs.epp_result_code()) < static_cast<int>(_rhs.epp_result_code());
}

bool has_extended_error(
        const EppResultFailure& _epp_result_failure,
        const EppExtendedError& _epp_extended_error)
{
    return _epp_result_failure.extended_errors() &&
           (_epp_result_failure.extended_errors()->find(_epp_extended_error) !=
                   _epp_result_failure.extended_errors()->end());
}

bool has_extended_error_with_param_reason(
        const EppResultFailure& _epp_result_failure,
        const Param::Enum _param,
        const Reason::Enum _reason)
{
    const EppExtendedError epp_extended_error =
                   EppExtendedError::of_scalar_parameter(
                           _param,
                           _reason);

    return has_extended_error(_epp_result_failure, epp_extended_error);
}

bool has_extended_error_with_param_index_reason(
        const EppResultFailure& _epp_result_failure,
        const Param::Enum _param,
        unsigned short _index,
        const Reason::Enum _reason)
{
    const EppExtendedError epp_extended_error =
                   EppExtendedError::of_vector_parameter(
                           _param,
                           _index,
                           _reason);

    return has_extended_error(_epp_result_failure, epp_extended_error);
}

std::set<EppExtendedError> extended_errors_with_param_reason(
        const EppResultFailure& _epp_result_failure,
        const Param::Enum _param,
        const Reason::Enum _reason)
{
    std::set<EppExtendedError> extended_errors_with_param_reason;
    if (_epp_result_failure.extended_errors()) {
        for (std::set<EppExtendedError>::const_iterator epp_extended_error = _epp_result_failure.extended_errors()->begin();
             epp_extended_error != _epp_result_failure.extended_errors()->end();
             ++epp_extended_error)
        {
            if (epp_extended_error->param() == _param && epp_extended_error->reason() == _reason) {
                extended_errors_with_param_reason.insert(*epp_extended_error);
            }
        }
    }
    return extended_errors_with_param_reason;
}

} // namespace Epp
