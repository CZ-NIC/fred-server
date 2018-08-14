/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef STREET_TRAITS_HH_B8DA8A66FA91C7EC22CC4E049020E8F4//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define STREET_TRAITS_HH_B8DA8A66FA91C7EC22CC4E049020E8F4

#include <array>

namespace Epp {
namespace Contact {

struct StreetTraits
{
    static constexpr auto number_of_rows = 3;
    template <typename T>
    using Rows = std::array<T, number_of_rows>;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//STREET_TRAITS_HH_B8DA8A66FA91C7EC22CC4E049020E8F4
