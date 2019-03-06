/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef EXCEPTIONS_HH_F24345F27301491EA2CE1945B673BC16
#define EXCEPTIONS_HH_F24345F27301491EA2CE1945B673BC16

#include <string>
#include <exception>
#include <stdexcept>

namespace LibFred {

/// Exception when SQL error
struct SQL_ERROR : public std::runtime_error
{
    SQL_ERROR() : std::runtime_error("sql error") { }
};


/// Exception when specified object is not found
struct NOT_FOUND : public std::runtime_error
{
    NOT_FOUND() : std::runtime_error("not found") { }
};


/// Exception when specified object already exists
struct ALREADY_EXISTS : public std::runtime_error
{
    ALREADY_EXISTS() : std::runtime_error("already exists") { }
};

/// object is not blocked - e.g. registrar not blocked
struct NOT_BLOCKED : public std::runtime_error
{
    NOT_BLOCKED() : std::runtime_error("object is not blocked") { };
};

struct INVALID_VALUE : public std::runtime_error
{
    INVALID_VALUE(const std::string &what) : std::runtime_error(what) { };

    INVALID_VALUE() : std::runtime_error("invalid value specified") { };
};

};

#endif
