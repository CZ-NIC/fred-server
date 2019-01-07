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

#ifndef EXCEPTIONS_HH_2595C4AB83554D95BAD1348002A17E35
#define EXCEPTIONS_HH_2595C4AB83554D95BAD1348002A17E35

#include <exception>

namespace LibFred {
namespace Registrar {

struct NonExistentRegistrar : std::exception
{
    const char* what() const noexcept override;
};

struct NoUpdateData : std::exception
{
    const char* what() const noexcept override;
};

struct UpdateRegistrarException : std::exception
{
    const char* what() const noexcept override;
};

} // namespace LibFred::Registrar
} // namespace LibFred


#endif
