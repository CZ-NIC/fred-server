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

#ifndef PLAINTEXT_HH_94F719E5DC354A5C88FDDEA555CB3E21
#define PLAINTEXT_HH_94F719E5DC354A5C88FDDEA555CB3E21

#include "src/util/password_storage/base64.hh"
#include "src/util/password_storage/password_data.hh"
#include "src/util/password_storage/impl/check_result.hh"

#include <cstring>

#include <memory>
#include <string>

namespace PasswordStorage {
namespace Impl {

class Plaintext
{
public:
    Plaintext();
    std::string get_prefix_and_parameters()const;
    PasswordData compute(const std::string& _plaintext_password)const;
    static CheckResult check_password_correctness(
            const std::string& _plaintext_password,
            const PasswordData& _encrypted_password_data);
};

struct AlgPlaintext
{
    static PasswordData encrypt_password(
            const std::string& _plaintext_password)
    {
        return Plaintext().compute(_plaintext_password);
    }
    static std::string get_prefix()
    {
        static const auto my_prefix = Plaintext().get_prefix_and_parameters();
        return my_prefix;
    }
    static bool is_encrypted_by_me(
            const PasswordData& encrypted_password_data)
    {
        static const auto my_prefix = get_prefix();
        const bool encrypted_password_starts_with_my_prefix =
                std::strncmp(encrypted_password_data.get_value().c_str(),
                             my_prefix.c_str(),
                             my_prefix.length()) == 0;
        return encrypted_password_starts_with_my_prefix;
    }
};

} // namespace PasswordStorage::Impl
} // namespace PasswordStorage

#endif
