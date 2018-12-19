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

struct ObjectAlreadyBlocked : std::exception
{
    const char* what() const noexcept override;
};

struct ObjectTransferProhibited : std::exception
{
    const char* what() const noexcept override;
};

struct ObjectNotBlocked : std::exception
{
    const char* what() const noexcept override;
};

struct HasDifferentBlock : std::exception
{
    const char* what() const noexcept override;
};

struct ObjectNotFound : std::exception
{
    const char* what() const noexcept override;
};

struct InvalidPublicRequestType : std::exception
{
    const char* what() const noexcept override;
};

struct NoContactEmail : std::exception
{
    const char* what() const noexcept override;
};

struct InvalidContactEmail : std::exception
{
    const char* what() const noexcept override;
};

struct OperationProhibited : std::exception
{
    const char* what() const noexcept override;
};

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
