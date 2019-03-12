/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/accounting/impl/exceptions.hh"

namespace Fred {
namespace Backend {
namespace Accounting {
namespace Impl {

const char* RegistrarNotFound::what() const noexcept
{
    return "registrar not found";
}

const char* ZoneNotFound::what() const noexcept
{
    return "zone not found";
}

const char* InvalidAccountData::what() const noexcept
{
    return "invalid account_number / bank_code";
}

const char* InvalidCreditValue::what() const noexcept
{
    return "invalid credit value";
}

const char* InvalidPaymentData::what() const noexcept
{
    return "invalid payment data";
}

const char* InvalidTaxDate::what() const noexcept
{
    return "tax_date must be valid date and no more than 15 days before invoice_date";
}

const char* PaymentAlreadyProcessed::what() const noexcept
{
    return "payment already processed";
}

} // namespace Fred::Backend::Accounting::Impl
} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred
