/*
 * Copyright (C) 2021  CZ.NIC, z. s. p. o.
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

#include "src/backend/epp/password.hh"

#include <stdexcept>
#include <utility>

namespace Epp {

Password::Password(std::string value)
    : value_{std::move(value)}
{ }

bool Password::is_empty() const noexcept
{
    return value_.empty();
}

} // namespace Epp

using namespace Epp;

bool Epp::operator==(const Password& lhs, const Password& rhs)
{
    if (lhs.is_empty() || rhs.is_empty())
    {
        throw std::invalid_argument{"both passwords must be set"};
    }
    return lhs.value_ == rhs.value_;
}

bool Epp::operator!=(const Password& lhs, const Password& rhs)
{
    return !(lhs == rhs);
}
