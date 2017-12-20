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

#ifndef GENERATE_SALT_HH_A6DF5CEF9518CEA33F18124D0F03344C//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define GENERATE_SALT_HH_A6DF5CEF9518CEA33F18124D0F03344C

#include "src/util/password_storage/base64.hh"

#include <random>

namespace PasswordStorage {
namespace Impl {

inline BinaryData generate_salt(int bytes)
{
    std::random_device source_of_randomness;
    std::mt19937_64 generate_random_number(source_of_randomness());
    std::shared_ptr<unsigned char> salt_data(new unsigned char[bytes], [](auto p) { delete[] p; });
    {
        unsigned char* data = salt_data.get();
        const unsigned char* const data_end = data + bytes;
        while (data != data_end)
        {
            auto random_value = generate_random_number();
            for (unsigned cnt = 0; cnt < sizeof(random_value); ++cnt)
            {
                *data = random_value & 0xff;
                ++data;
                if (data == data_end)
                {
                    break;
                }
                random_value >>= 8;
            }
        }
    }
    return BinaryData::from_raw_binary_data(salt_data, bytes);
}

} // namespace PasswordStorage::Impl
} // namespace PasswordStorage

#endif//GENERATE_SALT_HH_A6DF5CEF9518CEA33F18124D0F03344C
