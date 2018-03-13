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

#ifndef BUFFER_HH_00452E883E6E4E2584AE1E2D2A02B328
#define BUFFER_HH_00452E883E6E4E2584AE1E2D2A02B328

#include <string>

namespace Fred {
namespace Backend {

struct Buffer
{
    explicit Buffer(const std::string& _data);
    const std::string data;
};

} // namespace Fred::Backend
} // namespace Fred

#endif
