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

#ifndef EXCEPTIONS_HH_68A06AE9F47943A9A96D2D542BF74E12
#define EXCEPTIONS_HH_68A06AE9F47943A9A96D2D542BF74E12

#include <exception>

namespace CorbaConversion {
namespace Accounting {
namespace Impl {

struct InvalidPaymentData : std::exception
{
    const char* what() const noexcept;
};

struct InvalidRegistrar : std::exception
{
    const char* what() const noexcept;
};

struct InvalidPlaceAddress : std::exception
{
    const char* what() const noexcept;
};

} // namespace CorbaConversions::Accounting::Impl
} // namespace CorbaConversions::Accounting
} // namespace CorbaConversions

#endif
