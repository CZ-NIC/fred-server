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

#ifndef UTIL_HH_8345C51906DE4928B293893C7B1DF9B3
#define UTIL_HH_8345C51906DE4928B293893C7B1DF9B3

#include "src/backend/epp/contact/hideable.hh"

namespace Epp {
namespace Contact {

template <typename T>
T trim(const T& src);

template <typename T>
Hideable<T> trim(const Hideable<T>& src)
{
    return src.make_with_the_same_privacy(trim(*src));
}

} // namespace Epp::Contact
} // namespace Epp

#endif
