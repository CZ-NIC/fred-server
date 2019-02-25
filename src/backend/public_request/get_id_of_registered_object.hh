/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef GET_ID_OF_REGISTERED_OBJECT_HH_04A7BCB82CDC47FCAE74FF62B5B6D8B4
#define GET_ID_OF_REGISTERED_OBJECT_HH_04A7BCB82CDC47FCAE74FF62B5B6D8B4

#include "src/backend/public_request/object_type.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

unsigned long long get_id_of_registered_object(
        LibFred::OperationContext& ctx,
        ObjectType object_type,
        const std::string& handle);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
