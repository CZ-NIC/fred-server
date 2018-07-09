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

#ifndef EXCEPTIONS_HH_FF025438A7C648E18057D7BFA7B5572D
#define EXCEPTIONS_HH_FF025438A7C648E18057D7BFA7B5572D

#include <stdexcept>
#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

struct NoPublicRequest : std::exception
{
    virtual const char* what() const noexcept
    {
        return "no public request found";
    }
};

struct ObjectAlreadyBlocked : std::exception
{
    virtual const char* what() const noexcept
    {
        return "object is already blocked";
    }
};

struct ObjectTransferProhibited : std::exception
{
    virtual const char* what() const noexcept
    {
        return "object transfer is prohibited";
    }
};

struct ObjectNotBlocked : std::exception
{
    virtual const char* what() const noexcept
    {
        return "object is not blocked";
    }
};

struct HasDifferentBlock : std::exception
{
    virtual const char* what() const noexcept
    {
        return "a different unblock request has to be issued";
    }
};

struct ObjectNotFound : std::exception
{
    virtual const char* what() const noexcept
    {
        return "registry object with specified ID does not exist";
    }
};

struct InvalidPublicRequestType : std::exception
{
    virtual const char* what() const noexcept
    {
        return "public request is not of post type";
    }
};

struct NoContactEmail : std::exception
{
    virtual const char* what() const noexcept
    {
        return "no contact email associated with this object";
    }
};

struct InvalidContactEmail : std::exception
{
    virtual const char* what() const noexcept
    {
        return "invalid contact email associated with this object";
    }
};

struct OperationProhibited : std::exception
{
    virtual const char* what() const noexcept
    {
        return "operation is prohibited";
    }
};

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
