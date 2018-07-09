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

#ifndef OBJECT_TYPE_HH_A5A290117166457FA6053D0159EDE046
#define OBJECT_TYPE_HH_A5A290117166457FA6053D0159EDE046

#include "src/libfred/opcontext.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

struct ObjectType
{
    enum Enum
    {
        contact,
        nsset,
        domain,
        keyset
    };
};

unsigned long long get_id_of_registered_object(
        LibFred::OperationContext& ctx,
        ObjectType::Enum object_type,
        const std::string& handle);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
