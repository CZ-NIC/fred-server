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

#include "src/util/password_storage/impl/pbkdf2.hh"
#include "src/util/password_storage/impl/check_equality.hh"
#include "src/util/password_storage/impl/generate_salt.hh"
#include "src/util/password_storage/base64.hh"
#include "src/util/enum_conversion.hh"

#include <openssl/evp.h>

#include <cstring>

#include <memory>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <string>

namespace PasswordStorage {
namespace Impl {

namespace {

const char pbkdf2_tag[] = "pbkdf2";

const EVP_MD* call_hash_init_function(Pbkdf2::HashFunction hash_function)
{
    switch (hash_function)
    {
        case Pbkdf2::HashFunction::sha512:
            return ::EVP_sha512();
    }
    throw std::logic_error("unsupported hash function");
}

int get_size_of_hash(Pbkdf2::HashFunction hash_function)
{
    switch (hash_function)
    {
        case Pbkdf2::HashFunction::sha512:
            return 512 / 8;
    }
    throw std::logic_error("unsupported hash function");
}

BinaryData pbkdf2_compute_hash(
        int number_of_iterations,
        Pbkdf2::HashFunction hash_function,
        const BinaryData& salt,
        const std::string& plaintext_password)
{
    const int size_of_hash = get_size_of_hash(hash_function);
    const auto hash_data = std::shared_ptr<unsigned char>(
            new unsigned char[size_of_hash],
            [](auto p) { delete[] p; });
    const int result = ::PKCS5_PBKDF2_HMAC(plaintext_password.c_str(),
                                           plaintext_password.length(),
                                           salt.get_raw_binary_data().get(),
                                           salt.get_size_of_raw_binary_data(),
                                           number_of_iterations,
                                           call_hash_init_function(hash_function),
                                           size_of_hash,
                                           hash_data.get());
    const bool hash_was_successfully_computed = result == 1;
    if (!hash_was_successfully_computed)
    {
        throw std::runtime_error("unable to compute hash");
    }
    return BinaryData::from_raw_binary_data(hash_data, size_of_hash);
}

std::string to_hash_function_tag(Pbkdf2::HashFunction hash_function)
{
    switch (hash_function)
    {
        case Pbkdf2::HashFunction::sha512:
            return "sha512";
    }
    throw std::logic_error("unsupported hash function");
}

Pbkdf2::HashFunction to_hash_function(const std::string& hash_function_tag)
{
    static const Pbkdf2::HashFunction dst_values[] =
            {
                Pbkdf2::HashFunction::sha512
            };
    return Conversion::Enums::inverse_transformation(hash_function_tag, dst_values, to_hash_function_tag);
}

} // namespace PasswordStorage::Impl::{anonymous}

Pbkdf2::Pbkdf2(
        HashFunction _hash_function,
        int _number_of_iterations)
    : hash_function_(_hash_function),
      number_of_iterations_(_number_of_iterations)
{
    if (_number_of_iterations <= 0)
    {
        throw std::runtime_error("unacceptable number of iterations");
    }
}

std::string Pbkdf2::get_prefix_and_parameters()const
{
    std::ostringstream data;
    data << "$" << pbkdf2_tag << "$" << to_hash_function_tag(hash_function_) << "$" << number_of_iterations_ << "$";
    return data.str();
}

PasswordData Pbkdf2::compute(const std::string& _plaintext_password)const
{
    const auto salt = generate_salt(get_size_of_hash(hash_function_));
    const auto hash = pbkdf2_compute_hash(
            number_of_iterations_,
            hash_function_,
            salt,
            _plaintext_password);
    const std::string data =
            this->get_prefix_and_parameters() +
            Base64EncodedData(salt).get_base64_encoded_data() + "$" +
            Base64EncodedData(hash).get_base64_encoded_data();
    return PasswordData::construct_from(data);
}

CheckResult Pbkdf2::check_password_correctness(
        const std::string& _plaintext_password,
        const PasswordData& _encrypted_password_data)//"$pbkdf2$<hashed_by>$<number_of_iterations>$<salt::base64>$<hash::base64>"
{
    static const std::regex delimiter("\\$");// prevents '$' special meaning in regex
    const std::string encrypted_password_data = _encrypted_password_data.get_value();
    auto items_itr = std::sregex_token_iterator(
            encrypted_password_data.begin(),
            encrypted_password_data.end(),
            delimiter,
            -1);
    const auto items_end = std::sregex_token_iterator();
    if ((items_itr == items_end) ||
        (items_itr->length() != 0))//std::sub_match does not have method empty()
    {
        throw std::runtime_error("corrupted data");
    }

    ++items_itr;
    if (items_itr == items_end)
    {
        throw std::runtime_error("corrupted data");
    }
    if (*items_itr != pbkdf2_tag)
    {
        return CheckResult::algorithm_does_not_fit;
    }

    ++items_itr;
    if (items_itr == items_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const HashFunction hash_function = to_hash_function(*items_itr);
    const int size_of_hash = get_size_of_hash(hash_function);

    ++items_itr;
    if (items_itr == items_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const int number_of_iterations = std::stoi(*items_itr, nullptr, 10);

    ++items_itr;
    if (items_itr == items_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const BinaryData salt(Base64EncodedData::from_base64_encoded_string(*items_itr));

    ++items_itr;
    if (items_itr == items_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const std::string base64_encoded_hash = *items_itr;
    ++items_itr;
    if (items_itr != items_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const BinaryData stored_hash(Base64EncodedData::from_base64_encoded_string(base64_encoded_hash));
    if (stored_hash.get_size_of_raw_binary_data() != size_of_hash)
    {
        throw std::runtime_error("corrupted data");
    }

    const BinaryData computed_hash = pbkdf2_compute_hash(
                number_of_iterations,
                hash_function,
                salt,
                _plaintext_password);

    check_equality(
            computed_hash.get_raw_binary_data().get(),
            stored_hash.get_raw_binary_data().get(),
            size_of_hash,
            throw_incorrect_password_exception_on_nonezero_value<unsigned char>);
    return CheckResult::password_is_correct;
}

} // namespace PasswordStorage::Impl
} // namespace PasswordStorage
