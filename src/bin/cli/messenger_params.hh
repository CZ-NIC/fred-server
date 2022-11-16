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
#ifndef MESSENGER_PARAMS_HH_779C1D436F8041EB986E1D778638557B
#define MESSENGER_PARAMS_HH_779C1D436F8041EB986E1D778638557B

#include <chrono>
#include <string>

struct MessengerArgs
{
    std::string endpoint;
    bool archive;
    bool archive_rendered;
    std::chrono::seconds timeout;
};

#endif

