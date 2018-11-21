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

#include "src/libfred/banking/exceptions.hh"

namespace LibFred {
namespace Banking {

const char* RegistrarNotFound::what() const noexcept
{
    return "registrar not found";
}

const char* InvalidAccountData::what() const noexcept
{
    return "invalid account number or bank code";
}

const char* InvalidPaymentData::what() const noexcept
{
    return "invalid payment data";
}

const char* InvalidPriceValue::what() const noexcept
{
    return "invalid price value";
}

const char* PaymentAlreadyProcessed::what() const noexcept
{
    return "payment already processed";
}

} // namespace LibFred::Banking
} // namespace LibFred
