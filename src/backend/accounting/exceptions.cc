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

const char* InvalidTaxDateFormat::what() const noexcept
{
    return "invalid tax_date format";
}

const char* TaxDateTooOld::what() const noexcept
{
    return "tax_date value must be less than 15 days before invoice_date (which is today)";
}

const char* PaymentTooOld::what() const noexcept
{
    return "payment too old - account.date (tax_date) value must be less than 15 days before invoice_date (which is today)";
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
