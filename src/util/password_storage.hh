/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PASSWORD_STORAGE_HH_D62FBCFF6DD9A9CA715F3049D6376FF0//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PASSWORD_STORAGE_HH_D62FBCFF6DD9A9CA715F3049D6376FF0

#include "src/util/password_storage/password_data.hh"

#include <string>
#include <stdexcept>

namespace PasswordStorage {

PasswordData encrypt_password_by_preferred_method(
        const std::string& plaintext_password);

bool is_encrypted_by_preferred_method(
        const PasswordData& encrypted_password_data);

struct IncorrectPassword:std::runtime_error
{
    IncorrectPassword():std::runtime_error("incorrect password") { }
};

void check_password(
        const std::string& plaintext_password,
        const PasswordData& encrypted_password_data);

} // namespace PasswordStorage

#endif//PASSWORD_STORAGE_HH_D62FBCFF6DD9A9CA715F3049D6376FF0
