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

#include "util/password_storage/base64.hh"

#include <cstring>

#include <algorithm>
#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace PasswordStorage {

namespace {

struct RawData
{
    std::shared_ptr<unsigned char> data;
    int size_of_data;
};

RawData get_raw_binary_data_from_base64_encoded_data(const std::string& src)
{
    typedef boost::archive::iterators::transform_width<
                    boost::archive::iterators::binary_from_base64<const char*>, 8, 6> Base64DecodeIterator;

    RawData result;
    unsigned int size = src.size();
    // Remove the padding characters, cf. https://svn.boost.org/trac/boost/ticket/5629
    if ((0 < size) && (src[size - 1] == '='))
    {
        --size;
        if ((0 < size) && (src[size - 1] == '='))
        {
            --size;
        }
        if (size == 0)
        {
            result.data = nullptr;
            result.size_of_data = 0;
            return result;
        }
    }
    std::ostringstream data;
    std::copy(Base64DecodeIterator(src.c_str()),
              Base64DecodeIterator(src.c_str() + size),
              std::ostream_iterator<char>(data));
    result.data.reset(new unsigned char[data.str().length()]);
    std::memcpy(result.data.get(), data.str().c_str(), data.str().length());
    result.size_of_data = data.str().length();
    return result;
}

std::string get_base64_encoded_data_from_raw_binary_data(
        std::shared_ptr<unsigned char> data,
        int size_of_data)
{
    typedef boost::archive::iterators::base64_from_binary<
                    boost::archive::iterators::transform_width<const unsigned char*, 6, 8>> Base64EncodeIterator;

    std::ostringstream result;
    std::copy(Base64EncodeIterator(data.get()),
              Base64EncodeIterator(data.get() + size_of_data),
              std::ostream_iterator<char>(result));

    switch (size_of_data % 3)
    {
    case 0:
        return result.str();
    case 1:
        return result.str() + "==";
    case 2:
        return result.str() + "=";
    }
    throw std::logic_error("unexpected result of modulo 3 operation");
}

}//namespace PasswordStorage::{anonymous}

BinaryData::BinaryData()
{ }

BinaryData::BinaryData(const PasswordStorage::Base64EncodedData& _src)
{
    const RawData raw = get_raw_binary_data_from_base64_encoded_data(_src.get_base64_encoded_data());
    raw_binary_data_ = raw.data;
    size_of_raw_binary_data_ = raw.size_of_data;
}

BinaryData::~BinaryData()
{ }

BinaryData BinaryData::from_raw_binary_data(
        std::shared_ptr<unsigned char> _raw_binary_data,
        int _size_of_raw_binary_data)
{
    BinaryData result;
    result.raw_binary_data_ = _raw_binary_data;
    result.size_of_raw_binary_data_ = _size_of_raw_binary_data;
    return result;
}

std::shared_ptr<unsigned char> BinaryData::get_raw_binary_data()const
{
    return raw_binary_data_;
}

int BinaryData::get_size_of_raw_binary_data()const
{
    return size_of_raw_binary_data_;
}

Base64EncodedData::Base64EncodedData()
{ }

Base64EncodedData::Base64EncodedData(const BinaryData& _src)
    : base64_encoded_data_(get_base64_encoded_data_from_raw_binary_data(
            _src.get_raw_binary_data(),
            _src.get_size_of_raw_binary_data()))
{ }

Base64EncodedData::~Base64EncodedData()
{ }

Base64EncodedData Base64EncodedData::from_base64_encoded_string(const std::string& _base64_encoded)
{
    Base64EncodedData result;
    result.base64_encoded_data_ = _base64_encoded;
    return result;
}

const std::string& PasswordStorage::Base64EncodedData::get_base64_encoded_data()const
{
    return base64_encoded_data_;
}

}//namespace PasswordStorage
