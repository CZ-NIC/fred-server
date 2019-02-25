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
#ifndef UPDATE_ZONE_ACCESS_HH_591ADEA30B6E4414BF384D90719DDB63
#define UPDATE_ZONE_ACCESS_HH_591ADEA30B6E4414BF384D90719DDB63

#include "libfred/registrar/zone_access/registrar_zone_access_type.hh"

#include <exception>
#include <string>
#include <vector>

namespace Admin {
namespace Registrar {

void update_zone_access(const LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses& _zones);

struct UpdateZoneAccessException : std::exception
{
    const char* what() const noexcept override;
};

struct ZoneAccessNoUpdateData : std::exception
{
    const char* what() const noexcept override;
};

struct NonexistentZoneAccess : std::exception
{
    const char* what() const noexcept override;
};

struct ZoneAccessMissingParameters : std::exception
{
    const char* what() const noexcept override;
};

struct ZoneAccessNonexistentRegistrar : std::exception
{
    const char* what() const noexcept override;
};

struct NonexistentZone : std::exception
{
    const char* what() const noexcept override;
};

} // namespace Admin::Registrar
} // namespace Admin

#endif
