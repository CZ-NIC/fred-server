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

#ifndef CHECK_EQUALITY_HH_19FD627794361D94A2AEA40661216EE2//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CHECK_EQUALITY_HH_19FD627794361D94A2AEA40661216EE2

#include "src/util/password_storage.hh"

namespace PasswordStorage {
namespace Impl {

template <typename E, typename T>
void check_zero(T value)
{
    if (value != static_cast<T>(0))
    {
        throw E();
    }
}

template <typename T>
void throw_incorrect_password_exception_on_nonezero_value(T value)
{
    check_zero<IncorrectPassword>(value);
}

//see SlowEquals at https://crackstation.net/hashing-security.htm
template <typename T>
void check_equality(
        const T* lhs_ptr,
        const T* rhs_ptr,
        int items,
        void (*throw_exception_on_nonzero)(T));

} // namespace PasswordStorage::Impl
} // namespace PasswordStorage

#endif//CHECK_EQUALITY_HH_19FD627794361D94A2AEA40661216EE2
