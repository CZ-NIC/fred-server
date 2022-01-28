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
#ifndef PASSWORD_HH_82E5F2E4C257E300E9986A34FBCDCD8B//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define PASSWORD_HH_82E5F2E4C257E300E9986A34FBCDCD8B

#include <string>

namespace Epp {

class Password;
bool operator==(const Password& lhs, const Password& rhs);
bool operator!=(const Password& lhs, const Password& rhs);

class Password
{
public:
    explicit Password(std::string value = "");
    bool is_empty() const noexcept;
private:
    friend bool operator==(const Password& lhs, const Password& rhs);
    friend bool operator!=(const Password& lhs, const Password& rhs);
    std::string value_;
};

} // namespace Epp

#endif//PASSWORD_HH_82E5F2E4C257E300E9986A34FBCDCD8B
