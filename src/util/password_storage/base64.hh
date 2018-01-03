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

#ifndef BASE64_HH_B846DD2F17074D62AE03C4FAA9A1DE3F
#define BASE64_HH_B846DD2F17074D62AE03C4FAA9A1DE3F

#include <memory>
#include <string>

namespace PasswordStorage {

class BinaryData;
class Base64EncodedData;

class BinaryData
{
public:
    BinaryData();
    explicit BinaryData(const Base64EncodedData& _src);
    ~BinaryData();
    static BinaryData from_raw_binary_data(
            std::shared_ptr<unsigned char> _raw_binary_data,
            int _size_of_raw_binary_data);
    std::shared_ptr<unsigned char> get_raw_binary_data()const;
    int get_size_of_raw_binary_data()const;
private:
    std::shared_ptr<unsigned char> raw_binary_data_;
    int size_of_raw_binary_data_;
};

class Base64EncodedData
{
public:
    Base64EncodedData();
    explicit Base64EncodedData(const BinaryData& _src);
    ~Base64EncodedData();
    static Base64EncodedData from_base64_encoded_string(const std::string& _base64_encoded);
    const std::string& get_base64_encoded_data()const;
private:
    std::string base64_encoded_data_;
};

} // namespace PasswordStorage

#endif
