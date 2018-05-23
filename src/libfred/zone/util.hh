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

#ifndef UTIL_HH_3D464D2FBF714314991D5DF2D86C15F6
#define UTIL_HH_3D464D2FBF714314991D5DF2D86C15F6

#include <exception>
#include <string>

namespace LibFred {
namespace Zone {

struct NonExistentZone : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Zone does not exist.";
    }
};

struct NotEnumZone : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Could not set enum validation period for non-enum zone.";
    }
};

struct NoZoneData : std::exception
{
    virtual const char* what() const noexcept
    {
        return "No zone data for update.";
    }
};

struct CreateZoneException : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Failed to create zone due to an unknown exception.";
    }
};

struct InfoZoneException : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Failed to get zone info due to an unknown exception.";
    }
};

struct UpdateZoneException : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Failed to update zone due to an unknown exception.";
    }
};

bool is_enum_zone(const std::string& _fqdn);

} // namespace LibFred::Zone
} // namespace LibFred

#endif
