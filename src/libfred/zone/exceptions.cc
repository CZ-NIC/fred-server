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

#include "src/libfred/zone/exceptions.hh"

namespace LibFred {
namespace Zone {

const char* NonExistentZone::what() const noexcept
{
    return "Zone does not exist.";
}

const char* NotEnumZone::what() const noexcept
{
    return "Could not set enum validation period for non-enum zone.";
}

const char* NoZoneData::what() const noexcept
{
    return "No zone data for update.";
}

const char* DuplicateZone::what() const noexcept
{
    return "Zone already exists.";
}

const char* CreateZoneException::what() const noexcept
{
    return "Failed to create zone due to an unknown exception.";
}

const char* InfoZoneException::what() const noexcept
{
    return "Failed to get zone info due to an unknown exception.";
}

const char* UpdateZoneException::what() const noexcept
{
    return "Failed to update zone due to an unknown exception.";
}

} // namespace LibFred::Zone
} // namespace LibFred
