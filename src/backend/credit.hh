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

#ifndef CREDIT_HH_A02D7D4ACABE40B6B59C27648CA1D22B
#define CREDIT_HH_A02D7D4ACABE40B6B59C27648CA1D22B

#include "src/util/types/money.hh"

#include <string>

namespace Fred {
namespace Backend {

struct Credit
{
    explicit Credit(const std::string& _credit);
    Money value;
};

} // namespace Fred::Backend
} // namespace Fred

#endif
