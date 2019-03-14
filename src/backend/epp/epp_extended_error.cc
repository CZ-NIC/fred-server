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
#include "src/backend/epp/epp_extended_error.hh"

namespace Epp {

bool operator < (const Epp::EppExtendedError& _lhs, const Epp::EppExtendedError& _rhs)
{
    return (_lhs.param_ < _rhs.param_) ||
           ((_lhs.param_ == _rhs.param_) && (_lhs.position_ < _rhs.position_)) ||
           ((_lhs.param_ == _rhs.param_) && (_lhs.position_ == _rhs.position_) && (_lhs.reason_ < _rhs.reason_));
}

} // namespace Epp
