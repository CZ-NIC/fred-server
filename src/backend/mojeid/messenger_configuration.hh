/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#ifndef CONFIGURATION_HH_BC14892D4954478BB0A0F021DA9D7FA0
#define CONFIGURATION_HH_BC14892D4954478BB0A0F021DA9D7FA0

#include <string>

namespace Fred {
namespace Backend {
namespace MojeId {

struct MessengerConfiguration
{
    std::string endpoint;
    bool archive;
    bool archive_rendered;
};

} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

#endif
