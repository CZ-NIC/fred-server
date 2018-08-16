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

#include "src/backend/accounting/exceptions.hh"

namespace Fred {
namespace Backend {
namespace Accounting {

const char* InternalServerError::what() const noexcept
{
    return "internal server error";
}

const char* RegistrarNotFound::what() const noexcept
{
    return "registrar does not exist";
}

const char* InvalidZone::what() const noexcept
{
    return "zone does not exist";
}

const char* InvalidCreditValue::what() const noexcept
{
    return "invalid credit value";
}

const char* InvalidPaymentData::what() const noexcept
{
    return "invalid payment data";
}

const char* CreditAlreadyProcessed::what() const noexcept
{
    return "credit already processed";
}

const char* PaymentAlreadyProcessed::what() const noexcept
{
    return "payment already processed";
}

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred
