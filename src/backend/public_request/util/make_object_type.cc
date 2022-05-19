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

#include "src/backend/public_request/util/make_object_type.hh"

#include <stdexcept>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

ObjectType make_object_type(const std::string& _str)
{
        if (_str == "contact")
        {
            return ObjectType::contact;
        }
        else if (_str == "nsset")
        {
            return ObjectType::nsset;
        }
        else if (_str == "domain")
        {
            return ObjectType::domain;
        }
        else if (_str == "keyset")
        {
            return ObjectType::keyset;
        }
        throw std::runtime_error("unexpected object type");
}

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
