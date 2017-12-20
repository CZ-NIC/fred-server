/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  registry record statement exceptions
 */

#include "src/backend/record_statement/exceptions.hh"

namespace Registry {
namespace RecordStatement {

const char* InternalServerError::what()const noexcept
{
    return "internal server error";
}

const char* ObjectNotFound::what()const noexcept
{
    return "registry object with specified ID does not exist";
}

} // namespace Registry::RecordStatement
} // namespace Registry
