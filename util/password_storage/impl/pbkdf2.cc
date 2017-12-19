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

#include "util/password_storage/impl/pbkdf2.hh"
#include "util/password_storage/impl/check_equality.hh"
#include "util/password_storage/impl/generate_salt.hh"
#include "util/password_storage/base64.hh"
#include "util/enum_conversion.h"

#include <openssl/evp.h>

#include <cstring>

#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>

namespace PasswordStorage {
namespace Impl {

namespace {

const char pbkdf2_prefix[] = "$pbkdf2$";

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

}//namespace PasswordStorage::Impl::{anonymous}

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
    data << pbkdf2_prefix << to_hash_function_tag(hash_function_) << "$" << number_of_iterations_ << "$";
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
    const char* const alg_tag_start = _encrypted_password_data.get_value().c_str();
    const bool prefix_matches = std::strncmp(alg_tag_start, pbkdf2_prefix, std::strlen(pbkdf2_prefix)) == 0;
    if (!prefix_matches)
    {
        return CheckResult::algorithm_does_not_fit;
    }

    const char* data = alg_tag_start + std::strlen(pbkdf2_prefix);
    const char* const data_end = alg_tag_start + _encrypted_password_data.get_value().length();

    const char* const hash_function_tag_start = data;
    while ((data != data_end) && (*data != '$'))
    {
        ++data;
    }
    if (data == data_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const HashFunction hash_function = to_hash_function(std::string(hash_function_tag_start, data - hash_function_tag_start));
    const int size_of_hash = get_size_of_hash(hash_function);

    ++data;
    if (data == data_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const char* const number_of_iterations_start = data;
    while ((data != data_end) && (*data != '$'))
    {
        ++data;
    }
    if (data == data_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const int number_of_iterations = std::stoi(
            std::string(number_of_iterations_start, data - number_of_iterations_start), nullptr, 10);

    ++data;
    if (data == data_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const char* const salt_start = data;
    while ((data != data_end) && (*data != '$'))
    {
        ++data;
    }
    if (data == data_end)
    {
        throw std::runtime_error("corrupted data");
    }
    const BinaryData salt(Base64EncodedData::from_base64_encoded_string(std::string(salt_start, data - salt_start)));

    ++data;
    const char* const hash_start = data;
    const BinaryData stored_hash(Base64EncodedData::from_base64_encoded_string(std::string(hash_start, data_end - hash_start)));
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

}//namespace PasswordStorage::Impl
}//namespace PasswordStorage
