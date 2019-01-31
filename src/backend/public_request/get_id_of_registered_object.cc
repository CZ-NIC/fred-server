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

#include "src/backend/public_request/get_id_of_registered_object.hh"

#include "src/backend/public_request/object_type.hh"
#include "libfred/object/get_id_of_registered.hh"
#include "libfred/object/object_type.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

unsigned long long get_id_of_registered_object(
        LibFred::OperationContext& ctx,
        ObjectType object_type,
        const std::string& handle)
{
    switch (object_type)
    {
        case ObjectType::contact:
            return LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, handle);
        case ObjectType::nsset:
            return LibFred::get_id_of_registered<LibFred::Object_Type::nsset>(ctx, handle);
        case ObjectType::domain:
            return LibFred::get_id_of_registered<LibFred::Object_Type::domain>(ctx, handle);
        case ObjectType::keyset:
            return LibFred::get_id_of_registered<LibFred::Object_Type::keyset>(ctx, handle);
    }
    throw std::logic_error("unexpected Fred::Backend::PublicRequest::ObjectType enum value");
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
