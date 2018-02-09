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

#ifndef EXCEPTIONS_HH_F43965EE8C00484A82D6DD239D102C38
#define EXCEPTIONS_HH_F43965EE8C00484A82D6DD239D102C38

#include <exception>

namespace Registry {
namespace RecordStatement {

/**
 * Internal server error.
 * Unexpected failure, requires maintenance.
 */
struct InternalServerError:std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what()const noexcept;
};

/**
 * Requested object was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct ObjectNotFound:std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what()const noexcept;
};

struct ObjectDeleteCandidate:std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what()const noexcept;
};


} // namespace Registry::RecordStatement
} // namespace Registry

#endif
