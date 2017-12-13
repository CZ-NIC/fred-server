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

#include "util/password_storage/impl/plaintext.hh"
#include "util/password_storage/impl/check_equality.hh"
#include "util/password_storage/base64.hh"

#include <cstring>

#include <string>

namespace PasswordStorage {
namespace Impl {

namespace {

const char plaintext_prefix[] = "$plaintext$";

}//namespace PasswordStorage::Impl::{anonymous}

Plaintext::Plaintext()
{
}

std::string Plaintext::get_prefix_and_parameters()const
{
    return plaintext_prefix;
}

PasswordData Plaintext::compute(const std::string& _plaintext_password)const
{
    const std::string data =
            this->get_prefix_and_parameters() + _plaintext_password;
    return PasswordData::construct_from(data);
}

CheckResult Plaintext::check_password_correctness(
        const std::string& _plaintext_password,
        const PasswordData& _encrypted_password_data)//"$plaintext$<plaintext_password>"
{
    const char* const alg_tag_start = _encrypted_password_data.get_value().c_str();
    const bool prefix_matches = std::strncmp(alg_tag_start, plaintext_prefix, std::strlen(plaintext_prefix)) == 0;
    if (!prefix_matches)
    {
        return CheckResult::algorithm_does_not_fit;
    }

    const std::string stored_password = alg_tag_start + std::strlen(plaintext_prefix);
    throw_incorrect_password_exception_on_nonezero_value(_plaintext_password == stored_password);
    return CheckResult::password_is_correct;
}

}//namespace PasswordStorage::Impl
}//namespace PasswordStorage
