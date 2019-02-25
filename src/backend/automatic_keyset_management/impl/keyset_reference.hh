/*
 * Copyright (C) 2019  CZ.NIC, z.s.p.o.
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

#ifndef KEYSET_REFERENCE_HH_5792FFF26D80474BAF5539C0D3D01214
#define KEYSET_REFERENCE_HH_5792FFF26D80474BAF5539C0D3D01214

#include <string>

namespace Fred {
namespace Backend {
namespace AutomaticKeysetManagement {
namespace Impl {

struct KeysetReference
{
    KeysetReference(const unsigned long long _id, const std::string& _handle)
        : id(_id), handle(_handle)
    {
    }

    unsigned long long id;
    std::string handle;
};

} // namespace Fred::Backend::AutomaticKeysetManagement::Impl
} // namespace Fred::Backend::AutomaticKeysetManagement
} // namespace Fred::Backend
} // namespace Fred

#endif
