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

#include "util/password_storage/impl/check_equality.hh"

#include <stdexcept>

namespace PasswordStorage {
namespace Impl {

template <typename T>
void check_equality(
        const T* lhs_ptr,
        const T* rhs_ptr,
        int items,
        void (*throw_exception_on_nonzero)(T))
{
    T result = 0;
    for (int idx = 0; idx < items; ++idx)
    {
        result |= (lhs_ptr[idx] ^ rhs_ptr[idx]);
    }
    throw_exception_on_nonzero(result);
    if (result != static_cast<T>(0))
    {
        struct NotEqual:std::logic_error
        {
            NotEqual():std::logic_error("check_equality failed") { }
        };
        throw NotEqual();
    }
}

template void check_equality<unsigned char>(
        const unsigned char*,
        const unsigned char*,
        int,
        void (*)(unsigned char));

}//namespace PasswordStorage::Impl
}//namespace PasswordStorage
