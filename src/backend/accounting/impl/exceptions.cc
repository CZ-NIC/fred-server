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

const char* InvalidAccountNumberWithBankCode::what() const noexcept
{
    return "invalid account_number / bank_code";
}

const char* InvalidCreditValue::what() const noexcept
{
    return "invalid credit value";
}

} // namespace Fred::Backend::Accounting::Impl
} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred
