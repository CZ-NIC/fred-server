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

#ifndef EXCEPTIONS_HH_A96C589FC5F14474872247B053482D69
#define EXCEPTIONS_HH_A96C589FC5F14474872247B053482D69

#include <exception>

namespace LibFred {
namespace Registrar {
namespace ZoneAccess {

struct AddRegistrarZoneAccessException : std::exception
{
    const char* what() const noexcept override;
};

struct NonexistentRegistrar : std::exception
{
    const char* what() const noexcept override;
};

struct NonexistentZone : std::exception
{
    const char* what() const noexcept override;
};

} // namespace LibFred::Registrar::ZoneAccess
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
