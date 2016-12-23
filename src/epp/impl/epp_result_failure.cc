#include "src/epp/impl/epp_result_failure.h"

namespace Epp {

bool operator < (const Epp::EppResultFailure& _lhs, const Epp::EppResultFailure& _rhs)
{
    return static_cast<int>(_lhs.epp_result_code()) < static_cast<int>(_rhs.epp_result_code());
}

} // namespace Epp

