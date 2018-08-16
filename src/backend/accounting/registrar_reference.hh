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

#ifndef REGISTRAR_REFERENCE_HH_8190815422C3471F87D19D84CFD7EF25
#define REGISTRAR_REFERENCE_HH_8190815422C3471F87D19D84CFD7EF25

#include <string>

namespace Fred {
namespace Backend {
namespace Accounting {

struct RegistrarReference
{
    RegistrarReference(
            const std::string& _handle,
            const std::string& _name)
        : handle(_handle),
          name(_name)
    {
    }

    std::string handle;
    std::string name;
};

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
