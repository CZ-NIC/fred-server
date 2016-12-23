#include "src/epp/impl/epp_extended_error.h"

namespace Epp {

bool operator < (const Epp::EppExtendedError& _lhs, const Epp::EppExtendedError& _rhs)
{
    return (_lhs.param_ < _rhs.param_) ||
           ((_lhs.param_ == _rhs.param_) && (_lhs.position_ < _rhs.position_)) ||
           ((_lhs.param_ == _rhs.param_) && (_lhs.position_ == _rhs.position_) && (_lhs.reason_ < _rhs.reason_));
}

} // namespace Epp
