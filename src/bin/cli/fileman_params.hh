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

#ifndef FILEMAN_PARAMS_HH_9534241C31034AC9B22052FD98AA9DC1
#define FILEMAN_PARAMS_HH_9534241C31034AC9B22052FD98AA9DC1

#include <string>

struct FilemanArgs
{
    std::string endpoint;

    FilemanArgs() = default;

    explicit FilemanArgs(const std::string&  _endpoint)
        : endpoint(_endpoint)
    {
    }
};

#endif
