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

namespace Fred {
namespace Backend {
namespace PublicRequest {

struct NoPublicRequest : std::exception
{
    virtual const char* what() const noexcept;
};

struct ObjectAlreadyBlocked : std::exception
{
    virtual const char* what() const noexcept;
};

struct ObjectTransferProhibited : std::exception
{
    virtual const char* what() const noexcept;
};

struct ObjectNotBlocked : std::exception
{
    virtual const char* what() const noexcept;
};

struct HasDifferentBlock : std::exception
{
    virtual const char* what() const noexcept;
};

struct ObjectNotFound : std::exception
{
    virtual const char* what() const noexcept;
};

struct InvalidPublicRequestType : std::exception
{
    virtual const char* what() const noexcept;
};

struct NoContactEmail : std::exception
{
    virtual const char* what() const noexcept;
};

struct InvalidContactEmail : std::exception
{
    virtual const char* what() const noexcept;
};

struct OperationProhibited : std::exception
{
    virtual const char* what() const noexcept;
};

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
