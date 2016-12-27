#include "src/epp/impl/epp_result_failure.h"

namespace Epp {

bool operator < (const Epp::EppResultFailure& _lhs, const Epp::EppResultFailure& _rhs)
{
    return static_cast<int>(_lhs.epp_result_code()) < static_cast<int>(_rhs.epp_result_code());
}

bool has_extended_error(const Epp::EppResultFailure& _epp_result_failure, const Epp::EppExtendedError& _epp_extended_error) {
    return _epp_result_failure.extended_errors()->find(_epp_extended_error)
           !=
           _epp_result_failure.extended_errors()->end();
}

bool has_extended_error_with_param_reason(const Epp::EppResultFailure& _epp_result_failure, const Epp::Param::Enum& _param, const Epp::Reason::Enum& _reason) {
    const Epp::EppExtendedError epp_extended_error = 
                   Epp::EppExtendedError::of_scalar_parameter(
                           _param,
                           _reason);

    return has_extended_error(_epp_result_failure, epp_extended_error);
}

} // namespace Epp

