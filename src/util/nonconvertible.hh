/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NONCONVERTIBLE_HH_E7127163E2844FA8A8C6DA098FD9D4D7
#define NONCONVERTIBLE_HH_E7127163E2844FA8A8C6DA098FD9D4D7

#include <type_traits>

namespace Util {

template <typename T>
struct Nonconvertible
{
    typedef T BaseType;
    template <typename N>
    class Named
    {
    public:
        typedef T BaseType;
        typedef N Tag;
        Named(): value_() { }
        Named(const Named& _src): value_(_src.value_) { }
        template <class S>
        static Named construct_from(const S& _src)
        {
            static_assert(std::is_same<BaseType, S>::value, "type S has to be the same as T");
            return Named(_src);
        }
        Named& operator=(const Named& _src)
        {
            if (this != &_src)
            {
                value_ = _src.value_;
            }
            return *this;
        }
        friend bool operator==(const Named& lhs, const Named& rhs)
        {
            return lhs.value_ == rhs.value_;
        }
        const BaseType& get_value()const { return value_; }
    private:
        explicit Named(const BaseType& _src): value_(_src) { }
        BaseType value_;
    };
};

} // namespace Util

#endif
