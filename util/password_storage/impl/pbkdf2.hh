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

#ifndef PBKDF2_HH_B6647B465A59BDD3713C0F996B1D90A2//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PBKDF2_HH_B6647B465A59BDD3713C0F996B1D90A2

#include "util/password_storage/base64.hh"
#include "util/password_storage/password_data.hh"
#include "util/password_storage/impl/check_result.hh"

#include <cstring>

#include <memory>
#include <string>

namespace PasswordStorage {
namespace Impl {

//for more information see:
//https://www.openssl.org/docs/man1.1.0/crypto/PKCS5_PBKDF2_HMAC.html
//https://tools.ietf.org/html/rfc2898
//https://crackstation.net/hashing-security.htm
class Pbkdf2
{
public:
    enum class HashFunction
    {
        sha512,
    };
    Pbkdf2(HashFunction _hash_function,
           int _number_of_iterations);
    std::string get_prefix_and_parameters()const;
    PasswordData compute(const std::string& _plaintext_password)const;
    static CheckResult check_password_correctness(
            const std::string& _plaintext_password,
            const PasswordData& _encrypted_password_data);
private:
    const HashFunction hash_function_;
    const int number_of_iterations_;
};

template <Pbkdf2::HashFunction hash_function, int number_of_iterations>
struct AlgPbkdf2
{
    static PasswordData encrypt_password(
            const std::string& _plaintext_password)
    {
        return Pbkdf2(
                hash_function,
                number_of_iterations).compute(_plaintext_password);
    }
    static std::string get_prefix()
    {
        static const auto my_prefix = Pbkdf2(
                hash_function,
                number_of_iterations).get_prefix_and_parameters();
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

}//namespace PasswordStorage::Impl
}//namespace PasswordStorage

#endif//PBKDF2_HH_B6647B465A59BDD3713C0F996B1D90A2
