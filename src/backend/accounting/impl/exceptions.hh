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
#ifndef EXCEPTIONS_HH_7B0D086B5B374B9E9FF12A8850886F77
#define EXCEPTIONS_HH_7B0D086B5B374B9E9FF12A8850886F77

#include <exception>

namespace Fred {
namespace Backend {
namespace Accounting {
namespace Impl {

struct RegistrarNotFound : std::exception
{
    const char* what() const noexcept override;
};

struct ZoneNotFound : std::exception
{
    const char* what() const noexcept override;
};

struct InvalidAccountData : std::exception
{
    const char* what() const noexcept override;
};

struct InvalidCreditValue : std::exception
{
    const char* what() const noexcept override;
};

struct InvalidPaymentData : std::exception
{
    const char* what() const noexcept override;
};

struct PaymentAlreadyProcessed : std::exception
{
    const char* what() const noexcept override;
};

} // namespace Fred::Backend::Accounting::Impl
} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
