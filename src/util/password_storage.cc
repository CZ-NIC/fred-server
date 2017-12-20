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

#include "src/util/password_storage.hh"

#include "src/util/password_storage/impl/pbkdf2.hh"
#include "src/util/password_storage/impl/plaintext.hh"

#include <stdexcept>
#include <string>

namespace PasswordStorage {

namespace {

template <typename A0, typename ...An>
struct PasswordCorrectness
{
    static void check(
            const std::string& plaintext_password,
            const PasswordData& encrypted_password)
    {
        switch (A0::check_password_correctness(plaintext_password, encrypted_password))
        {
            case Impl::CheckResult::password_is_correct:
                return;
            case Impl::CheckResult::algorithm_does_not_fit:
                PasswordCorrectness<An...>::check(plaintext_password, encrypted_password);
                return;
        }
        throw std::logic_error("unexpected CheckResult value");
    }
};

template <typename A0>
struct PasswordCorrectness<A0>
{
    static void check(
            const std::string& plaintext_password,
            const PasswordData& encrypted_password)
    {
        switch (A0::check_password_correctness(plaintext_password, encrypted_password))
        {
            case Impl::CheckResult::password_is_correct:
                return;
            case Impl::CheckResult::algorithm_does_not_fit:
                throw std::runtime_error("password encrypted by unsupported algorithm");
        }
        throw std::logic_error("unexpected CheckResult value");
    }
};

const int cpu_cost = 16;
const int number_of_iterations = 0x01 << cpu_cost;
typedef Impl::AlgPbkdf2<Impl::Pbkdf2::HashFunction::sha512, number_of_iterations> PreferredHashAlgorithm;

} // namespace PasswordStorage::{anonymous}

PasswordData encrypt_password_by_preferred_method(
        const std::string& plaintext_password)
{
    return PreferredHashAlgorithm::encrypt_password(plaintext_password);
}

bool is_encrypted_by_preferred_method(
        const PasswordData& encrypted_password_data)
{
    return PreferredHashAlgorithm::is_encrypted_by_me(encrypted_password_data);
}

void check_password(
        const std::string& plaintext_password,
        const PasswordData& encrypted_password_data)
{
    PasswordCorrectness<Impl::Pbkdf2,
                        Impl::Plaintext>::check(plaintext_password, encrypted_password_data);
}

} // namespace PasswordStorage
