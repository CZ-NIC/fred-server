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

#ifndef UTIL_H_B03B5CE2A82E4F1EA6B482414595DB5D
#define UTIL_H_B03B5CE2A82E4F1EA6B482414595DB5D

#include <vector>

namespace Epp {
namespace Contact {

template <typename T>
T trim(const T& src);

template <typename T>
std::vector<T> trim(const std::vector<T>& src);

} // namespace Epp::Contact
} // namespace Epp

#endif
