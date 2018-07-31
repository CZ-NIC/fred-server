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

#ifndef EXCEPTIONS_HH_4FEE22F55E4E454A8197ED9FB055BFF6
#define EXCEPTIONS_HH_4FEE22F55E4E454A8197ED9FB055BFF6

#include <exception>

namespace LibFred {
namespace Banking {

/**
 * Internal server error.
 * Unexpected failure, requires maintenance.
 */
struct RegistrarNotFound : std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const noexcept;
};

struct InvalidAccountData : std::exception
{
    const char* what() const noexcept;
};

struct PaymentAlreadyProcessed : std::exception
{
    const char* what() const noexcept;
};

} // namespace LibFred::Banking
} // namespace LibFred

#endif
