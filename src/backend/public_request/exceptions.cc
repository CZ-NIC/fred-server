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

#include "src/backend/public_request/exceptions.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

const char* NoPublicRequest::what() const noexcept
{
    return "no public request found";
}

const char* ObjectAlreadyBlocked::what() const noexcept
{
    return "object is already blocked";
}

const char* ObjectTransferProhibited::what() const noexcept
{
    return "object transfer is prohibited";
}

const char* ObjectNotBlocked::what() const noexcept
{
    return "object is not blocked";
}

const char* HasDifferentBlock::what() const noexcept
{
    return "a different unblock request has to be issued";
}

const char* ObjectNotFound::what() const noexcept
{
    return "registry object with specified ID does not exist";
}

const char* InvalidPublicRequestType::what() const noexcept
{
    return "public request is not of post type";
}

const char* NoContactEmail::what() const noexcept
{
    return "no contact email associated with this object";
}

const char* InvalidContactEmail::what() const noexcept
{
    return "invalid contact email associated with this object";
}

const char* OperationProhibited::what() const noexcept
{
    return "operation is prohibited";
}


} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
