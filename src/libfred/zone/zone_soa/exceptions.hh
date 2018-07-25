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

#ifndef EXCEPTIONS_HH_71A41501CC8C44CD89DA2317F6F7DC88
#define EXCEPTIONS_HH_71A41501CC8C44CD89DA2317F6F7DC88


#include <exception>

namespace LibFred {
namespace Zone {

struct NonExistentZoneSoa : std::exception
{
    const char* what() const noexcept override
    {
        return "Zone soa does not exist.";
    }
};

struct AlreadyExistingZoneSoa : std::exception
{
    const char* what() const noexcept override
    {
        return "Zone soa already exists.";
    }
};

struct NoZoneSoaData : std::exception
{
    const char* what() const noexcept override
    {
        return "No zone soa data for update.";
    }
};

struct CreateZoneSoaException : std::exception
{
    const char* what() const noexcept override
    {
        return "Failed to create zone soa due to an unknown exception.";
    }
};

struct UpdateZoneSoaException : std::exception
{
    const char* what() const noexcept override
    {
        return "Failed to update zone soa due to an unknown exception.";
    }
};
struct InfoZoneSoaException : std::exception
{
    const char* what() const noexcept override
    {
        return "Failed to get zone soa info due to an unknown exception.";
    }
};

} // namespace LibFred::Zone
} // namespace LibFred

#endif
