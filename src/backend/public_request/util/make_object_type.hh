/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#ifndef MAKE_OBJECT_TYPE_HH_79D8D111838E46A7B57F3D08F7FB6609
#define MAKE_OBJECT_TYPE_HH_79D8D111838E46A7B57F3D08F7FB6609

#include "src/backend/public_request/object_type.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

ObjectType make_object_type(const std::string& _str);

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
